#include "pch.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

#include "Rendering/RenderingAPI.h"

namespace Firefly
{
    void VulkanSwapchain::Init(vk::Device device, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, uint32_t width, uint32_t height)
    {
        m_device = device;
        m_physicalDevice = physicalDevice;

        m_swapchainData.imageCount = 2;
        m_swapchainData.presentMode = vk::PresentModeKHR::eFifoRelaxed;
        m_swapchainData.imageFormat = vk::Format::eR8G8B8A8Srgb;
        m_swapchainData.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

        vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(surface);
        if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
        {
            m_swapchainData.extent = surfaceCapabilities.currentExtent;
        }
        else
        {
            vk::Extent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
            actualExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, actualExtent.height));
            m_swapchainData.extent = actualExtent;
        }

        m_swapchain = VulkanUtils::CreateSwapchain(m_device, m_physicalDevice, surface, m_swapchainData);

        m_images = m_device.getSwapchainImagesKHR(m_swapchain);

        m_imageViews.resize(m_images.size());
        uint32_t mipLevels = 1;
        for (size_t i = 0; i < m_images.size(); i++)
            m_imageViews[i] = VulkanUtils::CreateImageView(m_device, m_images[i], mipLevels, m_swapchainData.imageFormat, vk::ImageAspectFlagBits::eColor);
    }

    void VulkanSwapchain::Destroy()
    {
        for (const vk::ImageView& imageView : m_imageViews)
            m_device.destroyImageView(imageView);
        m_device.destroySwapchainKHR(m_swapchain);
    }

    vk::SwapchainKHR VulkanSwapchain::GetHandle() const
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

    vk::Format VulkanSwapchain::GetImageFormat() const
    {
        return m_swapchainData.imageFormat;
    }

    vk::ColorSpaceKHR VulkanSwapchain::GetColorSpace() const
    {
        return m_swapchainData.colorSpace;
    }

    vk::PresentModeKHR VulkanSwapchain::GetPresentMode() const
    {
        return m_swapchainData.presentMode;
    }

    vk::Extent2D VulkanSwapchain::GetExtent() const
    {
        return m_swapchainData.extent;
    }
}