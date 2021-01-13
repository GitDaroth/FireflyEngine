#include "pch.h"
#include "Rendering/Material.h"

namespace Firefly
{
	Material::Material(std::shared_ptr<Shader> shader) :
		m_shader(shader),
		m_color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
	{
	}

	Material::~Material()
	{
	}

	void Material::Destroy()
	{
	}

	std::shared_ptr<Shader> Material::GetShader() const
	{
		return m_shader;
	}

	void Material::SetColor(const glm::vec4& color)
	{
		m_color = color;
	}

	glm::vec4 Material::GetColor() const
	{
		return m_color;
	}
}