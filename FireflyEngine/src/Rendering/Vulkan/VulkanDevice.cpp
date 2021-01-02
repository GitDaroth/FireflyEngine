#include "pch.h"
#include "Rendering/Vulkan/VulkanDevice.h"

namespace Firefly
{
	VulkanDevice::VulkanDevice(vk::PhysicalDevice physicalDevice, VulkanSurface* surface, 
							   const std::vector<const char*>& requiredDeviceExtensions, 
							   const std::vector<const char*>& requiredDeviceLayers) :
		m_physicalDevice(physicalDevice)
	{
		FindRequiredQueueFamilyIndices(surface->GetSurface());

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

	VulkanDevice::~VulkanDevice()
	{
		m_device.destroy();
	}

	vk::Device VulkanDevice::GetDevice() const
	{
		return m_device;
	}

	vk::PhysicalDevice VulkanDevice::GetPhysicalDevice() const
	{
		return m_physicalDevice;
	}

	vk::Queue VulkanDevice::GetGraphicsQueue() const
	{
		return m_device.getQueue(m_graphicsQueueFamilyIndex, 0);
	}

	vk::Queue VulkanDevice::GetPresentQueue() const
	{
		return m_device.getQueue(m_presentQueueFamilyIndex, 0);
	}

	uint32_t VulkanDevice::GetGraphicsQueueFamilyIndex() const
	{
		return m_graphicsQueueFamilyIndex;
	}

	uint32_t VulkanDevice::GetPresentQueueFamilyIndex() const
	{
		return m_presentQueueFamilyIndex;
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