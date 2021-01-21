#pragma once

#include "Rendering/Renderer.h"
#include "Rendering/OpenGL/OpenGLContext.h"

#include "Rendering/OpenGL/OpenGLShader.h"

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
		std::shared_ptr<OpenGLContext> m_openGLContext;
		std::vector<Entity> m_entities;
		uint32_t m_windowWidth;
		uint32_t m_windowHeight;

		unsigned int m_irradianceMap;
		unsigned int m_prefilterMap;
		unsigned int m_brdfLUTTexture;
		unsigned int m_envCubemap;
		unsigned int m_cubeVAO;
		unsigned int m_quadVAO;
		std::shared_ptr<OpenGLShader> m_environmentCubeMapShader;
	};
}