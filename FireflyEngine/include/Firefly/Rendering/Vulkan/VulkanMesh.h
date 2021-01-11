#pragma once

#include "Rendering/Vulkan/VulkanDevice.h"
#include <glm/glm.hpp>

namespace Firefly
{
	class VulkanMesh
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

		VulkanMesh(VulkanDevice* device, vk::CommandPool commandPool, vk::Queue queue, const std::string& path, bool flipTexCoords = false);
		VulkanMesh(VulkanDevice* device, vk::CommandPool commandPool, vk::Queue queue, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		~VulkanMesh();

		void Bind(vk::CommandBuffer commandBuffer);

		uint32_t GetVertexCount() const;
		uint32_t GetIndexCount() const;

	private:
		void Init(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		void Load(const std::string& path, bool flipTexCoords);

		uint32_t m_vertexCount = 0;
		uint32_t m_indexCount = 0;

		vk::Buffer m_vertexBuffer;
		vk::DeviceMemory m_vertexBufferMemory;
		vk::Buffer m_indexBuffer;
		vk::DeviceMemory m_indexBufferMemory;

		vk::Device m_device;
		vk::PhysicalDevice m_physicalDevice;
		vk::CommandPool m_commandPool;
		vk::Queue m_queue;
	};
}