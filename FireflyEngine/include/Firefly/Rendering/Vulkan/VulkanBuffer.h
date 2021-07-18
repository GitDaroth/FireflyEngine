#pragma once

#include "Rendering/Buffer.h"
#include <vulkan/vulkan.hpp>

namespace Firefly
{
    namespace Rendering
    {
        class VulkanBuffer : Buffer
        {
        public:
            VulkanBuffer(const BufferDescription& description);
            virtual void Destroy() override;

            vk::Buffer GetHandle() const;

        protected:
            virtual void Init(void* data) override;

        private:
            vk::Buffer m_handle;
            vk::DeviceMemory m_memoryHandle;

            vk::Device m_device;
            vk::PhysicalDevice m_physicalDevice;
            vk::CommandPool m_commandPool;
            vk::Queue m_queue;
        };
    }
}