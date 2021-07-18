#include <pch.h>
#include "Rendering/OpenGL/OpenGLBuffer.h"

#include <glad/glad.h>

namespace Firefly
{
    namespace Rendering
    {
        OpenGLBuffer::OpenGLBuffer(const BufferDescription& description) :
            Buffer(description),
            m_handle(0)
        {
        }

        void OpenGLBuffer::Destroy()
        {
            glDeleteBuffers(1, &m_handle);
        }

        uint32_t OpenGLBuffer::GetHandle() const
        {
            return m_handle;
        }

        void OpenGLBuffer::Init(void* data)
        {
            glCreateBuffers(1, &m_handle);

            GLbitfield bufferFlags = 0;
            if ((m_cpuAccessFlags & BufferCpuAccessFlagBits::READ) || (m_cpuAccessFlags & BufferCpuAccessFlagBits::WRITE))
                bufferFlags |= GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
            if (m_cpuAccessFlags & BufferCpuAccessFlagBits::READ)
                bufferFlags |= GL_MAP_READ_BIT;
            if (m_cpuAccessFlags & BufferCpuAccessFlagBits::WRITE)
                bufferFlags |= GL_MAP_WRITE_BIT;

            glNamedBufferStorage(m_handle, m_size, data, bufferFlags);
        }
    }
}