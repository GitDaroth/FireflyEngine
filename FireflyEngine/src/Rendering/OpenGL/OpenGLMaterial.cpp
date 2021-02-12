#include "pch.h"
#include "Rendering/OpenGL/OpenGLMaterial.h"

#include "Rendering/OpenGL/OpenGLTexture.h"
#include "Rendering/OpenGL/OpenGLShader.h"

namespace Firefly
{
    OpenGLMaterial::OpenGLMaterial() :
        Material()
    {
    }

    void OpenGLMaterial::Destroy()
    {
    }

    void OpenGLMaterial::Bind()
    {
        std::shared_ptr<OpenGLShader> shader = std::dynamic_pointer_cast<OpenGLShader>(m_shader);
        shader->Bind();

        if (IsTextureEnabled(TextureUsage::Albedo))
        {
            shader->SetUniform("albedoTextureSampler", 0);
            std::dynamic_pointer_cast<OpenGLTexture>(GetTexture(TextureUsage::Albedo))->Bind(0);
        }
        if (IsTextureEnabled(TextureUsage::Normal))
        {
            shader->SetUniform("normalTextureSampler", 1);
            std::dynamic_pointer_cast<OpenGLTexture>(GetTexture(TextureUsage::Normal))->Bind(1);
        }
        if (IsTextureEnabled(TextureUsage::Roughness))
        {
            shader->SetUniform("roughnessTextureSampler", 2);
            std::dynamic_pointer_cast<OpenGLTexture>(GetTexture(TextureUsage::Roughness))->Bind(2);
        }
        if (IsTextureEnabled(TextureUsage::Metalness))
        {
            shader->SetUniform("metalnessTextureSampler", 3);
            std::dynamic_pointer_cast<OpenGLTexture>(GetTexture(TextureUsage::Metalness))->Bind(3);
        }
        if (IsTextureEnabled(TextureUsage::Occlusion))
        {
            shader->SetUniform("occlusionTextureSampler", 4);
            std::dynamic_pointer_cast<OpenGLTexture>(GetTexture(TextureUsage::Occlusion))->Bind(4);
        }
        if (IsTextureEnabled(TextureUsage::Height))
        {
            shader->SetUniform("heightTextureSampler", 5);
            std::dynamic_pointer_cast<OpenGLTexture>(GetTexture(TextureUsage::Height))->Bind(5);
        }

        shader->SetUniform("material.albedo", m_albedo);
        shader->SetUniform("material.roughness", m_roughness);
        shader->SetUniform("material.metalness", m_metalness);
        shader->SetUniform("material.heightScale", m_heightScale);
        shader->SetUniform("material.hasAlbedoTexture", (float)IsTextureEnabled(Material::TextureUsage::Albedo));
        shader->SetUniform("material.hasNormalTexture", (float)IsTextureEnabled(Material::TextureUsage::Normal));
        shader->SetUniform("material.hasRoughnessTexture", (float)IsTextureEnabled(Material::TextureUsage::Roughness));
        shader->SetUniform("material.hasMetalnessTexture", (float)IsTextureEnabled(Material::TextureUsage::Metalness));
        shader->SetUniform("material.hasOcclusionTexture", (float)IsTextureEnabled(Material::TextureUsage::Occlusion));
        shader->SetUniform("material.hasHeightTexture", (float)IsTextureEnabled(Material::TextureUsage::Height));
    }

    void OpenGLMaterial::OnInit()
    {
    }

    void OpenGLMaterial::OnSetTexture(std::shared_ptr<Texture> texture, TextureUsage usage)
    {
    }
}