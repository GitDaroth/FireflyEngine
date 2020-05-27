#pragma once

#include "Core.h"
#include "Window/Window.h"

namespace Firefly
{
	class FIREFLY_API Game
	{
	public:
		Game();
		virtual ~Game();

		void Run();

	private:
		std::unique_ptr<Window> m_window;
		bool m_isRunning = true;
	};
}