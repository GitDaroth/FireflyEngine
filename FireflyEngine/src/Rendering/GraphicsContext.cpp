#include "pch.h"
#include "Rendering/GraphicsContext.h"

#include "Window/Window.h"

namespace Firefly
{
    void GraphicsContext::Init(std::shared_ptr<Window> window)
    {
        m_window = window;
        OnInit(m_window);
    }

    uint32_t GraphicsContext::GetWidth() const
    {
        return m_window->GetWidth();
    }

    uint32_t GraphicsContext::GetHeight() const
    {
        return m_window->GetHeight();
    }
}