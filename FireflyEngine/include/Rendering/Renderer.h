#pragma once

#include "Rendering/RenderingAPI.h"

namespace Firefly
{
	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void Init();
		void Draw();

	private:
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<VertexArray> m_vertexArray;
	};
}