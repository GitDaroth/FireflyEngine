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
		void SetNormal(std::shared_ptr<Texture2D> texture);
		void SetRoughness(float factor);
		void SetRoughness(std::shared_ptr<Texture2D> texture);
		void SetMetalness(float factor);
		void SetMetalness(std::shared_ptr<Texture2D> texture);
		void SetOcclusion(std::shared_ptr<Texture2D> texture);
		void SetHeight(std::shared_ptr<Texture2D> texture);

		std::shared_ptr<Shader> GetShader();
	private:
		std::shared_ptr<Shader> m_shader;

		glm::vec4 m_albedoColor;
		float m_roughness;
		float m_metalness;

		bool m_hasAlbedoTexture;
		bool m_hasNormalTexture;
		bool m_hasRoughnessTexture;
		bool m_hasMetalnessTexture;
		bool m_hasOcclusionTexture;
		bool m_hasHeightTexture;
		std::shared_ptr<Texture2D> m_albedoTexture;
		std::shared_ptr<Texture2D> m_normalTexture;
		std::shared_ptr<Texture2D> m_roughnessTexture;
		std::shared_ptr<Texture2D> m_metalnessTexture;
		std::shared_ptr<Texture2D> m_occlusionTexture;
		std::shared_ptr<Texture2D> m_heightTexture;
	};
}