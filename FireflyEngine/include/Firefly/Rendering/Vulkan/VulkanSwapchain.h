#pragma once

#include "Rendering/Vulkan/VulkanDevice.h"
#include "Rendering/Vulkan/VulkanUtils.h"

namespace Firefly
{
	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(VulkanDevice* device, VulkanSurface* surface);
		~VulkanSwapchain();

		vk::SwapchainKHR GetSwapchain() const;
		uint32_t GetImageCount() const;
		std::vector<vk::ImageView> GetImageViews() const;

		vk::Format GetImageFormat() const;
		vk::ColorSpaceKHR GetColorSpace() const;
		vk::PresentModeKHR GetPresentMode() const;
		vk::Extent2D GetExtent() const;

	private:
		vk::SwapchainKHR m_swapchain;
		vk::Device m_device;
		vk::PhysicalDevice m_physicalDevice;

		std::vector<vk::Image> m_images;
		std::vector<vk::ImageView> m_imageViews;

		VulkanUtils::SwapchainData m_swapchainData;
	};
}