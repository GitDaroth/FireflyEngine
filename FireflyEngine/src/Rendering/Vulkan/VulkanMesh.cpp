#include "pch.h"
#include "Rendering/Vulkan/VulkanMesh.h"

#include "Rendering/RenderingAPI.h"
#include "Rendering/Vulkan/VulkanUtils.h"
#include "Rendering/Vulkan/VulkanContext.h"

namespace Firefly
{
    VulkanMesh::VulkanMesh()
    {
        std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
        m_device = vkContext->GetDevice()->GetHandle();
        m_physicalDevice = vkContext->GetDevice()->GetPhysicalDevice();
        m_commandPool = vkContext->GetCommandPool();
        m_queue = vkContext->GetDevice()->GetGraphicsQueue();
    }

    void VulkanMesh::Destroy()
    {
        m_device.destroyBuffer(m_vertexBuffer);
        m_device.freeMemory(m_vertexBufferMemory);
        m_device.destroyBuffer(m_indexBuffer);
        m_device.freeMemory(m_indexBufferMemory);
    }

    vk::Buffer VulkanMesh::GetVertexBuffer() const
    {
        return m_vertexBuffer;
    }

    vk::Buffer VulkanMesh::GetIndexBuffer() const
    {
        return m_indexBuffer;
    }

    void VulkanMesh::OnInit(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    {
        // Vertex Buffer
        vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();
        vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
        vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

        vk::Buffer stagingVertexBuffer;
        vk::DeviceMemory stagingVertexBufferMemory;
        VulkanUtils::CreateBuffer(m_device, m_physicalDevice, bufferSize, bufferUsageFlags, memoryPropertyFlags, stagingVertexBuffer, stagingVertexBufferMemory);

        void* mappedMemory;
        m_device.mapMemory(stagingVertexBufferMemory, 0, bufferSize, {}, &mappedMemory);
        memcpy(mappedMemory, vertices.data(), bufferSize);
        m_device.unmapMemory(stagingVertexBufferMemory);

        bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
        memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        VulkanUtils::CreateBuffer(m_device, m_physicalDevice, bufferSize, bufferUsageFlags, memoryPropertyFlags, m_vertexBuffer, m_vertexBufferMemory);

        VulkanUtils::CopyBuffer(m_device, m_commandPool, m_queue, stagingVertexBuffer, m_vertexBuffer, bufferSize);

        m_device.destroyBuffer(stagingVertexBuffer);
        m_device.freeMemory(stagingVertexBufferMemory);

        // Index Buffer
        bufferSize = sizeof(uint32_t) * indices.size();
        bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
        memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

        vk::Buffer stagingIndexBuffer;
        vk::DeviceMemory stagingIndexBufferMemory;
        VulkanUtils::CreateBuffer(m_device, m_physicalDevice, bufferSize, bufferUsageFlags, memoryPropertyFlags, stagingIndexBuffer, stagingIndexBufferMemory);

        m_device.mapMemory(stagingIndexBufferMemory, 0, bufferSize, {}, &mappedMemory);
        memcpy(mappedMemory, indices.data(), bufferSize);
        m_device.unmapMemory(stagingIndexBufferMemory);

        bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
        memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        VulkanUtils::CreateBuffer(m_device, m_physicalDevice, bufferSize, bufferUsageFlags, memoryPropertyFlags, m_indexBuffer, m_indexBufferMemory);

        VulkanUtils::CopyBuffer(m_device, m_commandPool, m_queue, stagingIndexBuffer, m_indexBuffer, bufferSize);

        m_device.destroyBuffer(stagingIndexBuffer);
        m_device.freeMemory(stagingIndexBufferMemory);
    }
}