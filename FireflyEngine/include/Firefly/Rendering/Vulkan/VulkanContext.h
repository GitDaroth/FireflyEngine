#pragma once

#include <vulkan/vulkan.hpp>
#include "Rendering/GraphicsContext.h"
#include "Rendering/Vulkan/VulkanDevice.h"

namespace Firefly
{
	class VulkanContext : public GraphicsContext
	{
	public:
		virtual void Init(std::shared_ptr<Window> window) override;
		virtual void Destroy() override;

		std::shared_ptr<VulkanDevice> GetDevice() const;
		vk::SurfaceKHR GetSurface() const;
		size_t GetWidth() const;
		size_t GetHeight() const;

	private:
		void CreateInstance();
		void DestroyInstance();

		void CreateSurface();
		void DestroySurface();

		void CreateDebugMessenger();
		void DestroyDebugMessenger();

		vk::PhysicalDevice PickPhysicalDevice();
		void CreateDevice();
		void DestroyDevice();

		void PrintGpuInfo();

		std::vector<const char*> GetRequiredInstanceExtensions() const;
		std::vector<const char*> GetRequiredInstanceLayers() const;
		std::vector<const char*> GetRequiredDeviceExtensions() const;
		std::vector<const char*> GetRequiredDeviceLayers() const;
		constexpr bool AreValidationLayersEnabled() const;

		vk::Instance m_instance;
		vk::SurfaceKHR m_surface;
		std::shared_ptr<Window> m_window;
		vk::DebugUtilsMessengerEXT m_debugMessenger;
		std::shared_ptr<VulkanDevice> m_device;

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
	};
}