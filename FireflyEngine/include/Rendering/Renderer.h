#pragma once

#include "Rendering/Model.h"
#include "Rendering/Camera.h"

namespace Firefly
{
	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void BeginScene(std::shared_ptr<Camera> camera);
		void EndScene();
		void SubmitDraw(std::shared_ptr<Model> model);

	private:
		glm::mat4 m_viewProjectionMatrix;
	};
}