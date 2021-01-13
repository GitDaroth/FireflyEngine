#pragma once

#include "Rendering/Shader.h"
#include <glm/glm.hpp>

namespace Firefly
{
	class Material
	{
	public:
		Material(std::shared_ptr<Shader> shader);
		~Material();

		void Destroy();

		std::shared_ptr<Shader> GetShader() const;

		void SetColor(const glm::vec4& color);
		glm::vec4 GetColor() const;

	private:
		std::shared_ptr<Shader> m_shader;
		glm::vec4 m_color;
	};
}