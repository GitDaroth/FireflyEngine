#pragma once

#include "Core.h"
#include "Window/Window.h"
#include "Event/WindowEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

namespace Firefly
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void OnEvent(std::shared_ptr<Event> event);

	protected:
		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnWindowEvent(std::shared_ptr<WindowEvent> event) = 0;
		virtual void OnKeyEvent(std::shared_ptr<KeyEvent> event) = 0;
		virtual void OnMouseEvent(std::shared_ptr<MouseEvent> event) = 0;

		std::unique_ptr<Window> m_window;
		bool m_isRunning = true;
		float m_lastFrameTime;
	};
}