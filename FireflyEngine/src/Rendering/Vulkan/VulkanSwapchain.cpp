#include "pch.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

#include "Rendering/Vulkan/VulkanContext.h"

namespace Firefly
{
	VulkanSwapchain::VulkanSwapchain(VulkanDevice* device, VulkanSurface* surface) :
		m_device(device->GetDevice()),
		m_physicalDevice(device->GetPhysicalDevice())
	{
		std::vector<vk::SurfaceFormatKHR> surfaceFormats = m_physicalDevice.getSurfaceFormatsKHR(surface->GetSurface());
		for (const vk::SurfaceFormatKHR& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				m_format = surfaceFormat;
				break;
			}
			else
			{
				m_format = surfaceFormats[0];
			}
		}

		std::vector<vk::PresentModeKHR> surfacePresentModes = m_physicalDevice.getSurfacePresentModesKHR(surface->GetSurface());
		for (const vk::PresentModeKHR& surfacePresentMode : surfacePresentModes)
		{
			if (surfacePresentMode == vk::PresentModeKHR::eFifoRelaxed)
			{
				m_presentMode = surfacePresentMode;
				break;
			}
			else
			{
				m_presentMode = vk::PresentModeKHR::eMailbox;
			}
		}
		
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(surface->GetSurface());
		if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
		{
			m_extent = surfaceCapabilities.currentExtent;
		}
		else
		{
			vk::Extent2D actualExtent = { static_cast<uint32_t>(surface->GetWidth()), static_cast<uint32_t>(surface->GetHeight()) };
			actualExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, actualExtent.height));
			m_extent = actualExtent;
		}

		uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
		if (surfaceCapabilities.maxImageCount > 0)
			imageCount = std::min(imageCount, surfaceCapabilities.maxImageCount);

		vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.flags = {};
		swapchainCreateInfo.surface = surface->GetSurface();
		swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
		swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchainCreateInfo.presentMode = m_presentMode;
		swapchainCreateInfo.clipped = true;
		swapchainCreateInfo.oldSwapchain = nullptr;
		swapchainCreateInfo.minImageCount = imageCount;
		swapchainCreateInfo.imageFormat = m_format.format;
		swapchainCreateInfo.imageColorSpace = m_format.colorSpace;
		swapchainCreateInfo.imageExtent = m_extent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		std::vector<uint32_t> queueFamilyIndices = { device->GetGraphicsQueueFamilyIndex(), device->GetPresentQueueFamilyIndex() };
		if (device->GetGraphicsQueueFamilyIndex() != device->GetPresentQueueFamilyIndex())
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

		m_images = m_device.getSwapchainImagesKHR(m_swapchain);

		m_imageViews.resize(m_images.size());
		uint32_t mipLevels = 1;
		for (size_t i = 0; i < m_images.size(); i++)
			m_imageViews[i] = VulkanContext::GetSingleton()->CreateImageView(m_images[i], mipLevels, m_format.format, vk::ImageAspectFlagBits::eColor);
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		for (const vk::ImageView& imageView : m_imageViews)
			m_device.destroyImageView(imageView);
		m_device.destroySwapchainKHR(m_swapchain);
	}

	vk::SwapchainKHR VulkanSwapchain::GetSwapchain() const
	{
		return m_swapchain;
	}

	uint32_t VulkanSwapchain::GetImageCount() const
	{
		return m_images.size();
	}

	std::vector<vk::ImageView> VulkanSwapchain::GetImageViews() const
	{
		return m_imageViews;
	}

	vk::SurfaceFormatKHR VulkanSwapchain::GetFormat() const
	{
		return m_format;
	}

	vk::PresentModeKHR VulkanSwapchain::GetPresentMode() const
	{
		return m_presentMode;
	}

	vk::Extent2D VulkanSwapchain::GetExtent() const
	{
		return m_extent;
	}
}