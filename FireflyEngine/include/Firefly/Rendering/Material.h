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
		void SetAlbedoMap(std::shared_ptr<Texture2D> texture);
		void SetNormalMap(std::shared_ptr<Texture2D> texture);
		void SetRoughness(float factor);
		void SetRoughnessMap(std::shared_ptr<Texture2D> texture);
		void SetMetalness(float factor);
		void SetMetalnessMap(std::shared_ptr<Texture2D> texture);
		void SetOcclusionMap(std::shared_ptr<Texture2D> texture);
		void SetHeightMap(std::shared_ptr<Texture2D> texture);
		void SetHeightScale(float scale);

		void EnableAlbedoMap(bool enabled);
		void EnableNormalMap(bool enabled);
		void EnableRoughnessMap(bool enabled);
		void EnableMetalnessMap(bool enabled);
		void EnableOcclusionMap(bool enabled);
		void EnableHeightMap(bool enabled);

		bool IsAlbedoMapEnabled() const;
		bool IsNormalMapEnabled() const;
		bool IsRoughnessMapEnabled() const;
		bool IsMetalnessMapEnabled() const;
		bool IsOcclusionMapEnabled() const;
		bool IsHeightMapEnabled() const;

		std::shared_ptr<Shader> GetShader();
	private:
		std::shared_ptr<Shader> m_shader;

		glm::vec4 m_albedoColor;
		float m_roughness;
		float m_metalness;

		bool m_useAlbedoMap;
		bool m_useNormalMap;
		bool m_useRoughnessMap;
		bool m_useMetalnessMap;
		bool m_useOcclusionMap;
		bool m_useHeightMap;
		std::shared_ptr<Texture2D> m_albedoMap;
		std::shared_ptr<Texture2D> m_normalMap;
		std::shared_ptr<Texture2D> m_roughnessMap;
		std::shared_ptr<Texture2D> m_metalnessMap;
		std::shared_ptr<Texture2D> m_occlusionMap;
		std::shared_ptr<Texture2D> m_heightMap;
		float m_heightScale;
	};
}