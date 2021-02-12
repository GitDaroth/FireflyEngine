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
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec3 tangent;
            glm::vec3 bitangent;
            glm::vec2 texCoords;
        };

        void Init(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
        void Init(const std::string& path, bool flipTexCoords = false);
        virtual void Destroy() = 0;

        uint32_t GetVertexCount() const;
        uint32_t GetIndexCount() const;

    protected:
        virtual void OnInit(std::vector<Vertex> vertices, std::vector<uint32_t> indices) = 0;

        uint32_t m_vertexCount = 0;
        uint32_t m_indexCount = 0;
    };
}