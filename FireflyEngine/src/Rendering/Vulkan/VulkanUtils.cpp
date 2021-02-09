#include "pch.h"
#include "Rendering/Vulkan/VulkanUtils.h"

#include <GLFW/glfw3.h>
#include "Rendering/RenderingAPI.h"
#include "Rendering/Vulkan/VulkanContext.h"
#include "Rendering/Vulkan/VulkanTexture.h"

namespace Firefly::VulkanUtils
{
	vk::Instance CreateInstance(const std::string& appName, uint32_t appVersion,
		const std::string& engineName, uint32_t engineVersion, uint32_t apiVersion,
		const std::vector<const char*>& requiredInstanceExtensions,
		const std::vector<const char*>& requiredInstanceLayers)
	{
		vk::ApplicationInfo applicationInfo{};
		applicationInfo.pNext = nullptr;
		applicationInfo.pApplicationName = appName.c_str();
		applicationInfo.applicationVersion = appVersion;
		applicationInfo.pEngineName = engineName.c_str();
		applicationInfo.engineVersion = engineVersion;
		applicationInfo.apiVersion = apiVersion;

		// TODO: Check required instance extensions and layers
		vk::InstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.pNext = nullptr;
		instanceCreateInfo.flags = {};
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.enabledExtensionCount = requiredInstanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();
		instanceCreateInfo.enabledLayerCount = requiredInstanceLayers.size();
		instanceCreateInfo.ppEnabledLayerNames = requiredInstanceLayers.data();

		vk::Instance instance;
		vk::Result result = vk::createInstance(&instanceCreateInfo, nullptr, &instance);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan instance!");
		return instance;
	}

	vk::SurfaceKHR CreateSurface(vk::Instance instance, GLFWwindow* window)
	{
		VkSurfaceKHR surface;
		vk::Result result = vk::Result(glfwCreateWindowSurface(instance, window, nullptr, &surface));
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan window surface!");
		return vk::SurfaceKHR(surface);
	}

	vk::Device CreateDevice(vk::PhysicalDevice physicalDevice,
		const std::vector<const char*>& requiredDeviceExtensions,
		const std::vector<const char*>& requiredDeviceLayers,
		vk::PhysicalDeviceFeatures deviceFeatures,
		const std::vector<vk::DeviceQueueCreateInfo>& queueCreateInfos)
	{
		vk::PhysicalDeviceFeatures2 deviceFeatures2{};
		deviceFeatures2.pNext = nullptr;
		deviceFeatures2.features = deviceFeatures;

		return CreateDevice(physicalDevice, requiredDeviceExtensions, requiredDeviceLayers, deviceFeatures2, queueCreateInfos);
	}

	vk::Device CreateDevice(vk::PhysicalDevice physicalDevice,
		const std::vector<const char*>& requiredDeviceExtensions,
		const std::vector<const char*>& requiredDeviceLayers,
		vk::PhysicalDeviceFeatures2 deviceFeatures2,
		const std::vector<vk::DeviceQueueCreateInfo>& queueCreateInfos)
	{
		// TODO: Check required device extensions and layers
		vk::DeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.pNext = &deviceFeatures2;
		deviceCreateInfo.flags = {};
		deviceCreateInfo.enabledExtensionCount = requiredDeviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
		deviceCreateInfo.enabledLayerCount = requiredDeviceLayers.size();
		deviceCreateInfo.ppEnabledLayerNames = requiredDeviceLayers.data();
		deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = nullptr;

		vk::Device device;
		vk::Result result = physicalDevice.createDevice(&deviceCreateInfo, nullptr, &device);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan logical device!");
		return device;
	}

