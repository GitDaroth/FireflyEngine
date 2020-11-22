#pragma once

#include "Scene/Scene.h"
#include "Rendering/Camera.h"

namespace Firefly
{
	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void DrawScene(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera);
	};
}