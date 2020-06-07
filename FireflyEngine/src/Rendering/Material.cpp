#include "pch.h"
#include "Rendering/Material.h"

namespace Firefly
{
	Material::Material(std::shared_ptr<Shader> shader) :
		m_shader(shader),
		m_albedoColor({0.f, 0.f, 0.f, 1.f}),
		m_roughness(0.f),
		m_metalness(0.f),
		m_hasAlbedoTexture(false),
		m_hasNormalTexture(false),
		m_hasRoughnessTexture(false),
		m_hasMetalnessTexture(false),
		m_hasOcclusionTexture(false),
		m_hasHeightTexture(false),
		m_albedoTexture(nullptr),
		m_normalTexture(nullptr),
		m_roughnessTexture(nullptr),
		m_metalnessTexture(nullptr),
		m_occlusionTexture(nullptr),
		m_heightTexture(nullptr)
	{
	}

	Material::~Material()
	{
	}

	void Material::Bind()
	{
		m_shader->Bind();
		if (m_hasAlbedoTexture)
		{
			m_shader->SetUniformInt("u_albedoTexture", 0);
			m_shader->SetUniformInt("u_hasAlbedoTexture", 1);
			m_albedoTexture->Bind(0);
		}
		else
		{
			m_shader->SetUniformInt("u_hasAlbedoTexture", 0);
			m_shader->SetUniformFloat4("u_albedoColor", m_albedoColor);
		}

		if (m_hasNormalTexture)
		{
			m_shader->SetUniformInt("u_normalTexture", 1);
			m_shader->SetUniformInt("u_hasNormalTexture", 1);
			m_normalTexture->Bind(1);
		}
		else
		{
			m_shader->SetUniformInt("u_hasNormalTexture", 0);
		}

		if (m_hasRoughnessTexture)
		{
			m_shader->SetUniformInt("u_roughnessTexture", 2);
			m_shader->SetUniformInt("u_hasRoughnessTexture", 1);
			m_albedoTexture->Bind(2);
		}
		else
		{
			m_shader->SetUniformInt("u_hasRoughnessTexture", 0);
			m_shader->SetUniformFloat("u_roughness", m_roughness);
		}

		if (m_hasMetalnessTexture)
		{
			m_shader->SetUniformInt("u_metalnessTexture", 3);
			m_shader->SetUniformInt("u_hasMetalnessTexture", 1);
			m_albedoTexture->Bind(3);
		}
		else
		{
			m_shader->SetUniformInt("u_hasMetalnessTexture", 0);
			m_shader->SetUniformFloat("u_metalness", m_metalness);
		}

		if (m_hasOcclusionTexture)
		{
			m_shader->SetUniformInt("u_occlusionTexture", 4);
			m_shader->SetUniformInt("u_hasOcclusionTexture", 1);
			m_normalTexture->Bind(4);
		}
		else
		{
			m_shader->SetUniformInt("u_hasOcclusionTexture", 0);
		}

		if (m_hasHeightTexture)
		{
			m_shader->SetUniformInt("u_heightTexture", 5);
			m_shader->SetUniformInt("u_hasHeightTexture", 1);
			m_normalTexture->Bind(5);
		}
		else
		{
			m_shader->SetUniformInt("u_hasHeightTexture", 0);
		}
	}

	void Material::SetAlbedo(const glm::vec4& color)
	{
		m_albedoColor = color;
	}

	void Material::SetAlbedo(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_albedoTexture = texture;
			m_hasAlbedoTexture = true;
		}
	}

	void Material::SetNormal(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_normalTexture = texture;
			m_hasNormalTexture = true;
		}
	}

	void Material::SetRoughness(float factor)
	{
		m_roughness = factor;
	}

	void Material::SetRoughness(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_roughnessTexture = texture;
			m_hasRoughnessTexture = true;
		}
	}

	void Material::SetMetalness(float factor)
	{
		m_metalness = factor;
	}

	void Material::SetMetalness(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_metalnessTexture = texture;
			m_hasMetalnessTexture = true;
		}
	}

	void Material::SetOcclusion(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_occlusionTexture = texture;
			m_hasOcclusionTexture = true;
		}
	}

	void Material::SetHeight(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_heightTexture = texture;
			m_hasHeightTexture = true;
		}
	}

	std::shared_ptr<Shader> Material::GetShader()
	{
		return m_shader;
	}
}