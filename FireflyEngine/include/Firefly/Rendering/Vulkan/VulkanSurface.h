#pragma once

#include "Rendering/Vulkan/VulkanInstance.h"

struct GLFWwindow;

namespace Firefly
{
	class VulkanDevice;

	class VulkanSurface
	{
	public:
		VulkanSurface(VulkanInstance* instance, void* window);
		~VulkanSurface();

		vk::SurfaceKHR GetSurface() const;
		size_t GetHeight() const;
		size_t GetWidth() const;

	private:
		vk::SurfaceKHR m_surface;
		vk::Instance m_instance;
		GLFWwindow* m_glfwWindow;
	};
}