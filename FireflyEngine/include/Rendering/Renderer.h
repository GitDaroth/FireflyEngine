#pragma once

#include "Rendering/VertexArray.h"

namespace Firefly
{
	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void BeginScene();
		void EndScene();
		void SubmitDraw(std::shared_ptr<VertexArray> vertexArray);
	};
}