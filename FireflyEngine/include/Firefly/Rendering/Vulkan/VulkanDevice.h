#pragma once

#include <vulkan/vulkan.hpp>

namespace Firefly
{
	class VulkanDevice
	{
	public:
		void Init(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface,
			const std::vector<const char*>& requiredDeviceExtensions,
			const std::vector<const char*>& requiredDeviceLayers);
		void Destroy();

		vk::Device GetDevice() const;
		vk::PhysicalDevice GetPhysicalDevice() const;

		vk::Queue GetGraphicsQueue() const;
		vk::Queue GetPresentQueue() const;
		uint32_t GetGraphicsQueueFamilyIndex() const;
		uint32_t GetPresentQueueFamilyIndex() const;

		void WaitIdle();

	private:
		void FindRequiredQueueFamilyIndices(vk::SurfaceKHR surface);

		vk::Device m_device;
		vk::PhysicalDevice m_physicalDevice;

		uint32_t m_graphicsQueueFamilyIndex = 0;
		uint32_t m_graphicsQueueIndex = 0;
		uint32_t m_presentQueueFamilyIndex = 0;
		uint32_t m_presentQueueIndex = 0;
	};
}