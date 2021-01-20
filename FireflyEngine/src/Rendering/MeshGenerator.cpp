#include "pch.h"
#include "Rendering/MeshGenerator.h"

#include "Rendering/RenderingAPI.h"

namespace Firefly
{
	std::shared_ptr<Mesh> MeshGenerator::CreateBox(std::shared_ptr<GraphicsContext> context, const glm::vec3& size, uint32_t subdivisions)
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

		std::shared_ptr<Mesh> boxMesh =  RenderingAPI::CreateMesh(context);
		boxMesh->Init(vertices, indices);

		return boxMesh;
	}

	std::shared_ptr<Mesh> MeshGenerator::CreateSphere(std::shared_ptr<GraphicsContext> context, float size, uint32_t latitude, uint32_t longitude)
	{
		std::vector<Mesh::Vertex> vertices;
		std::vector<uint32_t> indices;


		
		std::shared_ptr<Mesh> sphereMesh = RenderingAPI::CreateMesh(context);
		sphereMesh->Init(vertices, indices);

		return sphereMesh;
	}

	float MeshGenerator::DegreeToRad(float angle)
	{
		return angle * M_PI / 180.0f;
	}
}