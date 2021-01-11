#pragma once

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace Firefly::VulkanUtils
{
	vk::Instance CreateInstance(const std::string& appName, uint32_t appVersion,
								const std::string& engineName, uint32_t engineVersion, uint32_t apiVersion,
								const std::vector<const char*>& requiredInstanceExtensions,
								const std::vector<const char*>& requiredInstanceLayers);

	vk::SurfaceKHR CreateSurface(vk::Instance instance, GLFWwindow* window);

	vk::Device CreateDevice(vk::PhysicalDevice physicalDevice,
							const std::vector<const char*>& requiredDeviceExtensions,
							const std::vector<const char*>& requiredDeviceLayers,
							const std::vector<vk::DeviceQueueCreateInfo>& queueCreateInfos);

	struct SwapchainData
	{
		uint32_t imageCount;
		vk::Extent2D extent;
		vk::PresentModeKHR presentMode;
		vk::Format imageFormat;
		vk::ColorSpaceKHR colorSpace;
	};

	vk::SwapchainKHR CreateSwapchain(vk::Device device, vk::PhysicalDevice physicalDevice, 
									 vk::SurfaceKHR surface, SwapchainData& swapchainData);

	vk::CommandBuffer BeginOneTimeCommandBuffer(vk::Device device, vk::CommandPool commandPool);
	void EndCommandBuffer(vk::Device device, vk::CommandBuffer commandBuffer, vk::CommandPool commandPool, vk::Queue queue);
	void CreateBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DeviceSize bufferSize, vk::BufferUsageFlags bufferUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
	void CopyBuffer(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::Buffer sourceBuffer, vk::Buffer destinationBuffer, vk::DeviceSize size);
	void CreateImage(vk::Device device, vk::PhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits sampleCount, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags imageUsageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Image& image, vk::DeviceMemory& imageMemory);
	vk::ImageView CreateImageView(vk::Device device, vk::Image image, uint32_t mipLevels, vk::Format format, vk::ImageAspectFlags imageAspectFlags);
	void TransitionImageLayout(vk::Device device, vk::CommandPool commandPool, vk::Queue queue, vk::Image image, uint32_t mipLevels, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	vk::Format FindSupportedFormat(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& formatCandidates, vk::ImageTiling tiling, vk::FormatFeatureFlags formatFeatureFlags);
	vk::Format FindDepthFormat(vk::PhysicalDevice physicalDevice);
	bool HasStencilComponent(vk::Format format);
}