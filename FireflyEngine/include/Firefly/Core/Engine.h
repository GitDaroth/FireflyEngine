#pragma once

#include <chrono>
#include "Core/Application.h"

namespace Firefly
{
	class Engine
	{
	public:
		void Init();
		void Run();
		void Shutdown();

	private:
		float CalculateDeltaTime();

		Application* m_application;

		bool m_isRunning = true;
		std::chrono::steady_clock::time_point m_lastFrameTime;
	};

	extern Application* InstantiateApplication();
}