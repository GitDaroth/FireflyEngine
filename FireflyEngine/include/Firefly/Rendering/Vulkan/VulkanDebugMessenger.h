#pragma once

#include "Rendering/Vulkan/VulkanInstance.h"

namespace Firefly
{
	class VulkanDebugMessenger
	{
	public:
		VulkanDebugMessenger(VulkanInstance* instance);
		~VulkanDebugMessenger();

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		vk::DebugUtilsMessengerEXT m_debugMessenger;
		vk::Instance m_instance;
	};
}