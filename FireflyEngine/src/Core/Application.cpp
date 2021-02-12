#include "pch.h"
#include "Core/Application.h"

#include "Window/WindowsWindow.h"
#include "Input/Input.h"
#include "Core/ResourceRegistry.h"

namespace Firefly
{
    Application::Application()
    {
        m_window = std::make_shared<WindowsWindow>("Firefly Engine", 1280, 720);
        m_window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

        RenderingAPI::Init(m_window);
    }

    Application::~Application()
    {
        MeshRegistry::Instance().DestroyResources();
        ShaderRegistry::Instance().DestroyResources();
        MaterialRegistry::Instance().DestroyResources();
        TextureRegistry::Instance().DestroyResources();

        RenderingAPI::Destroy();
    }

    void Application::Update(float deltaTime)
    {
        m_window->SetTitle(std::to_string(1.f / deltaTime));
        m_window->OnUpdate(deltaTime);
        OnUpdate(deltaTime);
    }

    void Application::RequestShutdown()
    {
        m_isShutdownRequested = true;
    }

    bool Application::IsShutdownRequested() const
    {
        return m_isShutdownRequested;
    }

    void Application::OnEvent(std::shared_ptr<Event> event)
    {
        Input::OnEvent(event);

        if (auto windowEvent = event->AsType<WindowEvent>())
        {
            OnWindowEvent(windowEvent);

            if (auto closeEvent = windowEvent->AsType<WindowCloseEvent>())
            {
                RequestShutdown();
            }
            else if (auto resizeEvent = windowEvent->AsType<WindowResizeEvent>())
            {
                int width = resizeEvent->GetWidth();
                int height = resizeEvent->GetHeight();
            }
        }
        else if (auto keyEvent = event->AsType<KeyEvent>())
        {
            OnKeyEvent(keyEvent);
        }
        else if (auto mouseEvent = event->AsType<MouseEvent>())
        {
            OnMouseEvent(mouseEvent);
        }
        else if (auto gamepadEvent = event->AsType<GamepadEvent>())
        {
            OnGamepadEvent(gamepadEvent);
        }
    }
}