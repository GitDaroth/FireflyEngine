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

		void SetAlbedo(const glm::vec4& color);
		void SetAlbedo(std::shared_ptr<Texture2D> texture);

		std::shared_ptr<Shader> GetShader();
	private:
		std::shared_ptr<Shader> m_shader;

		glm::vec4 m_albedoColor;

		bool m_hasAlbedoTexture;
		std::shared_ptr<Texture2D> m_albedoTexture;
	};
}