#pragma once

#include "Rendering/Renderer.h"
#include "Rendering/OpenGL/OpenGLContext.h"

#include "Rendering/OpenGL/OpenGLShader.h"
#include "Rendering/OpenGL/OpenGLTexture.h"

#include "Rendering/RenderPass.h"

namespace Firefly
{
    class OpenGLRenderer : public Renderer
    {
    public:
        OpenGLRenderer();

        virtual void Init() override;
        virtual void Destroy() override;

        virtual void BeginDrawRecording() override;
        virtual void RecordDraw(const Entity& entity) override;
        virtual void EndDrawRecording() override;
        virtual void SubmitDraw(std::shared_ptr<Camera> camera) override;

    private:
        void CreateRenderPass();
        void DestroyRenderPass();
        void CreateFrameBuffer();
        void DestroyFrameBuffer();

        void CreatePBRShaderResources();
        void DestroyPBRShaderResources();


        std::shared_ptr<OpenGLContext> m_openGLContext;
        std::vector<Entity> m_entities;
        uint32_t m_windowWidth;
        uint32_t m_windowHeight;

        Texture::SampleCount m_msaaSampleCount = Texture::SampleCount::SAMPLE_4;
        std::shared_ptr<RenderPass> m_mainRenderPass;
        std::shared_ptr<FrameBuffer> m_mainFrameBuffer;
        std::shared_ptr<OpenGLTexture> m_depthTexture;
        std::shared_ptr<OpenGLTexture> m_colorTexture;
        std::shared_ptr<OpenGLTexture> m_colorResolveTexture;
        std::shared_ptr<OpenGLShader> m_screenTextureShader;

        std::shared_ptr<OpenGLTexture> m_environmentCubeMap;
        std::shared_ptr<OpenGLTexture> m_irradianceCubeMap;
        std::shared_ptr<OpenGLTexture> m_prefilterCubeMap;
        std::shared_ptr<OpenGLTexture> m_brdfLUT;
        unsigned int m_cubeVAO;
        unsigned int m_quadVAO;
        std::shared_ptr<OpenGLShader> m_environmentCubeMapShader;
    };
}