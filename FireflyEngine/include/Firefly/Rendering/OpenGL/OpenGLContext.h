#pragma once

#include <glad/glad.h>
#include "Rendering/GraphicsContext.h"

struct GLFWwindow;

namespace Firefly
{
    class OpenGLContext : public GraphicsContext
    {
    public:
        virtual void Destroy() override;

        void SwapBuffers();

    protected:
        virtual void OnInit(std::shared_ptr<Window> window) override;

    private:
        void PrintGpuInfo();

        GLFWwindow* m_glfwWindow;

        static void GLAPIENTRY DebugMessengerCallback(
            GLenum source, GLenum type, GLuint id,
            GLenum severity, GLsizei length,
            const GLchar* message, const void* userParam);
    };
}