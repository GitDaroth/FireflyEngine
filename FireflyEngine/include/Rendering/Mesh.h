#pragma once

#include "Rendering/VertexArray.h"
#include <glm/glm.hpp>

namespace Firefly
{
	class Mesh
	{
	public:
		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 texCoords;
		};

		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		~Mesh();

		void Bind();

		uint32_t GetIndexCount() const;

	private:
		std::shared_ptr<VertexArray> m_vertexArray;
	};
}