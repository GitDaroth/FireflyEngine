#include "pch.h"
#include "Rendering/Vulkan/VulkanMesh.h"

#include "Rendering/Vulkan/VulkanContext.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Firefly
{
	VulkanMesh::VulkanMesh(VulkanDevice* device, const std::string& path, bool flipTexCoords) :
		m_device(device->GetDevice())
	{
		Load(path, flipTexCoords);
	}

	VulkanMesh::VulkanMesh(VulkanDevice* device, std::vector<Vertex> vertices, std::vector<uint32_t> indices) :
		m_device(device->GetDevice())
	{
		Init(vertices, indices);
	}

	VulkanMesh::~VulkanMesh()
	{
		m_device.destroyBuffer(m_vertexBuffer);
		m_device.freeMemory(m_vertexBufferMemory);
		m_device.destroyBuffer(m_indexBuffer);
		m_device.freeMemory(m_indexBufferMemory);
	}

	void VulkanMesh::Bind(vk::CommandBuffer commandBuffer)
	{
		std::vector<vk::Buffer> vertexBuffers = { m_vertexBuffer };
		vk::DeviceSize offsets[] = { 0 };
		commandBuffer.bindVertexBuffers(0, vertexBuffers.size(), vertexBuffers.data(), offsets);
		commandBuffer.bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint32);
	}

	uint32_t VulkanMesh::GetVertexCount() const
	{
		return m_vertexCount;
	}

	uint32_t VulkanMesh::GetIndexCount() const
	{
		return m_indexCount;
	}

	void VulkanMesh::Init(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
	{
		m_vertexCount = vertices.size();
		m_indexCount = indices.size();

		// Vertex Buffer
		vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();
		vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
		vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

		vk::Buffer stagingVertexBuffer;
		vk::DeviceMemory stagingVertexBufferMemory;
		VulkanContext::GetSingleton()->CreateBuffer(bufferSize, bufferUsageFlags, memoryPropertyFlags, stagingVertexBuffer, stagingVertexBufferMemory);

		void* mappedMemory;
		m_device.mapMemory(stagingVertexBufferMemory, 0, bufferSize, {}, &mappedMemory);
		memcpy(mappedMemory, vertices.data(), bufferSize);
		m_device.unmapMemory(stagingVertexBufferMemory);

		bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
		memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
		VulkanContext::GetSingleton()->CreateBuffer(bufferSize, bufferUsageFlags, memoryPropertyFlags, m_vertexBuffer, m_vertexBufferMemory);

		VulkanContext::GetSingleton()->CopyBuffer(stagingVertexBuffer, m_vertexBuffer, bufferSize);

		m_device.destroyBuffer(stagingVertexBuffer);
		m_device.freeMemory(stagingVertexBufferMemory);

		// Index Buffer
		bufferSize = sizeof(uint32_t) * indices.size();
		bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
		memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

		vk::Buffer stagingIndexBuffer;
		vk::DeviceMemory stagingIndexBufferMemory;
		VulkanContext::GetSingleton()->CreateBuffer(bufferSize, bufferUsageFlags, memoryPropertyFlags, stagingIndexBuffer, stagingIndexBufferMemory);

		m_device.mapMemory(stagingIndexBufferMemory, 0, bufferSize, {}, &mappedMemory);
		memcpy(mappedMemory, indices.data(), bufferSize);
		m_device.unmapMemory(stagingIndexBufferMemory);

		bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
		memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
		VulkanContext::GetSingleton()->CreateBuffer(bufferSize, bufferUsageFlags, memoryPropertyFlags, m_indexBuffer, m_indexBufferMemory);

		VulkanContext::GetSingleton()->CopyBuffer(stagingIndexBuffer, m_indexBuffer, bufferSize);

		m_device.destroyBuffer(stagingIndexBuffer);
		m_device.freeMemory(stagingIndexBufferMemory);
	}

	void VulkanMesh::Load(const std::string& path, bool flipTexCoords)
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
}