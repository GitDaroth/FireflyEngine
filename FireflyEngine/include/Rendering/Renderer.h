#pragma once

#include "Rendering/Mesh.h"
#include "Rendering/Shader.h"
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
		void SubmitDraw(std::shared_ptr<Shader> shader, std::shared_ptr<Mesh> mesh, const glm::mat4& modelMatrix = glm::mat4(1.f));

	private:
		glm::mat4 m_viewProjectionMatrix;
	};
}