#include "pch.h"
#include "Rendering/Vulkan/VulkanSurface.h"

#include "Rendering/Vulkan/VulkanUtils.h"
#include <GLFW/glfw3.h>

namespace Firefly
{
	VulkanSurface::VulkanSurface(VulkanInstance* instance, void* window) :
		m_instance(instance->GetInstance())
	{
		m_glfwWindow = (GLFWwindow*)(window);
		FIREFLY_ASSERT(m_glfwWindow, "Vulkan requires a GLFWwindow pointer!");

		m_surface = VulkanUtils::CreateSurface(m_instance, m_glfwWindow);
	}

	VulkanSurface::~VulkanSurface()
	{
		m_instance.destroySurfaceKHR(m_surface);
	}

	vk::SurfaceKHR VulkanSurface::GetSurface() const
	{
		return m_surface;
	}
	size_t VulkanSurface::GetHeight() const
	{
		int width, height;
		glfwGetFramebufferSize(m_glfwWindow, &width, &height);
		return height;
	}
	size_t VulkanSurface::GetWidth() const
	{
		int width, height;
		glfwGetFramebufferSize(m_glfwWindow, &width, &height);
		return width;
	}
}