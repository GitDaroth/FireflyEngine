#pragma once

#include "Rendering/GraphicsContext.h"
#include <glm/glm.hpp>

namespace Firefly
{
	class Mesh
	{
	public:
		struct Vertex
		{
			glm::vec4 position;
			glm::vec4 normal;
			glm::vec4 tangent;
			glm::vec4 bitangent;
			glm::vec2 texCoords;
		};

		Mesh(std::shared_ptr<GraphicsContext> context);

		void Init(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		void Init(const std::string& path, bool flipTexCoords = false);
		virtual void Destroy() = 0;

		uint32_t GetVertexCount() const;
		uint32_t GetIndexCount() const;

	protected:
		virtual void OnInit(std::vector<Vertex> vertices, std::vector<uint32_t> indices) = 0;

		std::shared_ptr<GraphicsContext> m_context;
		uint32_t m_vertexCount = 0;
		uint32_t m_indexCount = 0;
	};
}