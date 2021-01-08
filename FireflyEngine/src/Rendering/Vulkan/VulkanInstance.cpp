#include "pch.h"
#include "Rendering/Vulkan/VulkanInstance.h"

#include "Rendering/Vulkan/VulkanUtils.h"

namespace Firefly
{
	VulkanInstance::VulkanInstance(const std::string& appName, const Version& appVersion,
								   const std::vector<const char*>& requiredInstanceExtensions,
								   const std::vector<const char*>& requiredInstanceLayers)
	{
		uint32_t vkAppVersion = VK_MAKE_VERSION(appVersion.major, appVersion.minor, appVersion.patch);
		uint32_t vkEngineVersion = VK_MAKE_VERSION(1, 0, 0);
		uint32_t vkApiVersion = VK_API_VERSION_1_2;
		m_instance = VulkanUtils::CreateInstance(appName, vkAppVersion,
												 ENGINE_NAME, vkEngineVersion, vkApiVersion,
												 requiredInstanceExtensions, requiredInstanceLayers);

		uint32_t apiVersion;
		vk::enumerateInstanceVersion(&apiVersion);
		Logger::Info("Vulkan", "API Version: {0}.{1}.{2}", VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion));
	}

	VulkanInstance::~VulkanInstance()
	{
		m_instance.destroy();
	}

	vk::Instance VulkanInstance::GetInstance() const
	{
		return m_instance;
	}
}