#include "pch.h"
#include "Game.h"

#include "Window/WindowsWindow.h"
#include "Event/MouseEvent.h"

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
		MouseScrollEvent mouseScrollEvent(0.2f, 0.f);
		Event* event = &mouseScrollEvent;
		if (event->IsType<InputEvent>())
			Logger::Debug("FireflyEngein", "Is InputEvent");
		if (event->IsType<MouseEvent>())
			Logger::Debug("FireflyEngein", "Is MouseEvent");
		if (event->IsType<MouseButtonEvent>())
			Logger::Debug("FireflyEngein", "Is MouseButtonEvent");

		Logger::Debug("FireflyEngein", event->AsType<MouseScrollEvent>()->ToString());

		while (m_isRunning)
		{
			m_window->OnUpdate();
		}
	}
}