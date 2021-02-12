#include "pch.h"
#include "Rendering/MeshGenerator.h"

#include "Rendering/RenderingAPI.h"

namespace Firefly
{
    std::shared_ptr<Mesh> MeshGenerator::CreateQuad(const glm::vec2& size, uint32_t subdivisions)
    {
        std::vector<Mesh::Vertex> vertices;
        std::vector<uint32_t> indices;

        Mesh::Vertex vertex;
        for (size_t y = 0; y < subdivisions; y++)
        {
            for (size_t x = 0; x < subdivisions; x++)
            {
                indices.push_back(x + y * (subdivisions + 1));
                indices.push_back(x + y * (subdivisions + 1) + 1);
                indices.push_back(x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(x + y * (subdivisions + 1));
                indices.push_back(x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(x + y * (subdivisions + 1) + 1 + subdivisions);
            }
        }

        vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        vertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        vertex.bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
        for (size_t y = 0; y < subdivisions + 1; y++)
        {
            for (size_t x = 0; x < subdivisions + 1; x++)
            {
                vertex.position = glm::vec3(((float)x / (float)subdivisions - 0.5f) * size.x, ((float)y / (float)subdivisions - 0.5f) * size.y, 0.0f);
                vertex.texCoords = glm::vec2((float)x / (float)subdivisions, (float)y / (float)subdivisions);
                vertices.push_back(vertex);
            }
        }

        return RenderingAPI::CreateMesh(vertices, indices);
    }

    std::shared_ptr<Mesh> MeshGenerator::CreateBox(const glm::vec3& size, uint32_t subdivisions)
    {
        std::vector<Mesh::Vertex> vertices;
        std::vector<uint32_t> indices;

        Mesh::Vertex vertex;
        // front
        for (size_t y = 0; y < subdivisions; y++)
        {
            for (size_t x = 0; x < subdivisions; x++)
            {
                indices.push_back(vertices.size() + x + y * (subdivisions + 1));
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 1);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1));
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 1 + subdivisions);
            }
        }

        vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        vertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        vertex.bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
        for (size_t y = 0; y < subdivisions + 1; y++)
        {
            for (size_t x = 0; x < subdivisions + 1; x++)
            {
                vertex.position = glm::vec3((float)x / (float)subdivisions - 0.5f, (float)y / (float)subdivisions - 0.5f, 0.5f) * size;
                vertex.texCoords = glm::vec2((float)x / (float)subdivisions, (float)y / (float)subdivisions);
                vertices.push_back(vertex);
            }
        }

        // back
        for (size_t y = 0; y < subdivisions; y++)
        {
            for (size_t x = 0; x < subdivisions; x++)
            {
                indices.push_back(vertices.size() + x + y * (subdivisions + 1));
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 1);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1));
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 1 + subdivisions);
            }
        }

        vertex.normal = glm::vec3(0.0f, 0.0f, -1.0f);
        vertex.tangent = glm::vec3(-1.0f, 0.0f, 0.0f);
        vertex.bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
        for (size_t y = 0; y < subdivisions + 1; y++)
        {
            for (size_t x = 0; x < subdivisions + 1; x++)
            {
                vertex.position = glm::vec3(0.5f - (float)x / (float)subdivisions, (float)y / (float)subdivisions - 0.5f, -0.5f) * size;
                vertex.texCoords = glm::vec2((float)x / (float)subdivisions, (float)y / (float)subdivisions);
                vertices.push_back(vertex);
            }
        }

        // right
        for (size_t y = 0; y < subdivisions; y++)
        {
            for (size_t x = 0; x < subdivisions; x++)
            {
                indices.push_back(vertices.size() + x + y * (subdivisions + 1));
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 1);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1));
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 1 + subdivisions);
            }
        }

        vertex.normal = glm::vec3(1.0f, 0.0f, 0.0f);
        vertex.tangent = glm::vec3(0.0f, 0.0f, -1.0f);
        vertex.bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
        for (size_t y = 0; y < subdivisions + 1; y++)
        {
            for (size_t x = 0; x < subdivisions + 1; x++)
            {
                vertex.position = glm::vec3(0.5f, (float)y / (float)subdivisions - 0.5f, 0.5f - (float)x / (float)subdivisions) * size;
                vertex.texCoords = glm::vec2((float)x / (float)subdivisions, (float)y / (float)subdivisions);
                vertices.push_back(vertex);
            }
        }

        // left
        for (size_t y = 0; y < subdivisions; y++)
        {
            for (size_t x = 0; x < subdivisions; x++)
            {
                indices.push_back(vertices.size() + x + y * (subdivisions + 1));
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 1);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1));
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 1 + subdivisions);
            }
        }

        vertex.normal = glm::vec3(-1.0f, 0.0f, 0.0f);
        vertex.tangent = glm::vec3(0.0f, 0.0f, 1.0f);
        vertex.bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
        for (size_t y = 0; y < subdivisions + 1; y++)
        {
            for (size_t x = 0; x < subdivisions + 1; x++)
            {
                vertex.position = glm::vec3(-0.5f, (float)y / (float)subdivisions - 0.5f, (float)x / (float)subdivisions - 0.5f) * size;
                vertex.texCoords = glm::vec2((float)x / (float)subdivisions, (float)y / (float)subdivisions);
                vertices.push_back(vertex);
            }
        }

        // top
        for (size_t y = 0; y < subdivisions; y++)
        {
            for (size_t x = 0; x < subdivisions; x++)
            {
                indices.push_back(vertices.size() + x + y * (subdivisions + 1));
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 1);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1));
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 1 + subdivisions);
            }
        }

        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        vertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        vertex.bitangent = glm::vec3(0.0f, 0.0f, -1.0f);
        for (size_t y = 0; y < subdivisions + 1; y++)
        {
            for (size_t x = 0; x < subdivisions + 1; x++)
            {
                vertex.position = glm::vec3((float)x / (float)subdivisions - 0.5f, 0.5f, 0.5f - (float)y / (float)subdivisions) * size;
                vertex.texCoords = glm::vec2((float)x / (float)subdivisions, (float)y / (float)subdivisions);
                vertices.push_back(vertex);
            }
        }

        // bottom
        for (size_t y = 0; y < subdivisions; y++)
        {
            for (size_t x = 0; x < subdivisions; x++)
            {
                indices.push_back(vertices.size() + x + y * (subdivisions + 1));
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 1);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1));
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 2 + subdivisions);
                indices.push_back(vertices.size() + x + y * (subdivisions + 1) + 1 + subdivisions);
            }
        }

        vertex.normal = glm::vec3(0.0f, -1.0f, 0.0f);
        vertex.tangent = glm::vec3(-1.0f, 0.0f, 0.0f);
        vertex.bitangent = glm::vec3(0.0f, 0.0f, -1.0f);
        for (size_t y = 0; y < subdivisions + 1; y++)
        {
            for (size_t x = 0; x < subdivisions + 1; x++)
            {
                vertex.position = glm::vec3(0.5f - (float)x / (float)subdivisions, -0.5f, 0.5f - (float)y / (float)subdivisions) * size;
                vertex.texCoords = glm::vec2((float)x / (float)subdivisions, (float)y / (float)subdivisions);
                vertices.push_back(vertex);
            }
        }

        return RenderingAPI::CreateMesh(vertices, indices);
    }

    std::shared_ptr<Mesh> MeshGenerator::CreateSphere(float size, uint32_t latitude, uint32_t longitude)
    {
        std::vector<Mesh::Vertex> vertices;
        std::vector<uint32_t> indices;

        latitude = std::max((int)latitude, 3);
        longitude = std::max((int)longitude, 3);

        for (size_t t = 0; t < longitude; t++)
        {
            for (size_t u = 0; u < latitude; u++)
            {
                if (t == 0) // top pole
                {
                    indices.push_back(0);
                    indices.push_back(u + 1);
                    indices.push_back(u + 2);
                }
                else if (t < longitude - 1) // mid segments
                {
                    indices.push_back(t * (latitude + 1) + u + 1);
                    indices.push_back(t * (latitude + 1) + u + 2);
                    indices.push_back((t - 1) * (latitude + 1) + u + 2);

                    indices.push_back(t * (latitude + 1) + u + 1);
                    indices.push_back((t - 1) * (latitude + 1) + u + 2);
                    indices.push_back((t - 1) * (latitude + 1) + u + 1);
                }
                else // bottom pole
                {
                    indices.push_back(t * (latitude + 1) + 1);
                    indices.push_back((t - 1) * (latitude + 1) + u + 2);
                    indices.push_back((t - 1) * (latitude + 1) + u + 1);
                }
            }
        }

        Mesh::Vertex vertex;

        // top pole
        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        vertex.position = vertex.normal * size * 0.5f;
        vertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        vertex.bitangent = glm::vec3(0.0f, 0.0f, -1.0f);
        vertex.texCoords = glm::vec2(0.5f, 1.f);
        vertices.push_back(vertex);

        float latitudeIncrement = 360.0f / latitude;
        float longitudeIncrement = 180.0f / longitude;
        for (float t = longitudeIncrement; t < 180.0f; t += longitudeIncrement)
        {
            for (float u = 0.0f; u <= 360.0f; u += latitudeIncrement)
            {
                vertex.normal = glm::vec3(sinf(DegreeToRad(t)) * sinf(DegreeToRad(u)),
                    cosf(DegreeToRad(t)),
                    sinf(DegreeToRad(t)) * cosf(DegreeToRad(u)));
                vertex.position = vertex.normal * size * 0.5f;
                vertex.tangent = glm::vec3(sinf(DegreeToRad(t)) * sinf(DegreeToRad(u + 90.0f)),
                    0.0f,
                    sinf(DegreeToRad(t)) * cosf(DegreeToRad(u + 90.0f)));
                vertex.bitangent = glm::vec3(sinf(DegreeToRad(t - 90.0f)) * sinf(DegreeToRad(u)),
                    cosf(DegreeToRad(t - 90.0f)),
                    sinf(DegreeToRad(t - 90.0f)) * cosf(DegreeToRad(u)));
                vertex.texCoords = glm::vec2(latitudeIncrement / 360.0f, 1.0f - longitudeIncrement / 180.0f);
                vertices.push_back(vertex);
            }
        }

        // bottom pole
        vertex.normal = glm::vec3(0.0f, -1.0f, 0.0f);
        vertex.position = vertex.normal * size * 0.5f;
        vertex.tangent = glm::vec3(-1.0f, 0.0f, 0.0f);
        vertex.bitangent = glm::vec3(0.0f, 0.0f, -1.0f);
        vertex.texCoords = glm::vec2(0.5f, 0.0f);
        vertices.push_back(vertex);

        return RenderingAPI::CreateMesh(vertices, indices);
    }

    float MeshGenerator::DegreeToRad(float angle)
    {
        return angle * M_PI / 180.0f;
    }
}