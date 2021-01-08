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
}