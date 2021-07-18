#pragma once

#include "Rendering/Buffer.h"

namespace Firefly
{
    namespace Rendering
    {
        class OpenGLBuffer : Buffer
        {
        public:
            OpenGLBuffer(const BufferDescription& description);
            virtual void Destroy() override;

            uint32_t GetHandle() const;

        protected:
            virtual void Init(void* data) override;

        private:
            uint32_t m_handle;
        };
    }
}