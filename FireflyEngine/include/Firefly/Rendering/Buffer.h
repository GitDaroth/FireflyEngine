#pragma once

#include <cstdint>
#include "Core/EnumFlags.h"

namespace Firefly
{
    namespace Rendering
    {
        enum class BufferUsageFlagBits : uint8_t
        {
            NONE = 0,
            VERTEX_BUFFER = 1 << 0,
            INDEX_BUFFER = 1 << 1,
            UNIFORM_BUFFER = 1 << 2,
            STORAGE_BUFFER = 1 << 3,
            INDIRECT_BUFFER = 1 << 4
        };
        using BufferUsageFlags = EnumFlags<BufferUsageFlagBits>;

        enum class BufferCpuAccessFlagBits : uint8_t
        {
            NONE = 0,
            WRITE = 1 << 0,
            READ = 1 << 1
        };
        using BufferCpuAccessFlags = EnumFlags<BufferCpuAccessFlagBits>;

        struct BufferDescription
        {
            uint32_t size = 0;
            uint32_t stride = 0;
            void* data = nullptr;
            BufferUsageFlags usageFlags = BufferUsageFlagBits::NONE;
            BufferCpuAccessFlags cpuAccessFlags = BufferCpuAccessFlagBits::NONE;
        };

        class Buffer
        {
        public:
            Buffer(const BufferDescription& description);
            virtual void Destroy() = 0;

            uint32_t GetSize() const;
            uint32_t GetStride() const;
            BufferUsageFlags GetUsageFlags() const;
            BufferCpuAccessFlags GetCpuAccessFlags() const;

        protected:
            virtual void Init(void* data) = 0;

            uint32_t m_size;
            uint32_t m_stride;
            BufferUsageFlags m_usageFlags;
            BufferCpuAccessFlags m_cpuAccessFlags;
        };
    }
}