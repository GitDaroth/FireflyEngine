#include <pch.h>
#include "Rendering/Vulkan/VulkanBuffer.h"

#include "Rendering/RenderingAPI.h"
#include "Rendering/Vulkan/VulkanUtils.h"
#include "Rendering/Vulkan/VulkanContext.h"

namespace Firefly
{
    namespace Rendering
    {
        VulkanBuffer::VulkanBuffer(const BufferDescription& description) :
            Buffer(description)
        {
            std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
            m_device = vkContext->GetDevice()->GetHandle();
            m_physicalDevice = vkContext->GetDevice()->GetPhysicalDevice();
            m_commandPool = vkContext->GetCommandPool();
            m_queue = vkContext->GetDevice()->GetGraphicsQueue();
        }

        void VulkanBuffer::Destroy()
        {
            m_device.destroyBuffer(m_handle);
            m_device.freeMemory(m_memoryHandle);
        }

        vk::Buffer VulkanBuffer::GetHandle() const
        {
            return m_handle;
        }

        void VulkanBuffer::Init(void* data)
        {
            vk::DeviceSize bufferSize = m_size;

            vk::MemoryPropertyFlags memoryPropertyFlags;
            if ((m_cpuAccessFlags & BufferCpuAccessFlagBits::READ) || (m_cpuAccessFlags & BufferCpuAccessFlagBits::WRITE))
                memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
            else
                memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

            vk::BufferUsageFlags bufferUsageFlags;
            if (data != nullptr)
                bufferUsageFlags |= vk::BufferUsageFlagBits::eTransferDst;
            if (m_usageFlags & BufferUsageFlagBits::VERTEX_BUFFER)
                bufferUsageFlags |= vk::BufferUsageFlagBits::eVertexBuffer;
            if (m_usageFlags & BufferUsageFlagBits::INDEX_BUFFER)
                bufferUsageFlags |= vk::BufferUsageFlagBits::eIndexBuffer;
            if (m_usageFlags & BufferUsageFlagBits::UNIFORM_BUFFER)
                bufferUsageFlags |= vk::BufferUsageFlagBits::eUniformBuffer;
            if (m_usageFlags & BufferUsageFlagBits::STORAGE_BUFFER)
                bufferUsageFlags |= vk::BufferUsageFlagBits::eStorageBuffer;
            if (m_usageFlags & BufferUsageFlagBits::INDIRECT_BUFFER)
                bufferUsageFlags |= vk::BufferUsageFlagBits::eIndirectBuffer;

            VulkanUtils::CreateBuffer(m_device, m_physicalDevice, bufferSize, bufferUsageFlags, memoryPropertyFlags, m_handle, m_memoryHandle);

            if (data != nullptr)
            {
                bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
                memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

                vk::Buffer stagingBuffer;
                vk::DeviceMemory stagingBufferMemory;
                VulkanUtils::CreateBuffer(m_device, m_physicalDevice, bufferSize, bufferUsageFlags, memoryPropertyFlags, stagingBuffer, stagingBufferMemory);

                void* mappedMemory;
                m_device.mapMemory(stagingBufferMemory, 0, bufferSize, {}, &mappedMemory);
                memcpy(mappedMemory, data, bufferSize);
                m_device.unmapMemory(stagingBufferMemory);

                VulkanUtils::CopyBuffer(m_device, m_commandPool, m_queue, stagingBuffer, m_handle, bufferSize);

                m_device.destroyBuffer(stagingBuffer);
                m_device.freeMemory(stagingBufferMemory);
            }
        }
    }
}