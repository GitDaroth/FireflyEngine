#pragma once

#include <chrono>

namespace Firefly
{
	struct Application;

	class Engine
	{
	public:
		void Init();
		void Run();
		void Shutdown();

	private:
		float CalculateDeltaTime();

		Application* m_application;

		bool m_isInitialized = false;
		bool m_isRunning = true;
		std::chrono::steady_clock::time_point m_lastFrameTime;
	};

	extern Application* InstantiateApplication();
}