#include "Rendering/Vulkan/VulkanContext.h"

#include <GLFW/glfw3.h>
#include <fstream>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

namespace Firefly
{
	VulkanContext::VulkanContext(void* window)
	{
		m_glfwWindow = (GLFWwindow*)(window);
		FIREFLY_ASSERT(m_glfwWindow, "Vulkan requires a GLFWwindow pointer!");

		m_vertices = {
			{{ -1.0f, -1.0f,  1.0f }, { 1.0f, 0.0f, 0.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{  1.0f, -1.0f,  1.0f }, { 0.0f, 1.0f, 0.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{  1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f, 1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{ -1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{ -1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{  1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{  1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, 1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{ -1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
		};
		m_indices = { 0,1,2, 2,3,0, 1,5,6, 6,2,1, 7,6,5, 5,4,7, 4,0,3, 3,7,4, 4,5,1, 1,0,4, 3,2,6, 6,7,3 };

		for (size_t i = 0; i < m_objectCount; i++)
		{
			m_modelMatrices.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(i, i, -(float)i)));
		}

		CreateInstance();
		CreateDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateDevice();
		CreateSwapchain();
		CreateCommandPool();
		AllocateCommandBuffers();
		CreateVertexBuffers();
		CreateIndexBuffers();
		CreateUniformBuffers();
		CreateDescriptorPool();
		AllocateDescriptorSets();
		CreateDepthImage();
		CreateRenderPass();
		CreateFramebuffers();
		CreateGraphicsPipeline();
		CreateSynchronizationPrimitivesForRendering();
	}

	VulkanContext::~VulkanContext()
	{
		m_device.waitIdle();

		DestroySynchronizationPrimitivesForRendering();
		DestroyGraphicsPipeline();
		DestroyFramebuffers();
		DestroyRenderPass();
		DestroyDepthImage();
		FreeDescriptorSets();
		DestroyDescriptorPool();
		DestroyUniformBuffers();
		DestroyIndexBuffers();
		DestroyVertexBuffers();
		FreeCommandBuffers();
		DestroyCommandPool();
		DestroySwapchain();
		DestroyDevice();
		DestroySurface();
		DestroyDebugMessenger();
		DestroyInstance();
	}

	void VulkanContext::Draw()
	{
		// AQUIRE NEXT IMAGE
		uint32_t currentImageIndex;
		vk::Result result = m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, m_isImageAvailableSemaphore[m_currentFrameIndex], nullptr, &currentImageIndex);
		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
		{
			RecreateSwapchain();
			return;
		}
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to aquire next image from the swapchain!");

		// RENDER TO IMAGE
		// wait until the indexed command buffer is not used anymore before recording new commands to it
		m_device.waitForFences(1, &m_isCommandBufferFinishedFences[currentImageIndex], true, UINT64_MAX);
		m_device.resetFences(1, &m_isCommandBufferFinishedFences[currentImageIndex]);

		m_commandBuffers[currentImageIndex].reset({});
		vk::CommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		result = m_commandBuffers[currentImageIndex].begin(&commandBufferBeginInfo);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to begin recording Vulkan command buffer!");

		std::array<vk::ClearValue, 2> clearValues = {}; // order of clear values needs to be in the order of attachments
		clearValues[0].color = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		vk::RenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.framebuffer = m_framebuffers[currentImageIndex];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = m_swapchainExtent;
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		m_commandBuffers[currentImageIndex].beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
		m_commandBuffers[currentImageIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);

		// update uniform buffer per frame
		m_uboPerFrame.viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		m_uboPerFrame.projectionMatrix = glm::perspective(glm::radians(45.0f), m_swapchainExtent.width / (float)m_swapchainExtent.height, 0.1f, 10000.0f);
		m_uboPerFrame.projectionMatrix[1][1] *= -1; // Vulkan has inverted y axis in comparison to OpenGL

		void* mappedMemoryPerFrame;
		m_device.mapMemory(m_uniformBufferMemoriesPerFrame[currentImageIndex], 0, sizeof(UboPerFrame), {}, &mappedMemoryPerFrame);
		memcpy(mappedMemoryPerFrame, &m_uboPerFrame, sizeof(UboPerFrame));
		m_device.unmapMemory(m_uniformBufferMemoriesPerFrame[currentImageIndex]);

		// update dynamic uniform buffer per object for model matrix
		for (size_t i = 0; i < m_objectCount; i++)
		{
			glm::mat4* modelMatrix = (glm::mat4*)((uint64_t)m_uboPerObject.modelMatrixData + i * m_modelMatrixUniformAlignment);
			*modelMatrix = m_modelMatrices[i];
		}
		void* mappedMemoryPerObject;
		size_t bufferSize = m_objectCount * m_modelMatrixUniformAlignment;
		m_device.mapMemory(m_uniformBufferMemoriesPerObject[currentImageIndex], 0, bufferSize, {}, &mappedMemoryPerObject);
		memcpy(mappedMemoryPerObject, m_uboPerObject.modelMatrixData, bufferSize);
		vk::MappedMemoryRange memoryRange {};
		memoryRange.memory = m_uniformBufferMemoriesPerObject[currentImageIndex];
		memoryRange.size = bufferSize;
		m_device.flushMappedMemoryRanges(1, &memoryRange);
		m_device.unmapMemory(m_uniformBufferMemoriesPerObject[currentImageIndex]);

		for (size_t i = 0; i < m_objectCount; i++)
		{
			std::vector<vk::Buffer> vertexBuffers = { m_vertexBuffer };
			vk::DeviceSize offsets[] = { 0 };
			m_commandBuffers[currentImageIndex].bindVertexBuffers(0, vertexBuffers.size(), vertexBuffers.data(), offsets);
			m_commandBuffers[currentImageIndex].bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint32);

			uint32_t dynamicOffset = i * m_modelMatrixUniformAlignment;
			m_commandBuffers[currentImageIndex].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1, &m_descriptorSets[currentImageIndex], 1, &dynamicOffset);

			m_commandBuffers[currentImageIndex].drawIndexed(m_indices.size(), 1, 0, 0, 0);
		}

		m_commandBuffers[currentImageIndex].endRenderPass();
		m_commandBuffers[currentImageIndex].end();

		std::vector<vk::Semaphore> isImageAvailableSemaphores = { m_isImageAvailableSemaphore[m_currentFrameIndex] };
		std::vector<vk::Semaphore> isRenderingFinishedSemaphores = { m_isRenderingFinishedSemaphore[m_currentFrameIndex] };
		vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		vk::SubmitInfo submitInfo{};
		submitInfo.waitSemaphoreCount = isImageAvailableSemaphores.size();
		submitInfo.pWaitSemaphores = isImageAvailableSemaphores.data();
		submitInfo.pWaitDstStageMask = &waitStageMask;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffers[currentImageIndex];
		submitInfo.signalSemaphoreCount = isRenderingFinishedSemaphores.size();
		submitInfo.pSignalSemaphores = isRenderingFinishedSemaphores.data();

		vk::Queue graphicsQueue = m_device.getQueue(m_graphicsQueueFamilyIndex, 0);
		result = graphicsQueue.submit(1, &submitInfo, m_isCommandBufferFinishedFences[currentImageIndex]);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to submit commands to the graphics queue!");

		// PRESENT RENDERED IMAGE
		vk::PresentInfoKHR presentInfo{};
		std::vector<vk::SwapchainKHR> swapchains = { m_swapchain };
		presentInfo.waitSemaphoreCount = isRenderingFinishedSemaphores.size();
		presentInfo.pWaitSemaphores = isRenderingFinishedSemaphores.data();
		presentInfo.swapchainCount = swapchains.size();
		presentInfo.pSwapchains = swapchains.data();
		presentInfo.pImageIndices = &currentImageIndex;
		presentInfo.pResults = nullptr;

		vk::Queue presentQueue = m_device.getQueue(m_presentQueueFamilyIndex, 0);
		result = presentQueue.presentKHR(&presentInfo);
		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
		{
			RecreateSwapchain();
			return;
		}
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to present the image with the present queue!");

		m_currentFrameIndex = (m_currentFrameIndex + 1) % m_swapchainImages.size();
	}

	void VulkanContext::CreateInstance() 
	{
		// TODO: Hand in the name and version of the application from client

		vk::ApplicationInfo applicationInfo{};
		applicationInfo.pNext = nullptr;
		applicationInfo.pApplicationName = "Sandbox";
		applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.pEngineName = "FireflyEngine";
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.apiVersion = VK_API_VERSION_1_2;

		// TODO: Check required instance extensions and layers
		std::vector<const char*> requiredInstanceExtensions = GetRequiredInstanceExtensions();
		std::vector<const char*> requiredInstanceLayers = GetRequiredInstanceLayers();
		vk::InstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.pNext = nullptr;
		instanceCreateInfo.flags = {};
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.enabledExtensionCount = requiredInstanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();
		instanceCreateInfo.enabledLayerCount = requiredInstanceLayers.size();
		instanceCreateInfo.ppEnabledLayerNames = requiredInstanceLayers.data();

		vk::Result result = vk::createInstance(&instanceCreateInfo, nullptr, &m_instance);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan instance!");
	}

	void VulkanContext::DestroyInstance()
	{
		m_instance.destroy();
	}

	void VulkanContext::CreateDebugMessenger()
	{
		if (!AreValidationLayersEnabled())
			return;

		vk::DebugUtilsMessengerCreateInfoEXT debugMessngerCreateInfo{};
		debugMessngerCreateInfo.pNext = nullptr;
		debugMessngerCreateInfo.flags = {};
		debugMessngerCreateInfo.pUserData = nullptr;
		debugMessngerCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		debugMessngerCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		debugMessngerCreateInfo.pfnUserCallback = DebugMessengerCallback;

		vk::DispatchLoaderDynamic dispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);
		vk::Result result = m_instance.createDebugUtilsMessengerEXT(&debugMessngerCreateInfo, nullptr, &m_debugMessenger, dispatchLoaderDynamic);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan debug messenger!");
	}

	void VulkanContext::DestroyDebugMessenger()
	{
		if (!AreValidationLayersEnabled())
			return;

		vk::DispatchLoaderDynamic dispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);
		m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, dispatchLoaderDynamic);
	}

	void VulkanContext::CreateSurface()
	{
		VkSurfaceKHR surface;
		vk::Result result = vk::Result(glfwCreateWindowSurface(m_instance, m_glfwWindow, nullptr, &surface));
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan window surface!");
		m_surface = vk::SurfaceKHR(surface);
	}

	void VulkanContext::DestroySurface()
	{
		m_instance.destroySurfaceKHR(m_surface);
	}

	void VulkanContext::PickPhysicalDevice()
	{
		std::vector<vk::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices();
		FIREFLY_ASSERT(physicalDevices.size() > 0, "Unable to find a graphics card with Vulkan support!");

		// TODO: Check required device extensions and other requirements
		// TODO: Pick most suitable device

		m_physicalDevice = physicalDevices[0];

		// Find required queue family indices
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();
		bool graphicsSupport = false;
		bool presentSupport = false;
		for (size_t i = 0; i < queueFamilyProperties.size(); i++)
		{
			if (!graphicsSupport)
			{
				if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
				{
					graphicsSupport = true;
					m_graphicsQueueFamilyIndex = i;
				}
			}
			if (!presentSupport)
			{
				vk::Bool32 surfaceSupport;
				m_physicalDevice.getSurfaceSupportKHR(i, m_surface, &surfaceSupport);
				if (surfaceSupport)
				{
					presentSupport = true;
					m_presentQueueFamilyIndex = i;
				}
			}
		}
		FIREFLY_ASSERT(graphicsSupport && presentSupport, "Picked Vulkan device doesn't support all required queue families!");
	}

	void VulkanContext::CreateDevice()
	{
		float queuePriority = 1.0f;
		vk::DeviceQueueCreateInfo graphicsQueueCreateInfo{};
		graphicsQueueCreateInfo.pNext = nullptr;
		graphicsQueueCreateInfo.flags = {};
		graphicsQueueCreateInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
		graphicsQueueCreateInfo.queueCount = 1;
		graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;

		vk::DeviceQueueCreateInfo presentQueueCreateInfo{};
		presentQueueCreateInfo.pNext = nullptr;
		presentQueueCreateInfo.flags = {};
		presentQueueCreateInfo.queueFamilyIndex = m_presentQueueFamilyIndex;
		presentQueueCreateInfo.queueCount = 1;
		presentQueueCreateInfo.pQueuePriorities = &queuePriority;

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = { graphicsQueueCreateInfo, presentQueueCreateInfo };

		std::vector<const char*> requiredDeviceExtensions = GetRequiredDeviceExtensions();
		std::vector<const char*> requiredDeviceLayers = GetRequiredDeviceLayers();
		vk::DeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.pNext = nullptr;
		deviceCreateInfo.flags = {};
		deviceCreateInfo.enabledExtensionCount = requiredDeviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
		deviceCreateInfo.enabledLayerCount = requiredDeviceLayers.size();
		deviceCreateInfo.ppEnabledLayerNames = requiredDeviceLayers.data();
		deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = nullptr;

		vk::Result result = m_physicalDevice.createDevice(&deviceCreateInfo, nullptr, &m_device);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan logical device!");
	}
	
	void VulkanContext::DestroyDevice() 
	{
		m_device.destroy();
	}

	void VulkanContext::CreateSwapchain()
	{
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
		if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
		{
			m_swapchainExtent = surfaceCapabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(m_glfwWindow, &width, &height);
			vk::Extent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
			actualExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, actualExtent.height));
			m_swapchainExtent = actualExtent;
		}

