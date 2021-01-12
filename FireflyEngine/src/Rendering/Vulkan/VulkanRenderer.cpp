#include "pch.h"
#include "Rendering/Vulkan/VulkanRenderer.h"

#include "Rendering/Vulkan/VulkanContext.h"
#include "Rendering/Vulkan/VulkanUtils.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Rendering/Vulkan/VulkanShader.h"

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

		ShaderCodePath shaderCodePath{};
		shaderCodePath.vertexShaderPath = "assets/shaders/triangle.vert.spv";
		shaderCodePath.fragmentShaderPath = "assets/shaders/triangle.frag.spv";
		m_shader = std::make_shared<VulkanShader>();
		m_shader->Init(m_device->GetDevice(), "Lit", shaderCodePath);

		VulkanMaterial* redMaterial = new VulkanMaterial(m_shader);
		redMaterial->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		VulkanMaterial* blueMaterial = new VulkanMaterial(m_shader);
		blueMaterial->SetColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		VulkanMaterial* greenMaterial = new VulkanMaterial(m_shader);
		greenMaterial->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		m_materials.push_back(redMaterial);
		m_materials.push_back(blueMaterial);
		m_materials.push_back(greenMaterial);

		m_pistolMesh = new VulkanMesh(m_device.get(), m_commandPool, m_device->GetGraphicsQueue(), "assets/meshes/pistol.fbx", true);
		m_globeMesh = new VulkanMesh(m_device.get(), m_commandPool, m_device->GetGraphicsQueue(), "assets/meshes/globe.fbx");
		m_armchairMesh = new VulkanMesh(m_device.get(), m_commandPool, m_device->GetGraphicsQueue(), "assets/meshes/armchair.fbx");
		VulkanRenderObject* pistol = new VulkanRenderObject(m_pistolMesh, redMaterial);
		VulkanRenderObject* globe = new VulkanRenderObject(m_globeMesh, blueMaterial);
		VulkanRenderObject* armchair = new VulkanRenderObject(m_armchairMesh, greenMaterial);
		pistol->SetModelMatrix(glm::rotate(glm::scale(glm::mat4(1), glm::vec3(0.01f)), -(float)M_PI_2, glm::vec3(1.f, 0.f, 0.f)));
		globe->SetModelMatrix(glm::scale(glm::translate(glm::mat4(1), glm::vec3(-1.5f, -0.5f, -1.5f)), glm::vec3(0.0075f)));
		armchair->SetModelMatrix(glm::scale(glm::translate(glm::rotate(glm::mat4(1), -(float)M_PI_2, glm::vec3(1.f, 0.f, 0.f)), glm::vec3(1.5f, 1.5f, -0.55f)), glm::vec3(0.01f)));
		m_renderObjects.push_back(pistol);
		m_renderObjects.push_back(globe);
		m_renderObjects.push_back(armchair);
		m_objectMaterialIndices.push_back(0);
		m_objectMaterialIndices.push_back(1);
		m_objectMaterialIndices.push_back(2);

		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSetLayouts();
		AllocateDescriptorSets();
		CreatePipelines();
	}

	void VulkanRenderer::Destroy()
	{
		m_device->WaitIdle();

		DestroyPipelines();
		DestroyDescriptorSetLayouts();
		DestroyDescriptorPool();
		DestroyUniformBuffers();

		for (auto object : m_renderObjects)
			delete object;
		m_renderObjects.clear();

		delete m_armchairMesh;
		delete m_globeMesh;
		delete m_pistolMesh;

		for (auto material : m_materials)
			delete material;
		m_materials.clear();

		m_shader->Destroy();

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

		UpdateUniformBuffers(camera);

		for (size_t i = 0; i < m_renderObjects.size(); i++)
		{
			std::string shaderTag = m_renderObjects[i]->GetMaterial()->GetShader()->GetTag();

			m_commandBuffers[m_currentImageIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines[shaderTag]);

			std::vector<vk::DescriptorSet> descriptorSets = 
			{ 
				m_sceneDataDescriptorSets[m_currentImageIndex], 
				m_materialDataDescriptorSets[m_currentImageIndex], 
				m_objectDataDescriptorSets[m_currentImageIndex] 
			};
			std::vector<uint32_t> dynamicOffsets =
			{
				static_cast<uint32_t>(m_objectMaterialIndices[i] * m_materialDataDynamicAlignment),
				static_cast<uint32_t>(i * m_objectDataDynamicAlignment)
			};
			m_commandBuffers[m_currentImageIndex].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayouts[shaderTag], 0, 
																	 descriptorSets.size(), descriptorSets.data(), 
																	 dynamicOffsets.size(), dynamicOffsets.data());

			m_renderObjects[i]->GetMesh()->Bind(m_commandBuffers[m_currentImageIndex]);

			m_commandBuffers[m_currentImageIndex].drawIndexed(m_renderObjects[i]->GetMesh()->GetIndexCount(), 1, 0, 0, 0);
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

	void VulkanRenderer::UpdateUniformBuffers(std::shared_ptr<Camera> camera)
	{
		// Scene Data ---------
		glm::vec3 cameraPosition = camera->GetPosition();
		glm::mat4 viewMatrix = camera->GetViewMatrix();
		glm::mat4 projectionMatrix = camera->GetProjectionMatrix();
		projectionMatrix[1][1] *= -1; // Vulkan has inverted y axis in comparison to OpenGL

		SceneData sceneData;
		sceneData.viewMatrix = viewMatrix;
		sceneData.projectionMatrix = projectionMatrix;
		sceneData.viewProjectionMatrix = projectionMatrix * viewMatrix;
		sceneData.cameraPosition = cameraPosition;

		void* mappedMemory;
		m_device->GetDevice().mapMemory(m_sceneDataUniformBufferMemories[m_currentImageIndex], 0, sizeof(SceneData), {}, &mappedMemory);
		memcpy(mappedMemory, &sceneData, sizeof(SceneData));
		m_device->GetDevice().unmapMemory(m_sceneDataUniformBufferMemories[m_currentImageIndex]);
		// --------------------
		// Material Data ------
		for (size_t i = 0; i < m_materials.size(); i++)
		{
			MaterialData* materialData = (MaterialData*)((uint64_t)m_materialData + (i * m_materialDataDynamicAlignment));
			(*materialData).color = m_materials[i]->GetColor();
		}

		m_device->GetDevice().mapMemory(m_materialDataUniformBufferMemories[m_currentImageIndex], 0, m_materialDataCount * m_materialDataDynamicAlignment, {}, &mappedMemory);
		memcpy(mappedMemory, m_materialData, m_materialDataCount * m_materialDataDynamicAlignment);
		m_device->GetDevice().unmapMemory(m_materialDataUniformBufferMemories[m_currentImageIndex]);
		// --------------------
		// Object Data --------
		for (size_t i = 0; i < m_renderObjects.size(); i++)
		{
			ObjectData* objectData = (ObjectData*)((uint64_t)m_objectData + (i * m_objectDataDynamicAlignment));
			(*objectData).modelMatrix = m_renderObjects[i]->GetModelMatrix();
			(*objectData).normalMatrix = glm::transpose(glm::inverse(glm::mat3(m_renderObjects[i]->GetModelMatrix())));
		}

		m_device->GetDevice().mapMemory(m_objectDataUniformBufferMemories[m_currentImageIndex], 0, m_objectDataCount * m_objectDataDynamicAlignment, {}, &mappedMemory);
		memcpy(mappedMemory, m_objectData, m_objectDataCount * m_objectDataDynamicAlignment);
		m_device->GetDevice().unmapMemory(m_objectDataUniformBufferMemories[m_currentImageIndex]);
		// --------------------
	}

	void VulkanRenderer::RecreateSwapchain()
	{
		m_device->WaitIdle();

		if (m_context->GetWidth() == 0 && m_context->GetHeight() == 0)
			return;

		DestroyPipelines();
		DestroyUniformBuffers();
		DestroyDescriptorPool();

		DestroySynchronizationPrimitivesForRendering();
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
		CreateSynchronizationPrimitivesForRendering();

		CreatePipelines();
		CreateDescriptorPool();
		CreateUniformBuffers();
		AllocateDescriptorSets();
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

	void VulkanRenderer::CreateUniformBuffers()
	{
		CreateSceneDataUniformBuffers();
		CreateMaterialDataUniformBuffers();
		CreateObjectDataUniformBuffers();
	}

	void VulkanRenderer::DestroyUniformBuffers()
	{
		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
		{
			m_device->GetDevice().destroyBuffer(m_sceneDataUniformBuffers[i]);
			m_device->GetDevice().freeMemory(m_sceneDataUniformBufferMemories[i]);
			m_device->GetDevice().destroyBuffer(m_materialDataUniformBuffers[i]);
			m_device->GetDevice().freeMemory(m_materialDataUniformBufferMemories[i]);
			m_device->GetDevice().destroyBuffer(m_objectDataUniformBuffers[i]);
			m_device->GetDevice().freeMemory(m_objectDataUniformBufferMemories[i]);
		}
	}

	void VulkanRenderer::CreateDescriptorPool()
	{
		vk::DescriptorPoolSize uniformBufferDescriptorPoolSize{};
		uniformBufferDescriptorPoolSize.type = vk::DescriptorType::eUniformBuffer;
		uniformBufferDescriptorPoolSize.descriptorCount = 100;

		vk::DescriptorPoolSize uniformBufferDynamicDescriptorPoolSize{};
		uniformBufferDynamicDescriptorPoolSize.type = vk::DescriptorType::eUniformBufferDynamic;
		uniformBufferDynamicDescriptorPoolSize.descriptorCount = 100;

		std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = { uniformBufferDescriptorPoolSize, uniformBufferDynamicDescriptorPoolSize };

		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
		descriptorPoolCreateInfo.pNext = nullptr;
		descriptorPoolCreateInfo.flags = {};
		descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
		descriptorPoolCreateInfo.maxSets = 10 * m_swapchain->GetImageCount();

		vk::Result result = m_device->GetDevice().createDescriptorPool(&descriptorPoolCreateInfo, nullptr, &m_descriptorPool);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan descriptor pool!");
	}

	void VulkanRenderer::DestroyDescriptorPool()
	{
		m_device->GetDevice().resetDescriptorPool(m_descriptorPool, {});
		m_device->GetDevice().destroyDescriptorPool(m_descriptorPool);
	}

	void VulkanRenderer::CreateDescriptorSetLayouts()
	{
		// SCENE DATA
		vk::DescriptorSetLayoutBinding sceneDataLayoutBinding{};
		sceneDataLayoutBinding.binding = 0;
		sceneDataLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		sceneDataLayoutBinding.descriptorCount = 1;
		sceneDataLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
		sceneDataLayoutBinding.pImmutableSamplers = nullptr;

		std::vector<vk::DescriptorSetLayoutBinding> bindings = { sceneDataLayoutBinding };

		vk::DescriptorSetLayoutCreateInfo sceneDataDescriptorSetLayoutCreateInfo{};
		sceneDataDescriptorSetLayoutCreateInfo.bindingCount = bindings.size();
		sceneDataDescriptorSetLayoutCreateInfo.pBindings = bindings.data();

		vk::Result result = m_device->GetDevice().createDescriptorSetLayout(&sceneDataDescriptorSetLayoutCreateInfo, nullptr, &m_sceneDataDescriptorSetLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor set layout!");

		// MATERIAL DATA
		vk::DescriptorSetLayoutBinding materialDataLayoutBinding{};
		materialDataLayoutBinding.binding = 0;
		materialDataLayoutBinding.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
		materialDataLayoutBinding.descriptorCount = 1;
		materialDataLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
		materialDataLayoutBinding.pImmutableSamplers = nullptr;

		bindings = { materialDataLayoutBinding };

		vk::DescriptorSetLayoutCreateInfo materialDataDescriptorSetLayoutCreateInfo{};
		materialDataDescriptorSetLayoutCreateInfo.bindingCount = bindings.size();
		materialDataDescriptorSetLayoutCreateInfo.pBindings = bindings.data();

		result = m_device->GetDevice().createDescriptorSetLayout(&materialDataDescriptorSetLayoutCreateInfo, nullptr, &m_materialDataDescriptorSetLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor set layout!");
		
		// OBJECT DATA
		vk::DescriptorSetLayoutBinding objectDataLayoutBinding{};
		objectDataLayoutBinding.binding = 0;
		objectDataLayoutBinding.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
		objectDataLayoutBinding.descriptorCount = 1;
		objectDataLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
		objectDataLayoutBinding.pImmutableSamplers = nullptr;

		bindings = { objectDataLayoutBinding };

		vk::DescriptorSetLayoutCreateInfo objectDataDescriptorSetLayoutCreateInfo{};
		objectDataDescriptorSetLayoutCreateInfo.bindingCount = bindings.size();
		objectDataDescriptorSetLayoutCreateInfo.pBindings = bindings.data();

		result = m_device->GetDevice().createDescriptorSetLayout(&objectDataDescriptorSetLayoutCreateInfo, nullptr, &m_objectDataDescriptorSetLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor set layout!");
	}

	void VulkanRenderer::DestroyDescriptorSetLayouts()
	{
		m_device->GetDevice().destroyDescriptorSetLayout(m_objectDataDescriptorSetLayout);
		m_device->GetDevice().destroyDescriptorSetLayout(m_materialDataDescriptorSetLayout);
		m_device->GetDevice().destroyDescriptorSetLayout(m_sceneDataDescriptorSetLayout);
	}

	void VulkanRenderer::AllocateDescriptorSets()
	{
		AllocateSceneDataDescriptorSets();
		AllocateMaterialDataDescriptorSets();
		AllocateObjectDataDescriptorSets();
	}

	void VulkanRenderer::CreateSceneDataUniformBuffers()
	{
		size_t bufferSize = sizeof(SceneData);
		vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		m_sceneDataUniformBuffers.resize(m_swapchain->GetImageCount());
		m_sceneDataUniformBufferMemories.resize(m_swapchain->GetImageCount());
		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
			VulkanUtils::CreateBuffer(m_device->GetDevice(), m_device->GetPhysicalDevice(), bufferSize, bufferUsageFlags, memoryPropertyFlags, m_sceneDataUniformBuffers[i], m_sceneDataUniformBufferMemories[i]);
	}

	void VulkanRenderer::CreateMaterialDataUniformBuffers()
	{
		size_t minUniformAlignment = m_device->GetPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
		m_materialDataDynamicAlignment = sizeof(SceneData);
		if (minUniformAlignment > 0)
			m_materialDataDynamicAlignment = (m_materialDataDynamicAlignment + minUniformAlignment - 1) & ~(minUniformAlignment - 1);

		size_t bufferSize = m_materialDataCount * m_materialDataDynamicAlignment; // TODO: grow/shrink dynamic buffer size dynamically
		m_materialData = (MaterialData*)_aligned_malloc(bufferSize, m_materialDataDynamicAlignment);
		vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		m_materialDataUniformBuffers.resize(m_swapchain->GetImageCount());
		m_materialDataUniformBufferMemories.resize(m_swapchain->GetImageCount());
		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
			VulkanUtils::CreateBuffer(m_device->GetDevice(), m_device->GetPhysicalDevice(), bufferSize, bufferUsageFlags, memoryPropertyFlags, m_materialDataUniformBuffers[i], m_materialDataUniformBufferMemories[i]);

	}

	void VulkanRenderer::CreateObjectDataUniformBuffers()
	{
		size_t minUniformAlignment = m_device->GetPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
		m_objectDataDynamicAlignment = sizeof(SceneData);
		if (minUniformAlignment > 0)
			m_objectDataDynamicAlignment = (m_objectDataDynamicAlignment + minUniformAlignment - 1) & ~(minUniformAlignment - 1);

		size_t bufferSize = m_objectDataCount * m_objectDataDynamicAlignment; // TODO: grow/shrink dynamic buffer size dynamically
		m_objectData = (ObjectData*)_aligned_malloc(bufferSize, m_objectDataDynamicAlignment);
		vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		m_objectDataUniformBuffers.resize(m_swapchain->GetImageCount());
		m_objectDataUniformBufferMemories.resize(m_swapchain->GetImageCount());
		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
			VulkanUtils::CreateBuffer(m_device->GetDevice(), m_device->GetPhysicalDevice(), bufferSize, bufferUsageFlags, memoryPropertyFlags, m_objectDataUniformBuffers[i], m_objectDataUniformBufferMemories[i]);
	}

	void VulkanRenderer::AllocateSceneDataDescriptorSets()
	{
		m_sceneDataDescriptorSets.resize(m_swapchain->GetImageCount());
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(m_swapchain->GetImageCount(), m_sceneDataDescriptorSetLayout);
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = m_swapchain->GetImageCount();
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		vk::Result result = m_device->GetDevice().allocateDescriptorSets(&descriptorSetAllocateInfo, m_sceneDataDescriptorSets.data());
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor sets!");

		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
		{
			vk::DescriptorBufferInfo sceneDataDescriptorBufferInfo{};
			sceneDataDescriptorBufferInfo.buffer = m_sceneDataUniformBuffers[i];
			sceneDataDescriptorBufferInfo.offset = 0;
			sceneDataDescriptorBufferInfo.range = sizeof(SceneData);

			vk::WriteDescriptorSet sceneDataWriteDescriptorSet{};
			sceneDataWriteDescriptorSet.dstSet = m_sceneDataDescriptorSets[i];
			sceneDataWriteDescriptorSet.dstBinding = 0;
			sceneDataWriteDescriptorSet.dstArrayElement = 0;
			sceneDataWriteDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
			sceneDataWriteDescriptorSet.descriptorCount = 1;
			sceneDataWriteDescriptorSet.pBufferInfo = &sceneDataDescriptorBufferInfo;
			sceneDataWriteDescriptorSet.pImageInfo = nullptr;
			sceneDataWriteDescriptorSet.pTexelBufferView = nullptr;

			std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { sceneDataWriteDescriptorSet };

			m_device->GetDevice().updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void VulkanRenderer::AllocateMaterialDataDescriptorSets()
	{
		m_materialDataDescriptorSets.resize(m_swapchain->GetImageCount());
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(m_swapchain->GetImageCount(), m_materialDataDescriptorSetLayout);
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = m_swapchain->GetImageCount();
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		vk::Result result = m_device->GetDevice().allocateDescriptorSets(&descriptorSetAllocateInfo, m_materialDataDescriptorSets.data());
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor sets!");

		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
		{
			vk::DescriptorBufferInfo materialDataDescriptorBufferInfo{};
			materialDataDescriptorBufferInfo.buffer = m_materialDataUniformBuffers[i];
			materialDataDescriptorBufferInfo.offset = 0;
			materialDataDescriptorBufferInfo.range = sizeof(MaterialData);

			vk::WriteDescriptorSet materialDataWriteDescriptorSet{};
			materialDataWriteDescriptorSet.dstSet = m_materialDataDescriptorSets[i];
			materialDataWriteDescriptorSet.dstBinding = 0;
			materialDataWriteDescriptorSet.dstArrayElement = 0;
			materialDataWriteDescriptorSet.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
			materialDataWriteDescriptorSet.descriptorCount = 1;
			materialDataWriteDescriptorSet.pBufferInfo = &materialDataDescriptorBufferInfo;
			materialDataWriteDescriptorSet.pImageInfo = nullptr;
			materialDataWriteDescriptorSet.pTexelBufferView = nullptr;

			std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { materialDataWriteDescriptorSet };

			m_device->GetDevice().updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void VulkanRenderer::AllocateObjectDataDescriptorSets()
	{
		m_objectDataDescriptorSets.resize(m_swapchain->GetImageCount());
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(m_swapchain->GetImageCount(), m_objectDataDescriptorSetLayout);
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = m_swapchain->GetImageCount();
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		vk::Result result = m_device->GetDevice().allocateDescriptorSets(&descriptorSetAllocateInfo, m_objectDataDescriptorSets.data());
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor sets!");

		for (size_t i = 0; i < m_swapchain->GetImageCount(); i++)
		{
			vk::DescriptorBufferInfo objectDataDescriptorBufferInfo{};
			objectDataDescriptorBufferInfo.buffer = m_objectDataUniformBuffers[i];
			objectDataDescriptorBufferInfo.offset = 0;
			objectDataDescriptorBufferInfo.range = sizeof(ObjectData);

			vk::WriteDescriptorSet objectDataWriteDescriptorSet{};
			objectDataWriteDescriptorSet.dstSet = m_objectDataDescriptorSets[i];
			objectDataWriteDescriptorSet.dstBinding = 0;
			objectDataWriteDescriptorSet.dstArrayElement = 0;
			objectDataWriteDescriptorSet.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
			objectDataWriteDescriptorSet.descriptorCount = 1;
			objectDataWriteDescriptorSet.pBufferInfo = &objectDataDescriptorBufferInfo;
			objectDataWriteDescriptorSet.pImageInfo = nullptr;
			objectDataWriteDescriptorSet.pTexelBufferView = nullptr;

			std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { objectDataWriteDescriptorSet };

			m_device->GetDevice().updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void VulkanRenderer::CreatePipelines()
	{
		// VERTEX INPUT STATE --------------------------
		vk::VertexInputBindingDescription vertexInputBindingDescription{};
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(VulkanMesh::Vertex);
		vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

		std::array<vk::VertexInputAttributeDescription, 5> vertexInputAttributeDescriptions{};
		vertexInputAttributeDescriptions[0].binding = 0;
		vertexInputAttributeDescriptions[0].location = 0;
		vertexInputAttributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[0].offset = offsetof(VulkanMesh::Vertex, position);
		vertexInputAttributeDescriptions[1].binding = 0;
		vertexInputAttributeDescriptions[1].location = 1;
		vertexInputAttributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[1].offset = offsetof(VulkanMesh::Vertex, normal);
		vertexInputAttributeDescriptions[2].binding = 0;
		vertexInputAttributeDescriptions[2].location = 2;
		vertexInputAttributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[2].offset = offsetof(VulkanMesh::Vertex, tangent);
		vertexInputAttributeDescriptions[3].binding = 0;
		vertexInputAttributeDescriptions[3].location = 3;
		vertexInputAttributeDescriptions[3].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[3].offset = offsetof(VulkanMesh::Vertex, bitangent);
		vertexInputAttributeDescriptions[4].binding = 0;
		vertexInputAttributeDescriptions[4].location = 4;
		vertexInputAttributeDescriptions[4].format = vk::Format::eR32G32Sfloat;
		vertexInputAttributeDescriptions[4].offset = offsetof(VulkanMesh::Vertex, texCoords);

		vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
		vertexInputStateCreateInfo.pNext = nullptr;
		vertexInputStateCreateInfo.flags = {};
		vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
		vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
		vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();
		// ---------------------------------------------
		// INPUT ASSEMBLY STATE ------------------------
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
		inputAssemblyStateCreateInfo.pNext = nullptr;
		inputAssemblyStateCreateInfo.flags = {};
		inputAssemblyStateCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;
		inputAssemblyStateCreateInfo.primitiveRestartEnable = false;
		// ---------------------------------------------
		// VIEWPORT STATE ------------------------------
		vk::Extent2D swapchainExtent = m_swapchain->GetExtent();
		vk::Viewport viewport{};
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = (float)swapchainExtent.width;
		viewport.height = (float)swapchainExtent.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		vk::Rect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapchainExtent;

		vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.pNext = nullptr;
		viewportStateCreateInfo.flags = {};
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pViewports = &viewport;
		viewportStateCreateInfo.scissorCount = 1;
		viewportStateCreateInfo.pScissors = &scissor;
		// ---------------------------------------------
		// RASTERIZATION STATE -------------------------
		vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
		rasterizationStateCreateInfo.pNext = nullptr;
		rasterizationStateCreateInfo.flags = {};
		rasterizationStateCreateInfo.depthClampEnable = false;
		rasterizationStateCreateInfo.rasterizerDiscardEnable = false;
		rasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;
		rasterizationStateCreateInfo.lineWidth = 1.f;
		rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
		rasterizationStateCreateInfo.frontFace = vk::FrontFace::eCounterClockwise;
		rasterizationStateCreateInfo.depthBiasEnable = false;
		rasterizationStateCreateInfo.depthBiasConstantFactor = 0.f;
		rasterizationStateCreateInfo.depthBiasClamp = 0.f;
		rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.f;
		rasterizationStateCreateInfo.depthClampEnable = false;
		// ---------------------------------------------
		// MULTISAMPLE STATE ---------------------------
		vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
		multisampleStateCreateInfo.pNext = nullptr;
		multisampleStateCreateInfo.flags = {};
		multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisampleStateCreateInfo.sampleShadingEnable = false;
		multisampleStateCreateInfo.minSampleShading = 1.0f;
		multisampleStateCreateInfo.pSampleMask = nullptr;
		multisampleStateCreateInfo.alphaToCoverageEnable = false;
		multisampleStateCreateInfo.alphaToOneEnable = false;
		// ---------------------------------------------
		// COLOR BLEND STATE ---------------------------
		vk::PipelineColorBlendAttachmentState colorBlendAttachmentState{};
		colorBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachmentState.blendEnable = false;
		colorBlendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eOne;
		colorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		colorBlendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachmentState.alphaBlendOp = vk::BlendOp::eAdd;

		vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
		colorBlendStateCreateInfo.logicOpEnable = false;
		colorBlendStateCreateInfo.logicOp = vk::LogicOp::eCopy;
		colorBlendStateCreateInfo.attachmentCount = 1;
		colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
		colorBlendStateCreateInfo.blendConstants[0] = 0.f;
		colorBlendStateCreateInfo.blendConstants[1] = 0.f;
		colorBlendStateCreateInfo.blendConstants[2] = 0.f;
		colorBlendStateCreateInfo.blendConstants[3] = 0.f;
		// ---------------------------------------------
		// DEPTH STENCIL STATE -------------------------
		vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
		depthStencilStateCreateInfo.pNext = nullptr;
		depthStencilStateCreateInfo.flags = {};
		depthStencilStateCreateInfo.depthTestEnable = true;
		depthStencilStateCreateInfo.depthWriteEnable = true;
		depthStencilStateCreateInfo.depthCompareOp = vk::CompareOp::eLess;
		depthStencilStateCreateInfo.depthBoundsTestEnable = false;
		depthStencilStateCreateInfo.minDepthBounds = 0.0f;
		depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
		depthStencilStateCreateInfo.stencilTestEnable = false;
		depthStencilStateCreateInfo.front = {};
		depthStencilStateCreateInfo.back = {};
		// ---------------------------------------------
		// PIPELINE LAYOUT -----------------------------
		//vk::PushConstantRange pushConstantRange{};
		//pushConstantRange.offset = 0;
		//pushConstantRange.size = sizeof(glm::mat4);
		//pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;

		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = { m_sceneDataDescriptorSetLayout, m_materialDataDescriptorSetLayout , m_objectDataDescriptorSetLayout};

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = {};
		pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		vk::PipelineLayout pipelineLayout;
		vk::Result result = m_device->GetDevice().createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan pipeline layout!");
		// ---------------------------------------------
		// SHADER STAGE STATE --------------------------
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos = m_shader->GetShaderStageCreateInfos();
		// ---------------------------------------------
		// GRAPHICS PIPELINE ---------------------------
		vk::GraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.pNext = nullptr;
		pipelineCreateInfo.flags = {};
		pipelineCreateInfo.stageCount = shaderStageCreateInfos.size();
		pipelineCreateInfo.pStages = shaderStageCreateInfos.data();
		pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
		pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
		pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
		pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
		pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
		pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
		pipelineCreateInfo.pDynamicState = nullptr;
		pipelineCreateInfo.layout = pipelineLayout;
		pipelineCreateInfo.renderPass = m_renderPass;
		pipelineCreateInfo.subpass = 0;
		pipelineCreateInfo.basePipelineHandle = nullptr;
		pipelineCreateInfo.basePipelineIndex = -1;

		vk::Pipeline pipeline;
		result = m_device->GetDevice().createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan graphics pipeline!");
		// ---------------------------------------------
		m_pipelineLayouts[m_shader->GetTag()] = pipelineLayout;
		m_pipelines[m_shader->GetTag()] = pipeline;
	}

	void VulkanRenderer::DestroyPipelines()
	{
		for (auto pipelineEntry : m_pipelines)
			m_device->GetDevice().destroyPipeline(pipelineEntry.second);
		m_pipelines.clear();

		for(auto pipelineLayoutEntry : m_pipelineLayouts)
			m_device->GetDevice().destroyPipelineLayout(pipelineLayoutEntry.second);
		m_pipelineLayouts.clear();
	}
}