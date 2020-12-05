#include "Rendering/Vulkan/VulkanContext.h"

#include <GLFW/glfw3.h>
#include <fstream>

namespace Firefly
{
	VulkanContext::VulkanContext(void* window)
	{
		m_glfwWindow = (GLFWwindow*)(window);
		FIREFLY_ASSERT(m_glfwWindow, "Vulkan requires a GLFWwindow pointer!");

		m_vertices = {
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}
		};
		m_indices = { 0, 1, 2, 2, 3, 0 };

		CreateInstance();
		CreateDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateDevice();
		CreateSwapchain();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateCommandPool();
		AllocateCommandBuffers();
		CreateSynchronizationPrimitivesForRendering();
		CreateVertexBuffer();
		CreateIndexBuffer();
	}

	VulkanContext::~VulkanContext()
	{
		m_device.waitIdle();

		DestroyIndexBuffer();
		DestroyVertexBuffer();
		DestroySynchronizationPrimitivesForRendering();
		FreeCommandBuffers();
		DestroyCommandPool();
		DestroyGraphicsPipeline();
		DestroyRenderPass();
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

		vk::CommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		result = m_commandBuffers[currentImageIndex].begin(&commandBufferBeginInfo);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to begin recording Vulkan command buffer!");

		std::array<vk::ClearValue, 1> clearValues = {}; // order of clear values needs to be in the order of attachments
		clearValues[0].color = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f };
		//clearValues[1].depthStencil = { 1.0f, 0 };

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

		std::vector<vk::Buffer> vertexBuffers = { m_vertexBuffer };
		vk::DeviceSize offsets[] = { 0 };
		m_commandBuffers[currentImageIndex].bindVertexBuffers(0, vertexBuffers.size(), vertexBuffers.data(), offsets);
		m_commandBuffers[currentImageIndex].bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint32);

		m_commandBuffers[currentImageIndex].drawIndexed(m_indices.size(), 1, 0, 0, 0);

		m_commandBuffers[currentImageIndex].endRenderPass();
		m_commandBuffers[currentImageIndex].end();

		std::vector<vk::Semaphore> isImageAvailableSemaphores = { m_isImageAvailableSemaphore[m_currentFrameIndex] };
		std::vector<vk::Semaphore> isRenderingFinishedSemaphores = { m_isRenderingFinishedSemaphore[m_currentFrameIndex] };
		vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eTopOfPipe; //vk::PipelineStageFlagBits::eColorAttachmentOutput;
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
		FreeCommandBuffers();
		DestroyGraphicsPipeline();
		DestroyRenderPass();
		DestroySwapchain();

		CreateSwapchain();
		CreateRenderPass();
		CreateGraphicsPipeline();
		AllocateCommandBuffers();
		CreateSynchronizationPrimitivesForRendering();
	}

	void VulkanContext::DestroySwapchain()
	{
		for (const vk::ImageView& imageView : m_swapchainImageViews)
			m_device.destroyImageView(imageView);
		m_device.destroySwapchainKHR(m_swapchain);
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

		vk::SubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpassDescription.flags = {};
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentReference;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.pDepthStencilAttachment = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		vk::RenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.pNext = nullptr;
		renderPassCreateInfo.flags = {};
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &colorAttachmentDescription;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 0;
		renderPassCreateInfo.pDependencies = nullptr;

		vk::Result result = m_device.createRenderPass(&renderPassCreateInfo, nullptr, &m_renderPass);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan render pass!");

		m_framebuffers.resize(m_swapchainImageViews.size());
		for (size_t i = 0; i < m_swapchainImageViews.size(); i++) 
		{
			std::vector<vk::ImageView> attachments = { m_swapchainImageViews[i] };
			vk::FramebufferCreateInfo framebufferCreateInfo{};
			framebufferCreateInfo.renderPass = m_renderPass;
			framebufferCreateInfo.attachmentCount = attachments.size();
			framebufferCreateInfo.pAttachments = attachments.data();
			framebufferCreateInfo.width = m_swapchainExtent.width;
			framebufferCreateInfo.height = m_swapchainExtent.height;
			framebufferCreateInfo.layers = 1;

			result = m_device.createFramebuffer(&framebufferCreateInfo, nullptr, &m_framebuffers[i]);
			FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan framebuffer!");
		}
	}

	void VulkanContext::DestroyRenderPass()
	{
		for (const vk::Framebuffer& framebuffer : m_framebuffers)
			m_device.destroyFramebuffer(framebuffer);
		m_device.destroyRenderPass(m_renderPass);
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
		rasterizationStateCreateInfo.frontFace = vk::FrontFace::eClockwise;
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
		//vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
		//depthStencilStateCreateInfo.pNext = nullptr;
		//depthStencilStateCreateInfo.flags = {};
		//depthStencilStateCreateInfo.depthTestEnable = true;
		//depthStencilStateCreateInfo.depthWriteEnable = true;
		//depthStencilStateCreateInfo.depthCompareOp = vk::CompareOp::eLess;
		//depthStencilStateCreateInfo.depthBoundsTestEnable = false;
		//depthStencilStateCreateInfo.minDepthBounds = 0.0f;
		//depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
		//depthStencilStateCreateInfo.stencilTestEnable = false;
		//depthStencilStateCreateInfo.front = {};
		//depthStencilStateCreateInfo.back = {};
		// ---------------------------------------------
		// PIPELINE LAYOUT -----------------------------
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = {};
		pipelineLayoutCreateInfo.setLayoutCount = 0;
		pipelineLayoutCreateInfo.pSetLayouts = nullptr;
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
		pipelineCreateInfo.pDepthStencilState = nullptr;
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
		m_commandBuffers.resize(m_framebuffers.size());
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

	void VulkanContext::CreateVertexBuffer()
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

	void VulkanContext::DestroyVertexBuffer()
	{
		m_device.destroyBuffer(m_vertexBuffer);
		m_device.freeMemory(m_vertexBufferMemory);
	}

	void VulkanContext::CreateIndexBuffer()
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

	void VulkanContext::DestroyIndexBuffer()
	{
		m_device.destroyBuffer(m_indexBuffer);
		m_device.freeMemory(m_indexBufferMemory);
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