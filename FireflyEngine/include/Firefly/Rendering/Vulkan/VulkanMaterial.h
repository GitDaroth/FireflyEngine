#pragma once

#include "Rendering/Vulkan/VulkanShader.h"
#include <glm/glm.hpp>

namespace Firefly
{
	class VulkanMaterial
	{
	public:
		VulkanMaterial(std::shared_ptr<VulkanShader> shader);
		~VulkanMaterial();

		std::shared_ptr<VulkanShader> GetShader() const;

		void SetColor(const glm::vec4& color);
		glm::vec4 GetColor() const;

	private:
		std::shared_ptr<VulkanShader> m_shader;
		glm::vec4 m_color;
	};
}