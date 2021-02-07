#pragma once

#include <vulkan/vulkan.hpp>
#include "Rendering/GraphicsContext.h"
#include "Rendering/Vulkan/VulkanDevice.h"

namespace Firefly
{
	class VulkanSwapchain;

	class VulkanContext : public GraphicsContext
	{
	public:
		virtual void Destroy() override;

		bool BeginScreenFrame();
		bool EndScreenFrame();

		void BeginOffscreenFrame();
		void EndOffscreenFrame();

		vk::CommandBuffer GetCurrentCommandBuffer();
		uint32_t GetCurrentImageIndex() const;

		std::shared_ptr<VulkanDevice> GetDevice() const;
		std::shared_ptr<VulkanSwapchain> GetSwapchain() const;
		vk::SurfaceKHR GetSurface() const;
		vk::CommandPool GetCommandPool() const;
		vk::DescriptorPool GetDescriptorPool() const;

	protected:
		virtual void OnInit(std::shared_ptr<Window> window) override;

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

		void CreateSwapchain();
		void DestroySwapchain();

		void CreateCommandPool();
		void DestroyCommandPool();

		void AllocateCommandBuffers();
		void FreeCommandBuffers();

		void CreateDescriptorPool();
		void DestroyDescriptorPool();

		void CreateSynchronizationPrimitives();
		void DestroySynchronizationPrimitives();

		void PrintGpuInfo();

		std::vector<const char*> GetRequiredInstanceExtensions() const;
		std::vector<const char*> GetRequiredInstanceLayers() const;
		std::vector<const char*> GetRequiredDeviceExtensions() const;
		std::vector<const char*> GetRequiredDeviceLayers() const;
		constexpr bool AreValidationLayersEnabled() const;

		vk::Instance m_instance;
		vk::SurfaceKHR m_surface;
		vk::DebugUtilsMessengerEXT m_debugMessenger;
		std::shared_ptr<VulkanDevice> m_device;
		std::shared_ptr<VulkanSwapchain> m_swapchain;
		vk::DescriptorPool m_descriptorPool;

		vk::CommandPool m_commandPool;
		std::vector<vk::CommandBuffer> m_screenCommandBuffers;
		vk::CommandBuffer m_offscreenCommandBuffer;
		vk::CommandBuffer m_currentCommandBuffer;

		uint32_t m_currentFrameIndex = 0;
		uint32_t m_currentImageIndex = 0;
		std::vector<vk::Semaphore> m_isNewImageAvailableSemaphores;
		std::vector<vk::Semaphore> m_isRenderedImageAvailableSemaphores;
		std::vector<vk::Fence> m_isScreenCommandBufferAvailableFences;
		vk::Fence m_isOffscreenCommandBufferAvailableFence;

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
	};
}