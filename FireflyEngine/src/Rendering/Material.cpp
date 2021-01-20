#include "pch.h"
#include "Rendering/Material.h"

namespace Firefly
{
	Material::Material() :
		m_shader(nullptr),
		m_albedo(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)),
		m_roughness(0.0f),
		m_metalness(0.0f),
		m_heightScale(0.1f)
	{
	}

	void Material::Init(std::shared_ptr<Shader> shader)
	{
		m_shader = shader;
		OnInit();
	}

	std::shared_ptr<Shader> Material::GetShader() const
	{
		return m_shader;
	}

	void Material::SetAlbedo(const glm::vec4& albedo)
	{
		m_albedo = albedo;
	}

	glm::vec4 Material::GetAlbedo() const
	{
		return m_albedo;
	}

	void Material::SetRoughness(float roughness)
	{
		m_roughness = roughness;
	}

	float Material::GetRoughness() const
	{
		return m_roughness;
	}

	void Material::SetMetalness(float metalness)
	{
		m_metalness = metalness;
	}

	float Material::GetMetalness() const
	{
		return m_metalness;
	}

	void Material::SetHeightScale(float heightScale)
	{
		m_heightScale = heightScale;
	}

	float Material::GetHeightScale() const
	{
		return m_heightScale;
	}

	void Material::SetTexture(std::shared_ptr<Texture> texture, TextureUsage usage)
	{
		if (!texture)
			return;

		m_textures[usage] = texture;
		m_useTextures[usage] = true;
		OnSetTexture(texture, usage);
	}

	std::shared_ptr<Texture> Material::GetTexture(TextureUsage usage)
	{
		if (m_textures.find(usage) != m_textures.end())
			return m_textures[usage];
		else
			return nullptr;
	}

	bool Material::HasTexture(TextureUsage usage)
	{
		return m_textures.find(usage) != m_textures.end();
	}

	void Material::EnableTexture(bool enable, TextureUsage usage)
	{
		if (m_textures.find(usage) != m_textures.end())
			m_useTextures[usage] = enable;
	}

	bool Material::IsTextureEnabled(TextureUsage usage)
	{
		if (m_useTextures.find(usage) != m_useTextures.end())
			return m_useTextures[usage];
		else
			return false;
	}

	void Material::ClearTextures()
	{
		m_textures.clear();
	}
}