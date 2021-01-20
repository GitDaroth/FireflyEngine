#pragma once

#include "Rendering/Mesh.h"
#include <vulkan/vulkan.hpp>

namespace Firefly
{
	class VulkanMesh : public Mesh
	{
	public:
		VulkanMesh();

		virtual void Destroy() override;

		vk::Buffer GetVertexBuffer() const;
		vk::Buffer GetIndexBuffer() const;

	protected:
		virtual void OnInit(std::vector<Vertex> vertices, std::vector<uint32_t> indices) override;

	private:
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