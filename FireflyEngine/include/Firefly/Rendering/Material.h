#pragma once

#include "Rendering/Shader.h"
#include "Rendering/Texture.h"
#include <glm/glm.hpp>

namespace Firefly
{
	class Material
	{
	public:
		enum class TextureUsage
		{
			Albedo,
			Normal,
			Roughness,
			Metalness,
			Occlusion,
			Height
		};

		Material(std::shared_ptr<GraphicsContext> context);

		void Init(std::shared_ptr<Shader> shader);
		virtual void Destroy() = 0;

		std::shared_ptr<Shader> GetShader() const;

		void SetAlbedo(const glm::vec4& albedo);
		glm::vec4 GetAlbedo() const;

		void SetRoughness(float roughness);
		float GetRoughness() const;

		void SetMetalness(float metalness);
		float GetMetalness() const;

		void SetHeightScale(float heightScale);
		float GetHeightScale() const;

		void SetTexture(std::shared_ptr<Texture> texture, TextureUsage usage);
		std::shared_ptr<Texture> GetTexture(TextureUsage usage);
		bool HasTexture(TextureUsage usage);
		void ClearTextures();

	protected:
		virtual void OnInit() = 0;
		virtual void OnSetTexture(std::shared_ptr<Texture> texture, TextureUsage usage) = 0;

		std::shared_ptr<GraphicsContext> m_context;

		std::shared_ptr<Shader> m_shader;
		glm::vec4 m_albedo;
		float m_roughness;
		float m_metalness;
		float m_heightScale;
		std::unordered_map<TextureUsage, std::shared_ptr<Texture>> m_textures;
	};
}