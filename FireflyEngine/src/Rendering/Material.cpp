#include "pch.h"
#include "Rendering/Material.h"

namespace Firefly
{
	Material::Material(std::shared_ptr<Shader> shader) :
		m_shader(shader),
		m_diffuseColor({0.f, 0.f, 0.f, 1.f}),
		m_hasDiffuseTexture(false),
		m_diffuseTexture(nullptr)
	{
		m_shader->Bind();
		m_shader->SetUniformFloat4("diffuseColor", m_diffuseColor);
		m_shader->SetUniformInt("hasDiffuseTexture", 0);
	}

	Material::~Material()
	{
	}

	void Material::Bind()
	{
		if(m_hasDiffuseTexture)
			m_diffuseTexture->Bind(0);
	}

	void Material::SetDiffuseColor(const glm::vec4& color)
	{
		m_diffuseColor = color;

		m_shader->Bind();
		m_shader->SetUniformFloat4("diffuseColor", m_diffuseColor);
	}

	void Material::SetDiffuseTexture(std::shared_ptr<Texture2D> texture)
	{
		if (texture)
		{
			m_diffuseTexture = texture;
			m_hasDiffuseTexture = true;

			m_shader->Bind();
			m_shader->SetUniformInt("diffuseTexture", 0);
			m_shader->SetUniformInt("hasDiffuseTexture", 1);
		}
	}

	std::shared_ptr<Shader> Material::GetShader()
	{
		return m_shader;
	}
}