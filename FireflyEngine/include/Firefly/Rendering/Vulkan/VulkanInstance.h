#pragma once

#include <vulkan/vulkan.hpp>

#include <string>
#include "Core/Version.h"

namespace Firefly
{
	class VulkanInstance
	{
	public:
		VulkanInstance(const std::string& appName, const Version& appVersion, 
					   const std::vector<const char*>& requiredInstanceExtensions, 
					   const std::vector<const char*>& requiredInstanceLayers);
		~VulkanInstance();

		vk::Instance GetInstance() const;

	private:
		vk::Instance m_instance;
	};
}