		std::vector<vk::SurfaceFormatKHR> surfaceFormats = m_physicalDevice.getSurfaceFormatsKHR(m_surface);
		for (const vk::SurfaceFormatKHR& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				m_swapchainSurfaceFormat = surfaceFormat;
				break;
			}
			else
			{
				m_swapchainSurfaceFormat = surfaceFormats[0];
			}
		}

		std::vector<vk::PresentModeKHR> presentModes = m_physicalDevice.getSurfacePresentModesKHR(m_surface);
		for (const vk::PresentModeKHR& presentMode : presentModes)
		{
			if (presentMode == vk::PresentModeKHR::eMailbox)
			{
				m_swapchainPresentMode = presentMode;
				break;
			}
			else
			{
				m_swapchainPresentMode = vk::PresentModeKHR::eFifo;
			}
		}

		uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
		if (surfaceCapabilities.maxImageCount > 0)
			imageCount = std::min(imageCount, surfaceCapabilities.maxImageCount);

		vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.flags = {};
		swapchainCreateInfo.surface = m_surface;
		swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
		swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchainCreateInfo.presentMode = m_swapchainPresentMode;
		swapchainCreateInfo.clipped = true;
		swapchainCreateInfo.oldSwapchain = nullptr;
		swapchainCreateInfo.minImageCount = imageCount;
		swapchainCreateInfo.imageFormat = m_swapchainSurfaceFormat.format;
		swapchainCreateInfo.imageColorSpace = m_swapchainSurfaceFormat.colorSpace;
		swapchainCreateInfo.imageExtent = m_swapchainExtent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		std::vector<uint32_t> queueFamilyIndices = { m_graphicsQueueFamilyIndex, m_presentQueueFamilyIndex };
		if (m_graphicsQueueFamilyIndex != m_presentQueueFamilyIndex)
		{
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchainCreateInfo.queueFamilyIndexCount = queueFamilyIndices.size();
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}
		else
		{
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
			swapchainCreateInfo.queueFamilyIndexCount = 0;
			swapchainCreateInfo.pQueueFamilyIndices = nullptr;
		}

		vk::Result result = m_device.createSwapchainKHR(&swapchainCreateInfo, nullptr, &m_swapchain);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan swapchain!");

		m_swapchainImages = m_device.getSwapchainImagesKHR(m_swapchain);

		m_swapchainImageViews.resize(m_swapchainImages.size());
		uint32_t mipLevels = 1;
		for (size_t i = 0; i < m_swapchainImages.size(); i++)
			m_swapchainImageViews[i] = CreateImageView(m_swapchainImages[i], mipLevels, m_swapchainSurfaceFormat.format, vk::ImageAspectFlagBits::eColor);
	}

	void VulkanContext::RecreateSwapchain()
	{
		m_device.waitIdle();

		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(m_glfwWindow, &width, &height);
		if (width == 0 && height == 0)
			return;

		DestroySynchronizationPrimitivesForRendering();
		DestroyGraphicsPipeline();
		DestroyFramebuffers();
		DestroyRenderPass();
		DestroyDepthImage();
		FreeDescriptorSets();
		FreeCommandBuffers();
		DestroySwapchain();

		CreateSwapchain();
		AllocateCommandBuffers();
		AllocateDescriptorSets();
		CreateDepthImage();
		CreateRenderPass();
		CreateFramebuffers();
		CreateGraphicsPipeline();
		CreateSynchronizationPrimitivesForRendering();
	}

	void VulkanContext::DestroySwapchain()
	{
		for (const vk::ImageView& imageView : m_swapchainImageViews)
			m_device.destroyImageView(imageView);
		m_device.destroySwapchainKHR(m_swapchain);
	}

	void VulkanContext::CreateCommandPool()
	{
		vk::CommandPoolCreateInfo commandPoolCreateInfo{};
		commandPoolCreateInfo.pNext = nullptr;
		commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		commandPoolCreateInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;

		vk::Result result = m_device.createCommandPool(&commandPoolCreateInfo, nullptr, &m_commandPool);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan command pool!");
	}

	void VulkanContext::DestroyCommandPool()
	{
		m_device.destroyCommandPool(m_commandPool);
	}

	void VulkanContext::AllocateCommandBuffers()
	{
		m_commandBuffers.resize(m_swapchainImages.size());
		vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = m_commandPool;
		commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
		commandBufferAllocateInfo.commandBufferCount = m_commandBuffers.size();

		vk::Result result = m_device.allocateCommandBuffers(&commandBufferAllocateInfo, m_commandBuffers.data());
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan command buffers!");
	}

	void VulkanContext::FreeCommandBuffers()
	{
		m_device.freeCommandBuffers(m_commandPool, m_commandBuffers.size(), m_commandBuffers.data());
	}

	void VulkanContext::CreateVertexBuffers()
	{
		vk::DeviceSize bufferSize = sizeof(Mesh::Vertex) * m_vertices.size();
		vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

		vk::Buffer stagingVertexBuffer;
		vk::DeviceMemory stagingVertexBufferMemory;
		CreateBuffer(bufferSize, bufferUsageFlags, memoryPropertyFlags, stagingVertexBuffer, stagingVertexBufferMemory);

		void* mappedMemory;
		m_device.mapMemory(stagingVertexBufferMemory, 0, bufferSize, {}, &mappedMemory);
		memcpy(mappedMemory, m_vertices.data(), bufferSize);
		m_device.unmapMemory(stagingVertexBufferMemory);

		bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
		memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
		CreateBuffer(bufferSize, bufferUsageFlags, memoryPropertyFlags, m_vertexBuffer, m_vertexBufferMemory);

		CopyBuffer(stagingVertexBuffer, m_vertexBuffer, bufferSize);

		m_device.destroyBuffer(stagingVertexBuffer);
		m_device.freeMemory(stagingVertexBufferMemory);
	}

	void VulkanContext::DestroyVertexBuffers()
	{
		m_device.destroyBuffer(m_vertexBuffer);
		m_device.freeMemory(m_vertexBufferMemory);
	}

	void VulkanContext::CreateIndexBuffers()
	{
		vk::DeviceSize bufferSize = sizeof(uint32_t) * m_indices.size();
		vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

		vk::Buffer stagingIndexBuffer;
		vk::DeviceMemory stagingIndexBufferMemory;
		CreateBuffer(bufferSize, bufferUsageFlags, memoryPropertyFlags, stagingIndexBuffer, stagingIndexBufferMemory);

		void* mappedMemory;
		m_device.mapMemory(stagingIndexBufferMemory, 0, bufferSize, {}, &mappedMemory);
		memcpy(mappedMemory, m_indices.data(), bufferSize);
		m_device.unmapMemory(stagingIndexBufferMemory);

		bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
		memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
		CreateBuffer(bufferSize, bufferUsageFlags, memoryPropertyFlags, m_indexBuffer, m_indexBufferMemory);

		CopyBuffer(stagingIndexBuffer, m_indexBuffer, bufferSize);

		m_device.destroyBuffer(stagingIndexBuffer);
		m_device.freeMemory(stagingIndexBufferMemory);
	}

	void VulkanContext::DestroyIndexBuffers()
	{
		m_device.destroyBuffer(m_indexBuffer);
		m_device.freeMemory(m_indexBufferMemory);
	}

	void VulkanContext::CreateUniformBuffers()
	{
		size_t bufferSize = sizeof(UboPerFrame);
		vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		m_uniformBuffersPerFrame.resize(m_swapchainImages.size());
		m_uniformBufferMemoriesPerFrame.resize(m_swapchainImages.size());
		for (size_t i = 0; i < m_swapchainImages.size(); i++)
			CreateBuffer(bufferSize, bufferUsageFlags, memoryPropertyFlags, m_uniformBuffersPerFrame[i], m_uniformBufferMemoriesPerFrame[i]);

		size_t minUniformBufferOffsetAlignment = m_physicalDevice.getProperties().limits.minUniformBufferOffsetAlignment;
		m_modelMatrixUniformAlignment = sizeof(glm::mat4);
		if (minUniformBufferOffsetAlignment > 0)
			m_modelMatrixUniformAlignment = (m_modelMatrixUniformAlignment + minUniformBufferOffsetAlignment - 1) & ~(minUniformBufferOffsetAlignment - 1);

		bufferSize = m_objectCount * m_modelMatrixUniformAlignment;
		m_uboPerObject.modelMatrixData = (glm::mat4*)_aligned_malloc(bufferSize, m_modelMatrixUniformAlignment);
		bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
		memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible;// | vk::MemoryPropertyFlagBits::eHostCoherent;
		m_uniformBuffersPerObject.resize(m_swapchainImages.size());
		m_uniformBufferMemoriesPerObject.resize(m_swapchainImages.size());
		for (size_t i = 0; i < m_swapchainImages.size(); i++)
			CreateBuffer(bufferSize, bufferUsageFlags, memoryPropertyFlags, m_uniformBuffersPerObject[i], m_uniformBufferMemoriesPerObject[i]);
	}

	void VulkanContext::DestroyUniformBuffers()
	{
		for (size_t i = 0; i < m_swapchainImages.size(); i++)
		{
			m_device.destroyBuffer(m_uniformBuffersPerObject[i]);
			m_device.freeMemory(m_uniformBufferMemoriesPerObject[i]);
			m_device.destroyBuffer(m_uniformBuffersPerFrame[i]);
			m_device.freeMemory(m_uniformBufferMemoriesPerFrame[i]);
		}
	}

	void VulkanContext::CreateDescriptorPool()
	{
		vk::DescriptorPoolSize uniformBufferDescriptorPoolSizePerFrame{};
		uniformBufferDescriptorPoolSizePerFrame.type = vk::DescriptorType::eUniformBuffer;
		uniformBufferDescriptorPoolSizePerFrame.descriptorCount = m_swapchainImages.size();

		vk::DescriptorPoolSize uniformBufferDescriptorPoolSizePerObject{};
		uniformBufferDescriptorPoolSizePerObject.type = vk::DescriptorType::eUniformBufferDynamic;
		uniformBufferDescriptorPoolSizePerObject.descriptorCount = m_swapchainImages.size();

		std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = { uniformBufferDescriptorPoolSizePerFrame, uniformBufferDescriptorPoolSizePerObject };

		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
		descriptorPoolCreateInfo.pNext = nullptr;
		descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
		descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
		descriptorPoolCreateInfo.maxSets = m_swapchainImages.size();

		vk::Result result = m_device.createDescriptorPool(&descriptorPoolCreateInfo, nullptr, &m_descriptorPool);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan descriptor pool!");
	}

	void VulkanContext::DestroyDescriptorPool()
	{
		m_device.destroyDescriptorPool(m_descriptorPool);
	}

	void VulkanContext::AllocateDescriptorSets()
	{
		// DESCRIPTOR LAYOUT
		vk::DescriptorSetLayoutBinding uniformBufferObjectLayoutBindingPerFrame{};
		uniformBufferObjectLayoutBindingPerFrame.binding = 0;
		uniformBufferObjectLayoutBindingPerFrame.descriptorType = vk::DescriptorType::eUniformBuffer;
		uniformBufferObjectLayoutBindingPerFrame.descriptorCount = 1;
		uniformBufferObjectLayoutBindingPerFrame.stageFlags = vk::ShaderStageFlagBits::eVertex;
		uniformBufferObjectLayoutBindingPerFrame.pImmutableSamplers = nullptr;

		vk::DescriptorSetLayoutBinding uniformBufferObjectLayoutBindingPerObject{};
		uniformBufferObjectLayoutBindingPerObject.binding = 1;
		uniformBufferObjectLayoutBindingPerObject.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
		uniformBufferObjectLayoutBindingPerObject.descriptorCount = 1;
		uniformBufferObjectLayoutBindingPerObject.stageFlags = vk::ShaderStageFlagBits::eVertex;
		uniformBufferObjectLayoutBindingPerObject.pImmutableSamplers = nullptr;

		std::vector<vk::DescriptorSetLayoutBinding> bindings = { uniformBufferObjectLayoutBindingPerFrame, uniformBufferObjectLayoutBindingPerObject };

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
		descriptorSetLayoutCreateInfo.pBindings = bindings.data();

		vk::Result result = m_device.createDescriptorSetLayout(&descriptorSetLayoutCreateInfo, nullptr, &m_descriptorSetLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor set layout!");

		// DESCRIPTOR SETS
		m_descriptorSets.resize(m_swapchainImages.size());
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(m_swapchainImages.size(), m_descriptorSetLayout);
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = m_swapchainImages.size();
		descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

		result = m_device.allocateDescriptorSets(&descriptorSetAllocateInfo, m_descriptorSets.data());
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to allocate Vulkan descriptor sets!");

		for (size_t i = 0; i < m_swapchainImages.size(); i++)
		{
			vk::DescriptorBufferInfo descriptorBufferInfoPerFrame{};
			descriptorBufferInfoPerFrame.buffer = m_uniformBuffersPerFrame[i];
			descriptorBufferInfoPerFrame.offset = 0;
			descriptorBufferInfoPerFrame.range = sizeof(UboPerFrame);

			vk::DescriptorBufferInfo descriptorBufferInfoPerObject{};
			descriptorBufferInfoPerObject.buffer = m_uniformBuffersPerObject[i];
			descriptorBufferInfoPerObject.offset = 0;
			descriptorBufferInfoPerObject.range = sizeof(glm::mat4);//m_modelMatrixUniformAlignment;

			std::array<vk::WriteDescriptorSet, 2> writeDescriptorSets{};
			writeDescriptorSets[0].dstSet = m_descriptorSets[i];
			writeDescriptorSets[0].dstBinding = 0;
			writeDescriptorSets[0].dstArrayElement = 0;
			writeDescriptorSets[0].descriptorType = vk::DescriptorType::eUniformBuffer;
			writeDescriptorSets[0].descriptorCount = 1;
			writeDescriptorSets[0].pBufferInfo = &descriptorBufferInfoPerFrame;
			writeDescriptorSets[0].pImageInfo = nullptr;
			writeDescriptorSets[0].pTexelBufferView = nullptr;

			writeDescriptorSets[1].dstSet = m_descriptorSets[i];
			writeDescriptorSets[1].dstBinding = 1;
			writeDescriptorSets[1].dstArrayElement = 0;
			writeDescriptorSets[1].descriptorType = vk::DescriptorType::eUniformBufferDynamic;
			writeDescriptorSets[1].descriptorCount = 1;
			writeDescriptorSets[1].pBufferInfo = &descriptorBufferInfoPerObject;
			writeDescriptorSets[1].pImageInfo = nullptr;
			writeDescriptorSets[1].pTexelBufferView = nullptr;

			m_device.updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void VulkanContext::FreeDescriptorSets()
	{
		m_device.freeDescriptorSets(m_descriptorPool, m_descriptorSets.size(), m_descriptorSets.data());
		m_device.destroyDescriptorSetLayout(m_descriptorSetLayout);
	}

	void VulkanContext::CreateDepthImage()
	{
		uint32_t mipLevels = 1;
		vk::Format depthFormat = FindDepthFormat();
		vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
		vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
		CreateImage(m_swapchainExtent.width, m_swapchainExtent.height, mipLevels, vk::SampleCountFlagBits::e1, depthFormat, tiling, imageUsageFlags, memoryPropertyFlags, m_depthImage, m_depthImageMemory);

		m_depthImageView = CreateImageView(m_depthImage, mipLevels, depthFormat, vk::ImageAspectFlagBits::eDepth);

		vk::ImageLayout oldLayout = vk::ImageLayout::eUndefined;
		vk::ImageLayout newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		TransitionImageLayout(m_depthImage, mipLevels, depthFormat, oldLayout, newLayout);
	}

	void VulkanContext::DestroyDepthImage()
	{
		m_device.destroyImageView(m_depthImageView);
		m_device.destroyImage(m_depthImage);
		m_device.freeMemory(m_depthImageMemory);
	}

	void VulkanContext::CreateRenderPass()
	{
		vk::AttachmentDescription colorAttachmentDescription{};
		colorAttachmentDescription.flags = {};
		colorAttachmentDescription.format = m_swapchainSurfaceFormat.format;
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
		depthAttachmentDescription.format = FindDepthFormat();
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

		vk::Result result = m_device.createRenderPass(&renderPassCreateInfo, nullptr, &m_renderPass);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan render pass!");
	}

	void VulkanContext::DestroyRenderPass()
	{
		m_device.destroyRenderPass(m_renderPass);
	}

	void VulkanContext::CreateFramebuffers()
	{
		m_framebuffers.resize(m_swapchainImageViews.size());
		for (size_t i = 0; i < m_swapchainImageViews.size(); i++)
		{
			std::vector<vk::ImageView> attachments = { m_swapchainImageViews[i], m_depthImageView };
			vk::FramebufferCreateInfo framebufferCreateInfo{};
			framebufferCreateInfo.renderPass = m_renderPass;
			framebufferCreateInfo.attachmentCount = attachments.size();
			framebufferCreateInfo.pAttachments = attachments.data();
			framebufferCreateInfo.width = m_swapchainExtent.width;
			framebufferCreateInfo.height = m_swapchainExtent.height;
			framebufferCreateInfo.layers = 1;

			vk::Result result = m_device.createFramebuffer(&framebufferCreateInfo, nullptr, &m_framebuffers[i]);
			FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan framebuffer!");
		}
	}

	void VulkanContext::DestroyFramebuffers()
	{
		for (const vk::Framebuffer& framebuffer : m_framebuffers)
			m_device.destroyFramebuffer(framebuffer);
	}

	void VulkanContext::CreateGraphicsPipeline()
	{
		// SHADER STAGES -------------------------------
		std::vector<char> vertexShaderCode = ReadBinaryFile("assets/shaders/triangle.vert.spv");
		vk::ShaderModule vertexShaderModule;
		vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo{};
		vertexShaderModuleCreateInfo.pNext = nullptr;
		vertexShaderModuleCreateInfo.flags = {};
		vertexShaderModuleCreateInfo.codeSize = vertexShaderCode.size();
		vertexShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexShaderCode.data());
		vk::Result result = m_device.createShaderModule(&vertexShaderModuleCreateInfo, nullptr, &vertexShaderModule);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan vertex shader module!");

		std::vector<char> fragmentShaderCode = ReadBinaryFile("assets/shaders/triangle.frag.spv");
		vk::ShaderModule fragmentShaderModule;
		vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo{};
		fragmentShaderModuleCreateInfo.pNext = nullptr;
		fragmentShaderModuleCreateInfo.flags = {};
		fragmentShaderModuleCreateInfo.codeSize = fragmentShaderCode.size();
		fragmentShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentShaderCode.data());
		result = m_device.createShaderModule(&fragmentShaderModuleCreateInfo, nullptr, &fragmentShaderModule);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan fragment shader module!");

		vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
		vertexShaderStageCreateInfo.pNext = nullptr;
		vertexShaderStageCreateInfo.flags = {};
		vertexShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eVertex;
		vertexShaderStageCreateInfo.module = vertexShaderModule;
		vertexShaderStageCreateInfo.pName = "main";
		vertexShaderStageCreateInfo.pSpecializationInfo = nullptr;

		vk::PipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{};
		fragmentShaderStageCreateInfo.pNext = nullptr;
		fragmentShaderStageCreateInfo.flags = {};
		fragmentShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eFragment;
		fragmentShaderStageCreateInfo.module = fragmentShaderModule;
		fragmentShaderStageCreateInfo.pName = "main";
		fragmentShaderStageCreateInfo.pSpecializationInfo = nullptr;

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };
		// ---------------------------------------------
		// VERTEX INPUT STATE --------------------------
		vk::VertexInputBindingDescription vertexInputBindingDescription{};
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(Mesh::Vertex);
		vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

		std::array<vk::VertexInputAttributeDescription, 5> vertexInputAttributeDescriptions{};
		vertexInputAttributeDescriptions[0].binding = 0;
		vertexInputAttributeDescriptions[0].location = 0;
		vertexInputAttributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[0].offset = offsetof(Mesh::Vertex, position);
		vertexInputAttributeDescriptions[1].binding = 0;
		vertexInputAttributeDescriptions[1].location = 1;
		vertexInputAttributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[1].offset = offsetof(Mesh::Vertex, normal);
		vertexInputAttributeDescriptions[2].binding = 0;
		vertexInputAttributeDescriptions[2].location = 2;
		vertexInputAttributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[2].offset = offsetof(Mesh::Vertex, tangent);
		vertexInputAttributeDescriptions[3].binding = 0;
		vertexInputAttributeDescriptions[3].location = 3;
		vertexInputAttributeDescriptions[3].format = vk::Format::eR32G32B32Sfloat;
		vertexInputAttributeDescriptions[3].offset = offsetof(Mesh::Vertex, bitangent);
		vertexInputAttributeDescriptions[4].binding = 0;
		vertexInputAttributeDescriptions[4].location = 4;
		vertexInputAttributeDescriptions[4].format = vk::Format::eR32G32Sfloat;
		vertexInputAttributeDescriptions[4].offset = offsetof(Mesh::Vertex, texCoords);

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
		vk::Viewport viewport{};
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = (float)m_swapchainExtent.width;
		viewport.height = (float)m_swapchainExtent.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		vk::Rect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_swapchainExtent;

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
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = {};
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		result = m_device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan pipeline layout!");
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
		pipelineCreateInfo.layout = m_pipelineLayout;
		pipelineCreateInfo.renderPass = m_renderPass;
		pipelineCreateInfo.subpass = 0;
		pipelineCreateInfo.basePipelineHandle = nullptr;
		pipelineCreateInfo.basePipelineIndex = -1;

		result = m_device.createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &m_graphicsPipeline);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan graphics pipeline!");
		// ---------------------------------------------
		m_device.destroyShaderModule(vertexShaderModule);
		m_device.destroyShaderModule(fragmentShaderModule);
	}

	void VulkanContext::DestroyGraphicsPipeline()
	{
		m_device.destroyPipeline(m_graphicsPipeline);
		m_device.destroyPipelineLayout(m_pipelineLayout);
	}

	void VulkanContext::CreateSynchronizationPrimitivesForRendering()
	{
		vk::SemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = {};

		vk::FenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		m_isImageAvailableSemaphore.resize(m_swapchainImages.size());
		m_isRenderingFinishedSemaphore.resize(m_swapchainImages.size());
		m_isCommandBufferFinishedFences.resize(m_swapchainImages.size());
		for (size_t i = 0; i < m_swapchainImages.size(); i++)
		{
			vk::Result result = m_device.createSemaphore(&semaphoreCreateInfo, nullptr, &m_isImageAvailableSemaphore[i]);
			FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Semaphore!");

			result = m_device.createSemaphore(&semaphoreCreateInfo, nullptr, &m_isRenderingFinishedSemaphore[i]);
			FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Semaphore!");

			result = m_device.createFence(&fenceCreateInfo, nullptr, &m_isCommandBufferFinishedFences[i]);
			FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Fence!");
		}
	}

	void VulkanContext::DestroySynchronizationPrimitivesForRendering()
	{
		for (size_t i = 0; i < m_swapchainImages.size(); i++)
		{
			m_device.destroyFence(m_isCommandBufferFinishedFences[i]);
			m_device.destroySemaphore(m_isRenderingFinishedSemaphore[i]);
			m_device.destroySemaphore(m_isImageAvailableSemaphore[i]);
		}
	}

	vk::CommandBuffer VulkanContext::BeginOneTimeCommandBuffer()
	{
		vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
		commandBufferAllocateInfo.commandPool = m_commandPool;
		commandBufferAllocateInfo.commandBufferCount = 1;

		vk::CommandBuffer commandBuffer;
		m_device.allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);

		vk::CommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.pNext = nullptr;
		commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;
		commandBuffer.begin(&commandBufferBeginInfo);

		return commandBuffer;
	}

	void VulkanContext::EndCommandBuffer(vk::CommandBuffer commandBuffer)
	{
		commandBuffer.end();

		vk::SubmitInfo submitInfo{};
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;

		vk::Queue graphicsQueue = m_device.getQueue(m_graphicsQueueFamilyIndex, 0);
		graphicsQueue.submit(1, &submitInfo, nullptr);
		graphicsQueue.waitIdle();

		m_device.freeCommandBuffers(m_commandPool, 1, &commandBuffer);
	}

	void VulkanContext::CreateBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags bufferUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
	{
		vk::BufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.flags = {};
		bufferCreateInfo.size = bufferSize;
		bufferCreateInfo.usage = bufferUsageFlags;
		bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		bufferCreateInfo.queueFamilyIndexCount = 0;
		bufferCreateInfo.pQueueFamilyIndices = nullptr;

		if (m_device.createBuffer(&bufferCreateInfo, nullptr, &buffer) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to create Vulkan buffer!");

		vk::MemoryRequirements memoryRequirements;
		m_device.getBufferMemoryRequirements(buffer, &memoryRequirements);

		// Find memory type ----------------------------------------
		vk::PhysicalDeviceMemoryProperties memoryProperties;
		m_physicalDevice.getMemoryProperties(&memoryProperties);
		uint32_t memoryTypeIndex = UINT32_MAX;
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			// IsMemoryTypeSupported && AreMemoryTypePropertyFlagsSupported
			if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
			{
				memoryTypeIndex = i;
				break;
			}
		}
		if (memoryTypeIndex == UINT32_MAX)
			throw std::runtime_error("Failed to find suitable memory type for Vulkan buffer!");
		// ---------------------------------------------------------

		vk::MemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.pNext = nullptr;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

		if (m_device.allocateMemory(&memoryAllocateInfo, nullptr, &bufferMemory) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to allocate Vulkan buffer memory!");

		m_device.bindBufferMemory(buffer, bufferMemory, 0);
	}

	void VulkanContext::CopyBuffer(vk::Buffer sourceBuffer, vk::Buffer destinationBuffer, vk::DeviceSize size)
	{
		vk::CommandBuffer commandBuffer = BeginOneTimeCommandBuffer();

		vk::BufferCopy bufferCopy{};
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;
		bufferCopy.size = size;
		commandBuffer.copyBuffer(sourceBuffer, destinationBuffer, 1, &bufferCopy);

		EndCommandBuffer(commandBuffer);
	}

	void VulkanContext::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits sampleCount, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags imageUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Image& image, vk::DeviceMemory& imageMemory)
	{
		vk::ImageCreateInfo imageCreateInfo{};
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.flags = {};
		imageCreateInfo.imageType = vk::ImageType::e2D;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = mipLevels;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.format = format;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageCreateInfo.usage = imageUsageFlags;
		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		imageCreateInfo.samples = sampleCount;

		if (m_device.createImage(&imageCreateInfo, nullptr, &image) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to create Vulkan image!");

		vk::MemoryRequirements memoryRequirements;
		m_device.getImageMemoryRequirements(image, &memoryRequirements);

		// Find memory type ----------------------------------------
		vk::PhysicalDeviceMemoryProperties memoryProperties;
		m_physicalDevice.getMemoryProperties(&memoryProperties);
		uint32_t memoryTypeIndex = UINT32_MAX;
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			// IsMemoryTypeSupported && AreMemoryTypePropertyFlagsSupported
			if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
			{
				memoryTypeIndex = i;
				break;
			}
		}
		if (memoryTypeIndex == UINT32_MAX)
			throw std::runtime_error("Failed to find suitable memory type for Vulkan buffer!");
		// ---------------------------------------------------------

		vk::MemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

		if (m_device.allocateMemory(&memoryAllocateInfo, nullptr, &imageMemory) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to allocate Vulkan image memory!");

		vkBindImageMemory(m_device, image, imageMemory, 0);
	}

	vk::ImageView VulkanContext::CreateImageView(vk::Image image, uint32_t mipLevels, vk::Format format, vk::ImageAspectFlags imageAspectFlags)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = {};
		imageViewCreateInfo.image = image;
		imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.subresourceRange.aspectMask = imageAspectFlags;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = mipLevels;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		vk::ImageView imageView;
		vk::Result result = m_device.createImageView(&imageViewCreateInfo, nullptr, &imageView);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan image view!");

		return imageView;
	}

	void VulkanContext::TransitionImageLayout(vk::Image image, uint32_t mipLevels, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
	{
		vk::CommandBuffer commandBuffer = BeginOneTimeCommandBuffer();

		vk::ImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.pNext = nullptr;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 1;

		vk::PipelineStageFlags sourcePipelineStageFlags;
		vk::PipelineStageFlags destinationPipelineStageFlags;

		if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

			if (HasStencilComponent(format))
				imageMemoryBarrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
		else
		{
			imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		}

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			imageMemoryBarrier.srcAccessMask = {};
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourcePipelineStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationPipelineStageFlags = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourcePipelineStageFlags = vk::PipelineStageFlagBits::eTransfer;
			destinationPipelineStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			imageMemoryBarrier.srcAccessMask = {};
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

			sourcePipelineStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationPipelineStageFlags = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else
			throw std::invalid_argument("Unsupported layout transition!");

		commandBuffer.pipelineBarrier(sourcePipelineStageFlags, destinationPipelineStageFlags, {}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		EndCommandBuffer(commandBuffer);
	}

	vk::Format VulkanContext::FindSupportedFormat(const std::vector<vk::Format>& formatCandidates, vk::ImageTiling tiling, vk::FormatFeatureFlags formatFeatureFlags)
	{
		for (vk::Format format : formatCandidates)
		{
			vk::FormatProperties formatProperties;
			m_physicalDevice.getFormatProperties(format, &formatProperties);

			if (tiling == vk::ImageTiling::eLinear && (formatProperties.linearTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
				return format;
			else if (tiling == vk::ImageTiling::eOptimal && (formatProperties.optimalTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
				return format;
		}

		throw std::runtime_error("Failed to find supported format!");
	}

	vk::Format VulkanContext::FindDepthFormat()
	{
		std::vector<vk::Format> formatCandidates = { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint };
		vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
		vk::FormatFeatureFlags formatFeatureFlags = vk::FormatFeatureFlagBits::eDepthStencilAttachment;
		return FindSupportedFormat(formatCandidates, tiling, formatFeatureFlags);
	}

	bool VulkanContext::HasStencilComponent(vk::Format format)
	{
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}

	std::vector<char> ReadBinaryFile(const std::string& fileName)
	{
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("Failed to open binary file!");

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> fileBytes(fileSize);
		file.seekg(0);
		file.read(fileBytes.data(), fileSize);
		file.close();

		return fileBytes;
	}

	std::vector<const char*> VulkanContext::GetRequiredInstanceExtensions() const
	{
		std::vector<const char*> requiredInstanceExtensions;
		if (AreValidationLayersEnabled())
			requiredInstanceExtensions.push_back("VK_EXT_debug_utils");

		uint32_t glfwInstanceExtensionCount = 0;
		const char** glfwInstanceExtensionNames;
		glfwInstanceExtensionNames = glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionCount);
		for (size_t i = 0; i < glfwInstanceExtensionCount; i++)
			requiredInstanceExtensions.push_back(glfwInstanceExtensionNames[i]);

		return requiredInstanceExtensions;
	}

	std::vector<const char*> VulkanContext::GetRequiredInstanceLayers() const
	{
		std::vector<const char*> requiredInstanceLayers;
		if (AreValidationLayersEnabled())
			requiredInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");

		return requiredInstanceLayers;
	}

	std::vector<const char*> VulkanContext::GetRequiredDeviceExtensions() const
	{
		std::vector<const char*> requiredDeviceExtensions;
		requiredDeviceExtensions.push_back("VK_KHR_swapchain");

		return requiredDeviceExtensions;
	}

	std::vector<const char*> VulkanContext::GetRequiredDeviceLayers() const
	{
		std::vector<const char*> requiredDeviceLayers;
		return requiredDeviceLayers;
	}

	constexpr bool VulkanContext::AreValidationLayersEnabled() const
	{
#ifdef NDEBUG
		return false;
#else 
		return true;
#endif
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugMessengerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::string messageTypeLabel;
		switch (messageType)
		{
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			messageTypeLabel = "general";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			messageTypeLabel = "validation";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			messageTypeLabel = "performance";
			break;
		}

		std::string message = "[" + messageTypeLabel + "] debug message: " + std::string(pCallbackData->pMessage);
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			Logger::Warn("Vulkan", message);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			Logger::Error("Vulkan", message);
			break;
		}
		return VK_FALSE;
	}

	std::vector<char> VulkanContext::ReadBinaryFile(const std::string& fileName)
	{
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);
		FIREFLY_ASSERT(file.is_open(), "Failed to open binary file: {0}!", fileName);

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> fileBytes(fileSize);
		file.seekg(0);
		file.read(fileBytes.data(), fileSize);
		file.close();

		return fileBytes;
	}
}