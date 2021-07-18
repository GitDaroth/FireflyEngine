#include <pch.h>
#include "Rendering/Buffer.h"

namespace Firefly
{
    namespace Rendering
    {
        Buffer::Buffer(const BufferDescription& description) :
            m_size(description.size),
            m_stride(description.stride),
            m_usageFlags(description.usageFlags),
            m_cpuAccessFlags(description.cpuAccessFlags)
        {
            Init(description.data);
        }

        uint32_t Buffer::GetSize() const
        {
            return m_size;
        }

        uint32_t Buffer::GetStride() const
        {
            return m_stride;
        }

        BufferUsageFlags Buffer::GetUsageFlags() const
        {
            return m_usageFlags;
        }

        BufferCpuAccessFlags Buffer::GetCpuAccessFlags() const
        {
            return m_cpuAccessFlags;
        }
    }
}