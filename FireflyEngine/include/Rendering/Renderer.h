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
		uint32_t m_vertexArray;
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<VertexBuffer> m_vertexBuffer;
		std::shared_ptr<IndexBuffer> m_indexBuffer;
	};
}