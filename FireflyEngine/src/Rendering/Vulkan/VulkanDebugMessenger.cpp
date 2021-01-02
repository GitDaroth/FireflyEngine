#include "pch.h"
#include "Rendering/Vulkan/VulkanDebugMessenger.h"

namespace Firefly
{
	VulkanDebugMessenger::VulkanDebugMessenger(VulkanInstance* instance) : 
		m_instance(instance->GetInstance())
	{
		vk::DebugUtilsMessengerCreateInfoEXT debugMessngerCreateInfo{};
		debugMessngerCreateInfo.pNext = nullptr;
		debugMessngerCreateInfo.flags = {};
		debugMessngerCreateInfo.pUserData = nullptr;
		debugMessngerCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		debugMessngerCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		debugMessngerCreateInfo.pfnUserCallback = DebugMessengerCallback;

		vk::DispatchLoaderDynamic dispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);
		vk::Result result = m_instance.createDebugUtilsMessengerEXT(&debugMessngerCreateInfo, nullptr, &m_debugMessenger, dispatchLoaderDynamic);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan debug messenger!");
	}

	VulkanDebugMessenger::~VulkanDebugMessenger()
	{
		vk::DispatchLoaderDynamic dispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);
		m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, dispatchLoaderDynamic);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugMessenger::DebugMessengerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::string messageTypeLabel;
		switch (messageType)
		{
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			messageTypeLabel = "general";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			messageTypeLabel = "validation";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			messageTypeLabel = "performance";
			break;
		}

		std::string message = "[" + messageTypeLabel + "] debug message: " + std::string(pCallbackData->pMessage);
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			Logger::Warn("Vulkan", message);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			Logger::Error("Vulkan", message);
			break;
		}
		return VK_FALSE;
	}
}