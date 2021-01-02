#include "pch.h"
#include "Rendering/Vulkan/VulkanInstance.h"

namespace Firefly
{
	VulkanInstance::VulkanInstance(const std::string& appName, const Version& appVersion,
								   const std::vector<const char*>& requiredInstanceExtensions,
								   const std::vector<const char*>& requiredInstanceLayers)
	{
		vk::ApplicationInfo applicationInfo{};
		applicationInfo.pNext = nullptr;
		applicationInfo.pApplicationName = appName.c_str();
		applicationInfo.applicationVersion = VK_MAKE_VERSION(appVersion.major, appVersion.minor, appVersion.patch);
		applicationInfo.pEngineName = ENGINE_NAME;
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.apiVersion = VK_API_VERSION_1_2;

		// TODO: Check required instance extensions and layers
		vk::InstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.pNext = nullptr;
		instanceCreateInfo.flags = {};
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.enabledExtensionCount = requiredInstanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();
		instanceCreateInfo.enabledLayerCount = requiredInstanceLayers.size();
		instanceCreateInfo.ppEnabledLayerNames = requiredInstanceLayers.data();

		vk::Result result = vk::createInstance(&instanceCreateInfo, nullptr, &m_instance);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan instance!");
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