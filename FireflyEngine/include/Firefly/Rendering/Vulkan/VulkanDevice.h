#pragma once

#include "Rendering/Vulkan/VulkanSurface.h"

namespace Firefly
{
	class VulkanDevice
	{
	public:
		VulkanDevice(vk::PhysicalDevice physicalDevice, VulkanSurface* surface,
					 const std::vector<const char*>& requiredDeviceExtensions,
					 const std::vector<const char*>& requiredDeviceLayers);
		~VulkanDevice();

		vk::Device GetDevice() const;
		vk::PhysicalDevice GetPhysicalDevice() const;

		vk::Queue GetGraphicsQueue() const;
		vk::Queue GetPresentQueue() const;
		uint32_t GetGraphicsQueueFamilyIndex() const;
		uint32_t GetPresentQueueFamilyIndex() const;

	private:
		void FindRequiredQueueFamilyIndices(vk::SurfaceKHR surface);

		vk::Device m_device;
		vk::PhysicalDevice m_physicalDevice;
		uint32_t m_graphicsQueueFamilyIndex;
		uint32_t m_presentQueueFamilyIndex;
	};
}