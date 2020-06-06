#include "pch.h"
#include "Rendering/Material.h"

namespace Firefly
{
	Material::Material(std::shared_ptr<Shader> shader) :
		m_shader(shader),
		m_albedoColor({0.f, 0.f, 0.f, 1.f}),
		m_hasAlbedoTexture(false),
		m_albedoTexture(nullptr)
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
			m_shader->SetUniformInt("albedoTexture", 0);
			m_shader->SetUniformInt("hasAlbedoTexture", 1);
			m_albedoTexture->Bind(0);
		}
		else
		{
			m_shader->SetUniformInt("hasAlbedoTexture", 0);
			m_shader->SetUniformFloat4("albedoColor", m_albedoColor);
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

	std::shared_ptr<Shader> Material::GetShader()
	{
		return m_shader;
	}
}