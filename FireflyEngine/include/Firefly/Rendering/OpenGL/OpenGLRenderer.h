#pragma once

#include "Rendering/Renderer.h"
#include "Rendering/OpenGL/OpenGLContext.h"

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
	};
}