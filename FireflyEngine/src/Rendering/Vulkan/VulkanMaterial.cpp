#include "pch.h"
#include "Rendering/Vulkan/VulkanMaterial.h"

namespace Firefly
{
	VulkanMaterial::VulkanMaterial(std::shared_ptr<VulkanShader> shader) :
		m_shader(shader),
		m_color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
	{
	}

	VulkanMaterial::~VulkanMaterial()
	{
	}

	std::shared_ptr<VulkanShader> VulkanMaterial::GetShader() const
	{
		return m_shader;
	}

	void VulkanMaterial::SetColor(const glm::vec4& color)
	{
		m_color = color;
	}

	glm::vec4 VulkanMaterial::GetColor() const
	{
		return m_color;
	}
}