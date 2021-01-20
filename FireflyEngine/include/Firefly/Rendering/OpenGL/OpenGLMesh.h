#pragma once

#include "Rendering/Mesh.h"

namespace Firefly
{
	class OpenGLMesh : public Mesh
	{
	public:
		OpenGLMesh();

		virtual void Destroy() override;

		void Bind();

	protected:
		virtual void OnInit(std::vector<Vertex> vertices, std::vector<uint32_t> indices) override;

	private:
		uint32_t m_vertexArray;
		uint32_t m_vertexBuffer;
		uint32_t m_indexBuffer;
	};
}