#include "pch.h"
#include "Core/Engine.h"

#include "Core/Application.h"

namespace Firefly
{
    void Engine::Init()
    {
        if (m_isInitialized)
            return;

        Logger::Init();
        Logger::Info(FIREFLY_ENGINE_NAME, "Version: {0}.{1}.{2}",
            FIREFLY_ENGINE_VERSION_MAJOR, FIREFLY_ENGINE_VERSION_MINOR, FIREFLY_ENGINE_VERSION_PATCH);

        m_application = Firefly::InstantiateApplication();

        m_isInitialized = true;
    }

    void Engine::Run()
    {
        if (!m_isInitialized)
            return;

        m_lastFrameTime = std::chrono::steady_clock::now();
        while (m_isRunning)
        {
            float deltaTime = CalculateDeltaTime();

            m_application->Update(deltaTime);

            if (m_application->IsShutdownRequested())
                m_isRunning = false;
        }
    }

    void Engine::Shutdown()
    {
        if (!m_isInitialized)
            return;

        delete m_application;
    }

    float Engine::CalculateDeltaTime()
    {
        auto currentFrameTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(currentFrameTime - m_lastFrameTime);
        float deltaTime = duration.count();
        m_lastFrameTime = currentFrameTime;

        return deltaTime;
    }
}