#pragma once

#include "Scene/Scene.h"
#include "Rendering/GraphicsContext.h"
#include "Rendering/Shader.h"
#include "Rendering/Camera.h"

namespace Firefly
{
	class Renderer
	{
	public:
		Renderer(std::shared_ptr<GraphicsContext> context);

		virtual void Init() = 0;
		virtual void Destroy() = 0;

		virtual void BeginDrawRecording() = 0;
		virtual void RecordDraw(const Entity& entity) = 0;
		virtual void EndDrawRecording() = 0;
		virtual void SubmitDraw(std::shared_ptr<Camera> camera) = 0;

	protected:
		std::shared_ptr<GraphicsContext> m_context;
	};
}