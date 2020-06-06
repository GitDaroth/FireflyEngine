#include "pch.h"
#include "Rendering/Mesh.h"

#include "Rendering/RenderingAPI.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Firefly
{
	Mesh::Mesh(const std::string& path)
	{
		Load(path);
	}

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
	{
		Init(vertices, indices);
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::Init(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
	{
		m_vertexArray = RenderingAPI::CreateVertexArray();
		m_vertexArray->Init();
		m_vertexArray->Bind();

		std::shared_ptr<VertexBuffer> vertexBuffer = RenderingAPI::CreateVertexBuffer();
		vertexBuffer->Init((float*)vertices.data(), vertices.size() * sizeof(Vertex));

		VertexBuffer::Layout layout = {
			{ ShaderDataType::Float3, "position" },
			{ ShaderDataType::Float3, "normal" },
			{ ShaderDataType::Float2, "texCoords" }
		};
		vertexBuffer->SetLayout(layout);

		std::shared_ptr<IndexBuffer> indexBuffer = RenderingAPI::CreateIndexBuffer();
		indexBuffer->Init(indices.data(), indices.size() * sizeof(uint32_t));

		m_vertexArray->AddVertexBuffer(vertexBuffer);
		m_vertexArray->SetIndexBuffer(indexBuffer);
	}

	void Mesh::Load(const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			Logger::Error("FireflyEngine", "assimp error: {0}", importer.GetErrorString());
		}
		else
		{
			if (scene->mNumMeshes == 1)
			{
				aiMesh* mesh = scene->mMeshes[0];

				std::vector<Vertex> vertices;
				for (int i = 0; i < mesh->mNumVertices; i++)
				{
					Vertex vertex;
					vertex.position.x = mesh->mVertices[i].x;
					vertex.position.y = mesh->mVertices[i].y;
					vertex.position.z = mesh->mVertices[i].z;
					vertex.normal.x = mesh->mNormals[i].x;
					vertex.normal.y = mesh->mNormals[i].y;
					vertex.normal.z = mesh->mNormals[i].z;
					if (mesh->mTextureCoords[0])
					{
						vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
						vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
					}
					else
					{
						vertex.texCoords.x = 0.f;
						vertex.texCoords.y = 0.f;
					}
					vertices.push_back(vertex);
				}

				std::vector<uint32_t> indices;
				for (int i = 0; i < mesh->mNumFaces; i++)
				{
					aiFace face = mesh->mFaces[i];
					for (int j = 0; j < face.mNumIndices; j++)
						indices.push_back(face.mIndices[j]);
				}

				Init(vertices, indices);
			}
			else
			{
				Logger::Error("FireflyEngine", "assimp error: Only single mesh files are supported!");
			}
		}
	}

	void Mesh::Bind()
	{
		m_vertexArray->Bind();
	}

	uint32_t Mesh::GetIndexCount() const
	{
		return m_vertexArray->GetIndexBuffer()->GetCount();
	}
}