	vk::SwapchainKHR CreateSwapchain(vk::Device device, vk::PhysicalDevice physicalDevice,
		vk::SurfaceKHR surface, SwapchainData& swapchainData)
	{
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
		swapchainData.imageCount = std::max(surfaceCapabilities.minImageCount, std::min(surfaceCapabilities.maxImageCount, swapchainData.imageCount));

		bool isSurfaceFormatSupported = false;
		std::vector<vk::SurfaceFormatKHR> supportedSurfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);
		for (const vk::SurfaceFormatKHR& supportedSurfaceFormat : supportedSurfaceFormats)
		{
			if (swapchainData.imageFormat == supportedSurfaceFormat.format && swapchainData.colorSpace == supportedSurfaceFormat.colorSpace)
			{
				isSurfaceFormatSupported = true;
				break;
			}
		}
		if (!isSurfaceFormatSupported)
			swapchainData.imageFormat = supportedSurfaceFormats[0].format;
		swapchainData.colorSpace = supportedSurfaceFormats[0].colorSpace;

		bool isPresentModeSupported = false;
		std::vector<vk::PresentModeKHR> surfacePresentModes = physicalDevice.getSurfacePresentModesKHR(surface);
		for (const vk::PresentModeKHR& surfacePresentMode : surfacePresentModes)
		{
			if (swapchainData.presentMode == surfacePresentMode)
			{
				isPresentModeSupported = true;
				break;
			}
		}
		if (!isPresentModeSupported)
			swapchainData.presentMode = vk::PresentModeKHR::eMailbox;

		vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.flags = {};
		swapchainCreateInfo.surface = surface;
		swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
		swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchainCreateInfo.presentMode = swapchainData.presentMode;
		swapchainCreateInfo.clipped = true;
		swapchainCreateInfo.oldSwapchain = nullptr;
		swapchainCreateInfo.minImageCount = swapchainData.imageCount;
		swapchainCreateInfo.imageFormat = swapchainData.imageFormat;
		swapchainCreateInfo.imageColorSpace = swapchainData.colorSpace;
		swapchainCreateInfo.imageExtent = swapchainData.extent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;

