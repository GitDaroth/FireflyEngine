#pragma once

#include "Core.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "Input/KeyCodes.h"
#include "Input/MouseButtonCodes.h"

namespace Firefly
{
	class FIREFLY_API Input
	{
	public:
		static bool IsKeyPressed(int keyCode);
		static bool IsMouseButtonPressed(int mouseButton);
		static int GetMousePositionX();
		static int GetMousePositionY();

		static void OnEvent(std::shared_ptr<Event> event);

	private:
		static std::unordered_map<int, bool> m_isKeyPressedMap;
		static std::unordered_map<int, bool> m_isMouseButtonPressedMap;
		static int m_mousePositionX;
		static int m_mousePositionY;
	};
}