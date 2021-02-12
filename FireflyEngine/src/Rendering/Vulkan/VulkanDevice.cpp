#include "pch.h"
#include "Rendering/Vulkan/VulkanDevice.h"

#include "Rendering/Vulkan/VulkanUtils.h"

namespace Firefly
{
    void VulkanDevice::Init(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface,
        const std::vector<const char*>& requiredDeviceExtensions,
        const std::vector<const char*>& requiredDeviceLayers)
    {
        m_physicalDevice = physicalDevice;

        FindRequiredQueueFamilyIndices(surface);

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        if (m_graphicsQueueFamilyIndex == m_presentQueueFamilyIndex)
        {
            m_graphicsQueueIndex = 0;
            m_presentQueueIndex = 1;

            float queuePriorities[2] = { 1.0f, 1.0f };
            vk::DeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.pNext = nullptr;
            queueCreateInfo.flags = {};
            queueCreateInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
            queueCreateInfo.queueCount = 2;
            queueCreateInfo.pQueuePriorities = queuePriorities;

            queueCreateInfos.push_back(queueCreateInfo);
        }
        else
        {
            m_graphicsQueueIndex = 0;
            m_presentQueueIndex = 0;

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

            queueCreateInfos.push_back(graphicsQueueCreateInfo);
            queueCreateInfos.push_back(presentQueueCreateInfo);
        }

        vk::PhysicalDeviceFeatures requiredDeviceFeatures{};
        requiredDeviceFeatures.samplerAnisotropy = true;
        requiredDeviceFeatures.sampleRateShading = true;
        requiredDeviceFeatures.geometryShader = true;

        vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
        descriptorIndexingFeatures.descriptorBindingPartiallyBound = true;
        descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = true;

        vk::PhysicalDeviceFeatures2 requiredDeviceFeatures2{};
        requiredDeviceFeatures2.pNext = &descriptorIndexingFeatures;
        requiredDeviceFeatures2.features = requiredDeviceFeatures;

        m_device = VulkanUtils::CreateDevice(physicalDevice, requiredDeviceExtensions, requiredDeviceLayers, requiredDeviceFeatures2, queueCreateInfos);
    }

    void VulkanDevice::Destroy()
    {
        m_device.destroy();
    }

    vk::Device VulkanDevice::GetHandle() const
    {
        return m_device;
    }

    vk::PhysicalDevice VulkanDevice::GetPhysicalDevice() const
    {
        return m_physicalDevice;
    }

    vk::Queue VulkanDevice::GetGraphicsQueue() const
    {
        return m_device.getQueue(m_graphicsQueueFamilyIndex, m_graphicsQueueIndex);
    }

    vk::Queue VulkanDevice::GetPresentQueue() const
    {
        return m_device.getQueue(m_presentQueueFamilyIndex, m_presentQueueIndex);
    }

    uint32_t VulkanDevice::GetGraphicsQueueFamilyIndex() const
    {
        return m_graphicsQueueFamilyIndex;
    }

    uint32_t VulkanDevice::GetPresentQueueFamilyIndex() const
    {
        return m_presentQueueFamilyIndex;
    }

    void VulkanDevice::WaitIdle()
    {
        m_device.waitIdle();
    }

    void VulkanDevice::FindRequiredQueueFamilyIndices(vk::SurfaceKHR surface)
    {
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
                m_physicalDevice.getSurfaceSupportKHR(i, surface, &surfaceSupport);
                if (surfaceSupport)
                {
                    presentSupport = true;
                    m_presentQueueFamilyIndex = i;
                }
            }
        }
        FIREFLY_ASSERT(graphicsSupport && presentSupport, "Picked Vulkan device doesn't support all required queue families!");
    }
}