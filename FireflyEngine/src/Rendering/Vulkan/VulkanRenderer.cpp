#include "pch.h"
#include "Rendering/Vulkan/VulkanRenderer.h"

#include "Rendering/Vulkan/VulkanContext.h"
#include "Rendering/Vulkan/VulkanUtils.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Firefly
{
	VulkanRenderer::VulkanRenderer()
	{
	}

	void VulkanRenderer::Init(std::shared_ptr<GraphicsContext> context)
	{
		m_context = std::dynamic_pointer_cast<VulkanContext>(context);
		m_device = m_context->GetDevice();

		CreateSwapchain();

		CreateCommandPool();
		AllocateCommandBuffers();

		CreateDepthImage();
		CreateRenderPass();
		CreateFramebuffers();
		CreateSynchronizationPrimitivesForRendering();

		m_material = new VulkanMaterial("assets/shaders/triangle.vert.spv", "assets/shaders/triangle.frag.spv", m_device->GetDevice(), m_renderPass, m_swapchain.get());
		m_pistolMesh = new VulkanMesh(m_device.get(), m_commandPool, m_device->GetGraphicsQueue(), "assets/meshes/pistol.fbx", true);
		m_globeMesh = new VulkanMesh(m_device.get(), m_commandPool, m_device->GetGraphicsQueue(), "assets/meshes/globe.fbx");
		m_armchairMesh = new VulkanMesh(m_device.get(), m_commandPool, m_device->GetGraphicsQueue(), "assets/meshes/armchair.fbx");
		VulkanRenderObject* pistol = new VulkanRenderObject(m_pistolMesh, m_material);
		VulkanRenderObject* globe = new VulkanRenderObject(m_globeMesh, m_material);
		VulkanRenderObject* armchair = new VulkanRenderObject(m_armchairMesh, m_material);
		pistol->SetModelMatrix(glm::rotate(glm::scale(glm::mat4(1), glm::vec3(0.01f)), -(float)M_PI_2, glm::vec3(1.f, 0.f, 0.f)));
		globe->SetModelMatrix(glm::scale(glm::translate(glm::mat4(1), glm::vec3(-1.5f, -0.5f, -1.5f)), glm::vec3(0.0075f)));
		armchair->SetModelMatrix(glm::scale(glm::translate(glm::rotate(glm::mat4(1), -(float)M_PI_2, glm::vec3(1.f, 0.f, 0.f)), glm::vec3(1.5f, 1.5f, -0.55f)), glm::vec3(0.01f)));
		m_renderObjects.push_back(pistol);
		m_renderObjects.push_back(globe);
		m_renderObjects.push_back(armchair);

		CreateCameraDataUniformBuffers();
		CreateDescriptorPool();
		AllocateGlobalDescriptorSets();
	}

	void VulkanRenderer::Destroy()
	{
		m_device->WaitIdle();

		DestroyDescriptorPool();
		DestroyCameraDataUniformBuffers();

		for (auto object : m_renderObjects)
			delete object;
		m_renderObjects.clear();

		delete m_armchairMesh;
		delete m_globeMesh;
		delete m_pistolMesh;
		delete m_material;

		DestroySynchronizationPrimitivesForRendering();
		DestroyFramebuffers();
		DestroyRenderPass();
		DestroyDepthImage();

		FreeCommandBuffers();
		DestroyCommandPool();

		DestroySwapchain();
	}

	void VulkanRenderer::BeginDrawRecording()
	{
	}

	void VulkanRenderer::RecordDraw(const Entity& entity)
	{
	}

	void VulkanRenderer::EndDrawRecording()
	{
	}

	void VulkanRenderer::SubmitDraw(std::shared_ptr<Camera> camera)
	{
		// AQUIRE NEXT IMAGE
		vk::Result result = m_device->GetDevice().acquireNextImageKHR(m_swapchain->GetSwapchain(), UINT64_MAX, m_isNewImageAvailableSemaphores[m_currentFrameIndex], nullptr, &m_currentImageIndex);
		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
		{
			RecreateSwapchain();
			return;
		}
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to aquire next image from the swapchain!");

		// RENDER TO IMAGE
		// wait until the indexed command buffer is not used anymore before recording new commands to it
		m_device->GetDevice().waitForFences(1, &m_isCommandBufferAvailableFences[m_currentImageIndex], true, UINT64_MAX);
		m_device->GetDevice().resetFences(1, &m_isCommandBufferAvailableFences[m_currentImageIndex]);

		m_commandBuffers[m_currentImageIndex].reset({});
		vk::CommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		result = m_commandBuffers[m_currentImageIndex].begin(&commandBufferBeginInfo);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to begin recording Vulkan command buffer!");

		std::array<vk::ClearValue, 2> clearValues = {}; // order of clear values needs to be in the order of attachments
		clearValues[0].color = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		vk::Extent2D swapchainExtent = m_swapchain->GetExtent();

		vk::RenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.framebuffer = m_framebuffers[m_currentImageIndex];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = swapchainExtent;
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		m_commandBuffers[m_currentImageIndex].beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

		glm::vec3 cameraPosition = camera->GetPosition();
		glm::mat4 viewMatrix = camera->GetViewMatrix();
		glm::mat4 projectionMatrix = camera->GetProjectionMatrix();
		projectionMatrix[1][1] *= -1; // Vulkan has inverted y axis in comparison to OpenGL

		CameraData cameraData;
		cameraData.position = glm::vec4(cameraPosition, 1.0f);
		cameraData.viewMatrix = viewMatrix;
		cameraData.projectionMatrix = projectionMatrix;
		cameraData.viewProjectionMatrix = projectionMatrix * viewMatrix;

		void* mappedMemory;
		m_device->GetDevice().mapMemory(m_cameraDataUniformBufferMemories[m_currentImageIndex], 0, sizeof(CameraData), {}, &mappedMemory);
		memcpy(mappedMemory, &cameraData, sizeof(CameraData));
		m_device->GetDevice().unmapMemory(m_cameraDataUniformBufferMemories[m_currentImageIndex]);

		for (auto object : m_renderObjects)
		{
			m_commandBuffers[m_currentImageIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, object->GetMaterial()->GetPipeline());

			std::vector<vk::DescriptorSet> descriptorSets = { m_globalDescriptorSets[m_currentImageIndex] };
			m_commandBuffers[m_currentImageIndex].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, object->GetMaterial()->GetPipelineLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

			glm::mat4 modelMatrix = object->GetModelMatrix();
			m_commandBuffers[m_currentImageIndex].pushConstants(object->GetMaterial()->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &modelMatrix);

			object->GetMesh()->Bind(m_commandBuffers[m_currentImageIndex]);

			m_commandBuffers[m_currentImageIndex].drawIndexed(object->GetMesh()->GetIndexCount(), 1, 0, 0, 0);
		}

		m_commandBuffers[m_currentImageIndex].endRenderPass();
		m_commandBuffers[m_currentImageIndex].end();

		std::vector<vk::Semaphore> isNewImageAvailableSemaphores = { m_isNewImageAvailableSemaphores[m_currentFrameIndex] };
		std::vector<vk::Semaphore> isRenderedImageAvailableSemaphores = { m_isRenderedImageAvailableSemaphores[m_currentFrameIndex] };
		vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput; // ColorAttachmentOutputStage waits for isImageAvailableSemaphore
		vk::SubmitInfo submitInfo{};
		submitInfo.waitSemaphoreCount = isNewImageAvailableSemaphores.size();
		submitInfo.pWaitSemaphores = isNewImageAvailableSemaphores.data();
		submitInfo.pWaitDstStageMask = &waitStageMask;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffers[m_currentImageIndex];
		submitInfo.signalSemaphoreCount = isRenderedImageAvailableSemaphores.size();
		submitInfo.pSignalSemaphores = isRenderedImageAvailableSemaphores.data();

		result = m_device->GetGraphicsQueue().submit(1, &submitInfo, m_isCommandBufferAvailableFences[m_currentImageIndex]);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to submit commands to the graphics queue!");

		// PRESENT RENDERED IMAGE
		vk::PresentInfoKHR presentInfo{};
		std::vector<vk::SwapchainKHR> swapchains = { m_swapchain->GetSwapchain() };
		presentInfo.waitSemaphoreCount = isRenderedImageAvailableSemaphores.size();
		presentInfo.pWaitSemaphores = isRenderedImageAvailableSemaphores.data();
		presentInfo.swapchainCount = swapchains.size();
		presentInfo.pSwapchains = swapchains.data();
		presentInfo.pImageIndices = &m_currentImageIndex;
		presentInfo.pResults = nullptr;

		result = m_device->GetPresentQueue().presentKHR(&presentInfo);
		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
		{
			RecreateSwapchain();
			return;
		}
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to present the image with the present queue!");

		m_currentFrameIndex = (m_currentFrameIndex + 1) % m_swapchain->GetImageCount();
	}

	void VulkanRenderer::RecreateSwapchain()
	{
		m_device->WaitIdle();

		if (m_context->GetWidth() == 0 && m_context->GetHeight() == 0)
			return;

		DestroyCameraDataUniformBuffers();
		DestroyDescriptorPool();

		DestroySynchronizationPrimitivesForRendering();
		m_material->DestroyPipeline();
		DestroyFramebuffers();
		DestroyRenderPass();
		DestroyDepthImage();
		FreeCommandBuffers();
		DestroySwapchain();

		CreateSwapchain();
		AllocateCommandBuffers();
		CreateDepthImage();
		CreateRenderPass();
		CreateFramebuffers();
		m_material->CreatePipeline(m_renderPass, m_swapchain.get());
		CreateSynchronizationPrimitivesForRendering();

		CreateDescriptorPool();
		CreateCameraDataUniformBuffers();
		AllocateGlobalDescriptorSets();
	}

	void VulkanRenderer::CreateSwapchain()
	{
		m_swapchain = std::make_shared<VulkanSwapchain>();
		m_swapchain->Init(m_context);
	}

	void VulkanRenderer::DestroySwapchain()
	{
		m_swapchain->Destroy();
	}

	void VulkanRenderer::CreateCommandPool()
	{
		vk::CommandPoolCreateInfo commandPoolCreateInfo{};
		commandPoolCreateInfo.pNext = nullptr;
		commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		commandPoolCreateInfo.queueFamilyIndex = m_device->GetGraphicsQueueFamilyIndex();

		vk::Result result = m_device->GetDevice().createCommandPool(&commandPoolCreateInfo, nullptr, &m_commandPool);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan command pool!");
	}

	void VulkanRenderer::DestroyCommandPool()
	{
		m_device->GetDevice().destroyCommandPool(m_commandPool);
	}

	void VulkanRenderer::AllocateCommandBuffers()
	{
		m_commandBuffers.resize(m_swapchain->GetImageCount());
		vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = m_commandPool;
		commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
		commandBufferAllocateInfo.commandBufferCount = m_commandBuffers.size();

		vk::Result result = m_device->GetDevice().allocateCommandBuffers(&commandBufferAllocateInfo, m_commandBuffers.data());
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan command buffers!");
	}

	void VulkanRenderer::FreeCommandBuffers()
	{
		m_device->GetDevice().freeCommandBuffers(m_commandPool, m_commandBuffers.size(), m_commandBuffers.data());
	}

	void VulkanRenderer::CreateCameraDataUniformBuffers()
	{
		size_t bufferSize = sizeof(CameraData);
		vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		m_cameraDataUniformBuffers.resize(m_swapchain->GetImageCount());
		m_cameraDataUniformBufferMemories.resize(m_swapchain->GetImageCount());
		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
			VulkanUtils::CreateBuffer(m_device->GetDevice(), m_device->GetPhysicalDevice(), bufferSize, bufferUsageFlags, memoryPropertyFlags, m_cameraDataUniformBuffers[i], m_cameraDataUniformBufferMemories[i]);
	}

	void VulkanRenderer::DestroyCameraDataUniformBuffers()
	{
		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
		{
			m_device->GetDevice().destroyBuffer(m_cameraDataUniformBuffers[i]);
			m_device->GetDevice().freeMemory(m_cameraDataUniformBufferMemories[i]);
		}
	}

	void VulkanRenderer::CreateDescriptorPool()
	{
		vk::DescriptorPoolSize uniformBufferDescriptorPoolSize{};
		uniformBufferDescriptorPoolSize.type = vk::DescriptorType::eUniformBuffer;
		uniformBufferDescriptorPoolSize.descriptorCount = 100;

		std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = { uniformBufferDescriptorPoolSize };

		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
		descriptorPoolCreateInfo.pNext = nullptr;
		descriptorPoolCreateInfo.flags = {};
		descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
		descriptorPoolCreateInfo.maxSets = 4 * m_swapchain->GetImageCount();

		vk::Result result = m_device->GetDevice().createDescriptorPool(&descriptorPoolCreateInfo, nullptr, &m_descriptorPool);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan descriptor pool!");
	}

	void VulkanRenderer::DestroyDescriptorPool()
	{
		m_device->GetDevice().resetDescriptorPool(m_descriptorPool, {});
		m_device->GetDevice().destroyDescriptorPool(m_descriptorPool);
	}

	void VulkanRenderer::AllocateGlobalDescriptorSets()
	{
		// DESCRIPTOR SETS
		m_globalDescriptorSets.resize(m_swapchain->GetImageCount());
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(m_swapchain->GetImageCount(), m_material->GetGlobalDescriptorSetLayout());
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = m_swapchain->GetImageCount();
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		vk::Result result = m_device->GetDevice().allocateDescriptorSets(&descriptorSetAllocateInfo, m_globalDescriptorSets.data());
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor sets!");

		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
		{
			vk::DescriptorBufferInfo cameraDataDescriptorBufferInfo{};
			cameraDataDescriptorBufferInfo.buffer = m_cameraDataUniformBuffers[i];
			cameraDataDescriptorBufferInfo.offset = 0;
			cameraDataDescriptorBufferInfo.range = sizeof(CameraData);

			vk::WriteDescriptorSet cameraDataWriteDescriptorSet{};
			cameraDataWriteDescriptorSet.dstSet = m_globalDescriptorSets[i];
			cameraDataWriteDescriptorSet.dstBinding = 0;
			cameraDataWriteDescriptorSet.dstArrayElement = 0;
			cameraDataWriteDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
			cameraDataWriteDescriptorSet.descriptorCount = 1;
			cameraDataWriteDescriptorSet.pBufferInfo = &cameraDataDescriptorBufferInfo;
			cameraDataWriteDescriptorSet.pImageInfo = nullptr;
			cameraDataWriteDescriptorSet.pTexelBufferView = nullptr;

			std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { cameraDataWriteDescriptorSet };

			m_device->GetDevice().updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void VulkanRenderer::CreateDepthImage()
	{
		m_depthFormat = VulkanUtils::FindDepthFormat(m_device->GetPhysicalDevice());
		uint32_t mipLevels = 1;
		vk::Extent2D swapchainExtent = m_swapchain->GetExtent();
		vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
		vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
		VulkanUtils::CreateImage(m_device->GetDevice(), m_device->GetPhysicalDevice(), swapchainExtent.width, swapchainExtent.height, mipLevels, vk::SampleCountFlagBits::e1, m_depthFormat, tiling, imageUsageFlags, memoryPropertyFlags, m_depthImage, m_depthImageMemory);

		m_depthImageView = VulkanUtils::CreateImageView(m_device->GetDevice(), m_depthImage, mipLevels, m_depthFormat, vk::ImageAspectFlagBits::eDepth);

		vk::ImageLayout oldLayout = vk::ImageLayout::eUndefined;
		vk::ImageLayout newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		VulkanUtils::TransitionImageLayout(m_device->GetDevice(), m_commandPool, m_device->GetGraphicsQueue(), m_depthImage, mipLevels, m_depthFormat, oldLayout, newLayout);
	}

	void VulkanRenderer::DestroyDepthImage()
	{
		m_device->GetDevice().destroyImageView(m_depthImageView);
		m_device->GetDevice().destroyImage(m_depthImage);
		m_device->GetDevice().freeMemory(m_depthImageMemory);
	}

	void VulkanRenderer::CreateRenderPass()
	{
		vk::AttachmentDescription colorAttachmentDescription{};
		colorAttachmentDescription.flags = {};
		colorAttachmentDescription.format = m_swapchain->GetImageFormat();
		colorAttachmentDescription.samples = vk::SampleCountFlagBits::e1;
		colorAttachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachmentDescription.finalLayout = vk::ImageLayout::ePresentSrcKHR;

		vk::AttachmentReference colorAttachmentReference{};
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::AttachmentDescription depthAttachmentDescription{};
		depthAttachmentDescription.flags = {};
		depthAttachmentDescription.format = m_depthFormat;
		depthAttachmentDescription.samples = vk::SampleCountFlagBits::e1;
		depthAttachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachmentDescription.storeOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		depthAttachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
		depthAttachmentDescription.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::AttachmentReference depthAttachmentReference{};
		depthAttachmentReference.attachment = 1;
		depthAttachmentReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpassDescription.flags = {};
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentReference;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		vk::SubpassDependency subpassDependency{};
		subpassDependency.dependencyFlags = {};
		subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependency.dstSubpass = 0;
		subpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		subpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		subpassDependency.srcAccessMask = {};
		subpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		std::array<vk::AttachmentDescription, 2> attachments = { colorAttachmentDescription, depthAttachmentDescription };
		vk::RenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.pNext = nullptr;
		renderPassCreateInfo.flags = {};
		renderPassCreateInfo.attachmentCount = attachments.size();
		renderPassCreateInfo.pAttachments = attachments.data();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &subpassDependency;

		vk::Result result = m_device->GetDevice().createRenderPass(&renderPassCreateInfo, nullptr, &m_renderPass);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan render pass!");
	}

	void VulkanRenderer::DestroyRenderPass()
	{
		m_device->GetDevice().destroyRenderPass(m_renderPass);
	}

	void VulkanRenderer::CreateFramebuffers()
	{
		vk::Extent2D swapchainExtent = m_swapchain->GetExtent();
		m_framebuffers.resize(m_swapchain->GetImageCount());
		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
		{
			std::vector<vk::ImageView> attachments = { m_swapchain->GetImageViews()[i], m_depthImageView };
			vk::FramebufferCreateInfo framebufferCreateInfo{};
			framebufferCreateInfo.renderPass = m_renderPass;
			framebufferCreateInfo.attachmentCount = attachments.size();
			framebufferCreateInfo.pAttachments = attachments.data();
			framebufferCreateInfo.width = swapchainExtent.width;
			framebufferCreateInfo.height = swapchainExtent.height;
			framebufferCreateInfo.layers = 1;

			vk::Result result = m_device->GetDevice().createFramebuffer(&framebufferCreateInfo, nullptr, &m_framebuffers[i]);
			FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan framebuffer!");
		}
	}

	void VulkanRenderer::DestroyFramebuffers()
	{
		for (const vk::Framebuffer& framebuffer : m_framebuffers)
			m_device->GetDevice().destroyFramebuffer(framebuffer);
	}

	void VulkanRenderer::CreateSynchronizationPrimitivesForRendering()
	{
		vk::SemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = {};

		vk::FenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		m_isNewImageAvailableSemaphores.resize(m_swapchain->GetImageCount());
		m_isRenderedImageAvailableSemaphores.resize(m_swapchain->GetImageCount());
		m_isCommandBufferAvailableFences.resize(m_swapchain->GetImageCount());
		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
		{
			vk::Result result = m_device->GetDevice().createSemaphore(&semaphoreCreateInfo, nullptr, &m_isNewImageAvailableSemaphores[i]);
			FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Semaphore!");

			result = m_device->GetDevice().createSemaphore(&semaphoreCreateInfo, nullptr, &m_isRenderedImageAvailableSemaphores[i]);
			FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Semaphore!");

			result = m_device->GetDevice().createFence(&fenceCreateInfo, nullptr, &m_isCommandBufferAvailableFences[i]);
			FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Fence!");
		}
	}

	void VulkanRenderer::DestroySynchronizationPrimitivesForRendering()
	{
		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
		{
			m_device->GetDevice().destroyFence(m_isCommandBufferAvailableFences[i]);
			m_device->GetDevice().destroySemaphore(m_isRenderedImageAvailableSemaphores[i]);
			m_device->GetDevice().destroySemaphore(m_isNewImageAvailableSemaphores[i]);
		}
	}
}