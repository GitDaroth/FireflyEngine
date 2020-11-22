#pragma once

#include "Input/GamepadButtonCodes.h"

#define FIREFLY_MAX_GAMEPADS	16
#define FIREFLY_GAMEPAD_1		0
#define FIREFLY_GAMEPAD_2		1
#define FIREFLY_GAMEPAD_3		2
#define FIREFLY_GAMEPAD_4		3
#define FIREFLY_GAMEPAD_5		4
#define FIREFLY_GAMEPAD_6		5
#define FIREFLY_GAMEPAD_7		6
#define FIREFLY_GAMEPAD_8		7
#define FIREFLY_GAMEPAD_9		8
#define FIREFLY_GAMEPAD_10		9
#define FIREFLY_GAMEPAD_11		10
#define FIREFLY_GAMEPAD_12		11
#define FIREFLY_GAMEPAD_13		12
#define FIREFLY_GAMEPAD_14		13
#define FIREFLY_GAMEPAD_15		14
#define FIREFLY_GAMEPAD_16		15

namespace Firefly
{
	class Gamepad
	{
	public:
		Gamepad(const std::string& name);
		Gamepad(const Gamepad& gamepad);
		~Gamepad();

		void SetName(const std::string& name);
		std::string GetName() const;

		void SetIsButtonPressed(int buttonCode, bool isPressed);
		bool IsButtonPressed(int buttonCode) const;

		void SetAxisLeft(float xPos, float yPos);
		float GetAxisLeftX() const;
		float GetAxisLeftY() const;
		void SetAxisRight(float xPos, float yPos);
		float GetAxisRightX() const;
		float GetAxisRightY() const;
		void SetTriggerLeft(float pos);
		float GetTriggerLeft() const;
		void SetTriggerRight(float pos);
		float GetTriggerRight() const;

	private:
		std::string m_name;
		std::unordered_map<int, bool> m_isButtonPressedMap;
		float m_axisLeftX;
		float m_axisLeftY;
		float m_axisRightX;
		float m_axisRightY;
		float m_triggerLeft;
		float m_triggerRight;
	};
}