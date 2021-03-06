#pragma once

#include "Rendering/Material.h"

namespace Firefly
{
    class OpenGLMaterial : public Material
    {
    public:
        OpenGLMaterial();

        virtual void Destroy() override;

        void Bind();

    protected:
        virtual void OnInit() override;
        virtual void OnSetTexture(std::shared_ptr<Texture> texture, TextureUsage usage) override;

    private:
    };
}