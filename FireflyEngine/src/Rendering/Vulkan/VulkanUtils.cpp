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
}