#include "pch.h"
#include "Rendering/Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Firefly
{
	Mesh::Mesh(std::shared_ptr<GraphicsContext> context) :
		m_context(context)
	{
	}

	void Mesh::Init(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
	{
		m_vertexCount = vertices.size();
		m_indexCount = indices.size();
		OnInit(vertices, indices);
	}

	void Mesh::Init(const std::string& path, bool flipTexCoords)
	{
		Assimp::Importer importer;
		auto importFlags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace;
		if (flipTexCoords)
			importFlags |= aiProcess_FlipUVs;
		const aiScene* scene = importer.ReadFile(path, importFlags);

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
						vertex.tangent.x = mesh->mTangents[i].x;
						vertex.tangent.y = mesh->mTangents[i].y;
						vertex.tangent.z = mesh->mTangents[i].z;
						vertex.bitangent.x = mesh->mBitangents[i].x;
						vertex.bitangent.y = mesh->mBitangents[i].y;
						vertex.bitangent.z = mesh->mBitangents[i].z;
						vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
						vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
					}
					else
					{
						vertex.tangent.x = 1.f;
						vertex.tangent.y = 0.f;
						vertex.tangent.z = 0.f;
						vertex.bitangent.x = 0.f;
						vertex.bitangent.y = 1.f;
						vertex.bitangent.z = 0.f;
						vertex.texCoords.x = 0.f;
						vertex.texCoords.y = 0.f;
					}
					vertices.push_back(vertex);
				}

				std::vector<uint32_t> indices;
				for (int i = 0; i < mesh->mNumFaces; i++)
				{
					aiFace face = mesh->mFaces[i];
					for (int j = 0; j < 3; j++)
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

	uint32_t Mesh::GetVertexCount() const
	{
		return m_vertexCount;
	}

	uint32_t Mesh::GetIndexCount() const
	{
		return m_indexCount;
	}
}