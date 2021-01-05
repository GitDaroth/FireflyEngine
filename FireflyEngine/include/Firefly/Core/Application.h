#pragma once

#include "Window/Window.h"
#include "Event/WindowEvent.h"
#include "Input/Input.h"

namespace Firefly
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Update(float deltaTime);
		void OnEvent(std::shared_ptr<Event> event);

		void RequestShutdown();
		bool IsShutdownRequested() const;

	protected:
		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnWindowEvent(std::shared_ptr<WindowEvent> event) = 0;
		virtual void OnKeyEvent(std::shared_ptr<KeyEvent> event) = 0;
		virtual void OnMouseEvent(std::shared_ptr<MouseEvent> event) = 0;
		virtual void OnGamepadEvent(std::shared_ptr<GamepadEvent> event) = 0;

		std::unique_ptr<Window> m_window;
		bool m_isShutdownRequested = false;
	};
}