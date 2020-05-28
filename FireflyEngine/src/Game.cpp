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

		m_window->SetEventCallback(std::bind(&Game::OnEvent, this, std::placeholders::_1));
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

	void Game::OnEvent(std::shared_ptr<Event> event)
	{
		Logger::Debug("FireflyEngine", event->ToString());
	}
}