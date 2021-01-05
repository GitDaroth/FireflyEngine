#include "pch.h"
#include "Core/Engine.h"

#include <GLFW/glfw3.h>

namespace Firefly
{
	void Engine::Init()
	{
		Logger::Init();
		m_application = Firefly::InstantiateApplication();
	}

	void Engine::Run()
	{
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