		vk::SwapchainKHR swapchain;
		vk::Result result = device.createSwapchainKHR(&swapchainCreateInfo, nullptr, &swapchain);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan swapchain!");
		return swapchain;
	}

	vk::PipelineLayout CreatePipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
	{
		std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
		vk::Device device = vkContext->GetDevice()->GetHandle();

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = {};
		pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		vk::PipelineLayout pipelineLayout;
		vk::Result result = device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan pipeline layout!");

		return pipelineLayout;
	}

	vk::Pipeline CreatePipeline(vk::PipelineLayout layout, std::shared_ptr<VulkanRenderPass> renderPass, std::shared_ptr<VulkanShader> shader, vk::FrontFace frontFace)
	{
		std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
		vk::Device device = vkContext->GetDevice()->GetHandle();

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
		vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{};
		viewportStateCreateInfo.pNext = nullptr;
		viewportStateCreateInfo.flags = {};
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pViewports = nullptr;
		viewportStateCreateInfo.scissorCount = 1;
		viewportStateCreateInfo.pScissors = nullptr;
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
		rasterizationStateCreateInfo.frontFace = frontFace;
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
		multisampleStateCreateInfo.rasterizationSamples = VulkanTexture::ConvertToVulkanSampleCount(renderPass->GetSampleCount());
		multisampleStateCreateInfo.sampleShadingEnable = renderPass->IsSampleShadingEnabled();
		multisampleStateCreateInfo.minSampleShading = renderPass->GetMinSampleShading();
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
		depthStencilStateCreateInfo.depthTestEnable = renderPass->isDepthTestingEnabled();
		depthStencilStateCreateInfo.depthWriteEnable = renderPass->isDepthTestingEnabled();
		depthStencilStateCreateInfo.depthCompareOp = VulkanRenderPass::ConvertToVulkanCompareOperation(renderPass->GetDepthCompareOperation());
		depthStencilStateCreateInfo.depthBoundsTestEnable = false;
		depthStencilStateCreateInfo.minDepthBounds = 0.0f;
		depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
		depthStencilStateCreateInfo.stencilTestEnable = false;
		depthStencilStateCreateInfo.front = {};
		depthStencilStateCreateInfo.back = {};
		// ---------------------------------------------
		// DYNAMIC STATE -------------------------------
		std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
		dynamicStateCreateInfo.pNext = nullptr;
		dynamicStateCreateInfo.flags = {};
		dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
		dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
		// ---------------------------------------------
		// SHADER STAGE STATE --------------------------
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos = shader->GetShaderStageCreateInfos();
		// ---------------------------------------------
		// GRAPHICS PIPELINE ---------------------------s
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
		pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
		pipelineCreateInfo.layout = layout;
		pipelineCreateInfo.renderPass = renderPass->GetHandle();
		pipelineCreateInfo.subpass = 0;
		pipelineCreateInfo.basePipelineHandle = nullptr;
		pipelineCreateInfo.basePipelineIndex = -1;

		vk::Pipeline pipeline;
		vk::Result result = device.createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan graphics pipeline!");
		// ---------------------------------------------

		return pipeline;
	}

	vk::CommandBuffer BeginOneTimeCommandBuffer(vk::Device device, vk::CommandPool commandPool)
	{
		vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
		commandBufferAllocateInfo.commandPool = commandPool;
		commandBufferAllocateInfo.commandBufferCount = 1;

		vk::CommandBuffer commandBuffer;
		device.allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);

		vk::CommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.pNext = nullptr;
		commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;
		commandBuffer.begin(&commandBufferBeginInfo);

		return commandBuffer;
	}

	void EndCommandBuffer(vk::Device device, vk::CommandBuffer commandBuffer, vk::CommandPool commandPool, vk::Queue queue)
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

		queue.submit(1, &submitInfo, nullptr);
		queue.waitIdle();

		device.freeCommandBuffers(commandPool, 1, &commandBuffer);
	}

	void CreateBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DeviceSize bufferSize, vk::BufferUsageFlags bufferUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
	{
		vk::BufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.flags = {};
		bufferCreateInfo.size = bufferSize;
		bufferCreateInfo.usage = bufferUsageFlags;
		bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		bufferCreateInfo.queueFamilyIndexCount = 0;
		bufferCreateInfo.pQueueFamilyIndices = nullptr;

		if (device.createBuffer(&bufferCreateInfo, nullptr, &buffer) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to create Vulkan buffer!");

		vk::MemoryRequirements memoryRequirements;
		device.getBufferMemoryRequirements(buffer, &memoryRequirements);

		// Find memory type ----------------------------------------
		vk::PhysicalDeviceMemoryProperties memoryProperties;
		physicalDevice.getMemoryProperties(&memoryProperties);
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

		if (device.allocateMemory(&memoryAllocateInfo, nullptr, &bufferMemory) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to allocate Vulkan buffer memory!");

		device.bindBufferMemory(buffer, bufferMemory, 0);
	}

	void CopyBuffer(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::Buffer sourceBuffer, vk::Buffer destinationBuffer, vk::DeviceSize size)
	{
		vk::CommandBuffer commandBuffer = BeginOneTimeCommandBuffer(device, commandPool);

		vk::BufferCopy bufferCopy{};
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;
		bufferCopy.size = size;
		commandBuffer.copyBuffer(sourceBuffer, destinationBuffer, 1, &bufferCopy);

		EndCommandBuffer(device, commandBuffer, commandPool, queue);
	}

	void CreateImage(vk::Device device, vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits sampleCount, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags imageUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Image& image, vk::DeviceMemory& imageMemory)
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

		if (device.createImage(&imageCreateInfo, nullptr, &image) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to create Vulkan image!");

		vk::MemoryRequirements memoryRequirements;
		device.getImageMemoryRequirements(image, &memoryRequirements);

		// Find memory type ----------------------------------------
		vk::PhysicalDeviceMemoryProperties memoryProperties;
		physicalDevice.getMemoryProperties(&memoryProperties);
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

		if (device.allocateMemory(&memoryAllocateInfo, nullptr, &imageMemory) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to allocate Vulkan image memory!");

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	vk::ImageView CreateImageView(vk::Device device, vk::Image image, uint32_t mipLevels, vk::Format format, vk::ImageAspectFlags imageAspectFlags)
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
		vk::Result result = device.createImageView(&imageViewCreateInfo, nullptr, &imageView);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan image view!");

		return imageView;
	}

	void TransitionImageLayout(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::Image image, uint32_t mipLevels, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
	{
		vk::CommandBuffer commandBuffer = BeginOneTimeCommandBuffer(device, commandPool);

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

		EndCommandBuffer(device, commandBuffer, commandPool, queue);
	}

	void CopyBufferToImage(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::Buffer sourceBuffer, vk::Image destinationImage, uint32_t width, uint32_t height)
	{
		vk::CommandBuffer commandBuffer = BeginOneTimeCommandBuffer(device, commandPool);

		vk::BufferImageCopy bufferImageCopy{};
		bufferImageCopy.bufferOffset = 0;
		bufferImageCopy.bufferRowLength = 0;
		bufferImageCopy.bufferImageHeight = 0;
		bufferImageCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		bufferImageCopy.imageSubresource.mipLevel = 0;
		bufferImageCopy.imageSubresource.baseArrayLayer = 0;
		bufferImageCopy.imageSubresource.layerCount = 1;
		bufferImageCopy.imageOffset = { 0, 0, 0 };
		bufferImageCopy.imageExtent = { width, height, 1 };
		commandBuffer.copyBufferToImage(sourceBuffer, destinationImage, vk::ImageLayout::eTransferDstOptimal, 1, &bufferImageCopy);

		EndCommandBuffer(device, commandBuffer, commandPool, queue);
	}

	void GenerateMipmaps(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool commandPool, vk::Queue queue, vk::Image image, vk::Format format, int32_t width, int32_t height, uint32_t mipLevels)
	{
		// Check if image format supports linear blitting
		vk::FormatProperties formatProperties;
		physicalDevice.getFormatProperties(format, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
			throw std::runtime_error("Texture image format does not support linear blitting!");

		vk::CommandBuffer commandBuffer = BeginOneTimeCommandBuffer(device, commandPool);

		vk::ImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.pNext = nullptr;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 1;
		imageMemoryBarrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = width;
		int32_t mipHeight = height;
		for (uint32_t i = 1; i < mipLevels; i++)
		{
			imageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
			imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			imageMemoryBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			vk::ImageBlit imageBlit{};
			imageBlit.srcOffsets[0] = { 0, 0, 0 };
			imageBlit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			imageBlit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			imageBlit.srcSubresource.mipLevel = i - 1;
			imageBlit.srcSubresource.baseArrayLayer = 0;
			imageBlit.srcSubresource.layerCount = 1;
			imageBlit.dstOffsets[0] = { 0, 0, 0 };
			imageBlit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			imageBlit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;;
			imageBlit.dstSubresource.mipLevel = i;
			imageBlit.dstSubresource.baseArrayLayer = 0;
			imageBlit.dstSubresource.layerCount = 1;
			commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, 1, &imageBlit, vk::Filter::eLinear);

			imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			imageMemoryBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			if (mipWidth > 1)
				mipWidth /= 2;
			if (mipHeight > 1)
				mipHeight /= 2;
		}

		imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
		imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		imageMemoryBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		EndCommandBuffer(device, commandBuffer, commandPool, queue);
	}

	vk::Format FindSupportedFormat(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& formatCandidates, vk::ImageTiling tiling, vk::FormatFeatureFlags formatFeatureFlags)
	{
		for (vk::Format format : formatCandidates)
		{
			vk::FormatProperties formatProperties;
			physicalDevice.getFormatProperties(format, &formatProperties);

			if (tiling == vk::ImageTiling::eLinear && (formatProperties.linearTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
				return format;
			else if (tiling == vk::ImageTiling::eOptimal && (formatProperties.optimalTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
				return format;
		}

		throw std::runtime_error("Failed to find supported format!");
	}

	vk::Format FindDepthFormat(vk::PhysicalDevice physicalDevice)
	{
		std::vector<vk::Format> formatCandidates = { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint };
		vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
		vk::FormatFeatureFlags formatFeatureFlags = vk::FormatFeatureFlagBits::eDepthStencilAttachment;
		return FindSupportedFormat(physicalDevice, formatCandidates, tiling, formatFeatureFlags);
	}

	bool HasStencilComponent(vk::Format format)
	{
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}
}