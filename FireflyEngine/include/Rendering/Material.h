#pragma once

#include "Rendering/Shader.h"
#include "Rendering/Texture.h"

namespace Firefly
{
	class Material
	{
	public:
		Material(std::shared_ptr<Shader> shader);
		~Material();

		void Bind();

		void SetDiffuseColor(const glm::vec4& color);
		void SetDiffuseTexture(std::shared_ptr<Texture2D> texture);

		std::shared_ptr<Shader> GetShader();
	private:
		std::shared_ptr<Shader> m_shader;

		glm::vec4 m_diffuseColor;

		bool m_hasDiffuseTexture;
		std::shared_ptr<Texture2D> m_diffuseTexture;
	};
}