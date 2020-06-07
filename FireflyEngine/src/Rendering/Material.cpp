#include "pch.h"
#include "Rendering/Material.h"

namespace Firefly
{
	Material::Material(std::shared_ptr<Shader> shader) :
		m_shader(shader),
		m_albedoColor({0.f, 0.f, 0.f, 1.f}),
		m_roughness(0.f),
		m_metalness(0.f),
		m_useAlbedoMap(false),
		m_useNormalMap(false),
		m_useRoughnessMap(false),
		m_useMetalnessMap(false),
		m_useOcclusionMap(false),
		m_useHeightMap(false),
		m_albedoMap(nullptr),
		m_normalMap(nullptr),
		m_roughnessMap(nullptr),
		m_metalnessMap(nullptr),
		m_occlusionMap(nullptr),
		m_heightMap(nullptr)
	{
	}

	Material::~Material()
	{
	}

	void Material::Bind()
	{
		m_shader->Bind();
		if (m_useAlbedoMap)
		{
			m_shader->SetUniformInt("u_albedoMap", 0);
			m_shader->SetUniformInt("u_useAlbedoMap", 1);
			m_albedoMap->Bind(0);
		}
		else
		{
			m_shader->SetUniformInt("u_useAlbedoMap", 0);
			m_shader->SetUniformFloat4("u_albedoColor", m_albedoColor);
		}

		if (m_useNormalMap)
		{
			m_shader->SetUniformInt("u_normalMap", 1);
			m_shader->SetUniformInt("u_useNormalMap", 1);
			m_normalMap->Bind(1);
		}
		else
		{
			m_shader->SetUniformInt("u_useNormalMap", 0);
		}

		if (m_useRoughnessMap)
		{
			m_shader->SetUniformInt("u_roughnessMap", 2);
			m_shader->SetUniformInt("u_useRoughnessMap", 1);
			m_roughnessMap->Bind(2);
		}
		else
		{
			m_shader->SetUniformInt("u_useRoughnessMap", 0);
			m_shader->SetUniformFloat("u_roughness", m_roughness);
		}

		if (m_useMetalnessMap)
		{
			m_shader->SetUniformInt("u_metalnessMap", 3);
			m_shader->SetUniformInt("u_useMetalnessMap", 1);
			m_metalnessMap->Bind(3);
		}
		else
		{
			m_shader->SetUniformInt("u_useMetalnessMap", 0);
			m_shader->SetUniformFloat("u_metalness", m_metalness);
		}

		if (m_useOcclusionMap)
		{
			m_shader->SetUniformInt("u_occlusionMap", 4);
			m_shader->SetUniformInt("u_useOcclusionMap", 1);
			m_occlusionMap->Bind(4);
		}
		else
		{
			m_shader->SetUniformInt("u_useOcclusionMap", 0);
		}

		if (m_useHeightMap)
		{
			m_shader->SetUniformInt("u_heightMap", 5);
			m_shader->SetUniformInt("u_useHeightMap", 1);
			m_heightMap->Bind(5);
		}
		else
		{
			m_shader->SetUniformInt("u_useHeightMap", 0);
		}
	}

	void Material::SetAlbedo(const glm::vec4& color)
	{
		m_albedoColor = color;
	}

	void Material::SetAlbedoMap(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_albedoMap = texture;
			m_useAlbedoMap = true;
		}
	}

	void Material::SetNormalMap(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_normalMap = texture;
			m_useNormalMap = true;
		}
	}

	void Material::SetRoughness(float factor)
	{
		m_roughness = factor;
	}

	void Material::SetRoughnessMap(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_roughnessMap = texture;
			m_useRoughnessMap = true;
		}
	}

	void Material::SetMetalness(float factor)
	{
		m_metalness = factor;
	}

	void Material::SetMetalnessMap(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_metalnessMap = texture;
			m_useMetalnessMap = true;
		}
	}

	void Material::SetOcclusionMap(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_occlusionMap = texture;
			m_useOcclusionMap = true;
		}
	}

	void Material::SetHeightMap(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_heightMap = texture;
			m_useHeightMap = true;
		}
	}

	void Material::EnableAlbedoMap(bool enabled)
	{
		if (m_albedoMap)
			m_useAlbedoMap = enabled;
	}

	void Material::EnableNormalMap(bool enabled)
	{
		if (m_normalMap)
			m_useNormalMap = enabled;
	}

	void Material::EnableRoughnessMap(bool enabled)
	{
		if (m_roughnessMap)
			m_useRoughnessMap = enabled;
	}

	void Material::EnableMetalnessMap(bool enabled)
	{
		if (m_metalnessMap)
			m_useMetalnessMap = enabled;
	}

	void Material::EnableOcclusionMap(bool enabled)
	{
		if (m_occlusionMap)
			m_useOcclusionMap = enabled;
	}

	void Material::EnableHeightMap(bool enabled)
	{
		if (m_heightMap)
			m_useHeightMap = enabled;
	}

	bool Material::IsAlbedoMapEnabled() const
	{
		return m_useAlbedoMap;
	}

	bool Material::IsNormalMapEnabled() const
	{
		return m_useNormalMap;
	}

	bool Material::IsRoughnessMapEnabled() const
	{
		return m_useRoughnessMap;
	}

	bool Material::IsMetalnessMapEnabled() const
	{
		return m_useMetalnessMap;
	}

	bool Material::IsOcclusionMapEnabled() const
	{
		return m_useOcclusionMap;
	}

	bool Material::IsHeightMapEnabled() const
	{
		return m_useHeightMap;
	}

	std::shared_ptr<Shader> Material::GetShader()
	{
		return m_shader;
	}
}