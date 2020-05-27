#include "pch.h"
#include "Game.h"

#include "Window/WindowsWindow.h"

namespace Firefly
{
	Game::Game()
	{
		#ifdef FIREFLY_WINDOWS
			m_window = std::make_unique<WindowsWindow>();
		#endif
	}

	Game::~Game()
	{
	}

	void Game::Run()
	{
		while (m_isRunning)
		{
			m_window->OnUpdate();
		}
	}
}