#include "pch.h"
#include "Rendering/Vulkan/VulkanUtils.h"

#include <GLFW/glfw3.h>

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
							const std::vector<vk::DeviceQueueCreateInfo>& queueCreateInfos)
	{
		// TODO: Check required device extensions and layers
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
		if(!isPresentModeSupported)
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