#pragma once

#include "Window.h"
#include <GLFW/glfw3.h>

namespace Firefly
{
    class WindowsWindow : public Window
    {
    public:
        WindowsWindow(const std::string& title, int width, int height);
        virtual ~WindowsWindow();

        virtual void OnUpdate(float deltaTime) override;

        virtual int GetHeight() const override;
        virtual int GetWidth() const override;
        GLFWwindow* GetGlfwWindow();

    protected:
        virtual void OnSetTitle(const std::string& title) override;
        virtual void OnSetSize(int width, int height) override;
        virtual void SetupKeyCodeConversionMap() override;
        virtual void SetupMouseButtonCodeConversionMap() override;
        virtual void SetupGamepadButtonCodeConversionMap() override;

    private:
        void SetupWindowEvents();
        void SetupInputEvents();
        void PollGamepadEvents();
        float ApplyGamepadAxisDeadZone(float axisValue);

        GLFWwindow* m_window;
    };
}