#pragma once

#include "Event.h"

namespace Firefly
{
	class GamepadEvent : public InputEvent 
	{
	public:
		GamepadEvent(int gamepadNumber, const std::string& gamepadName) :
			m_gamepadNumber(gamepadNumber), m_gamepadName(gamepadName) {}

		inline int GetGamepadNumber() { return m_gamepadNumber; }
		inline std::string GetGamepadName() { return m_gamepadName; }

	protected:
		int m_gamepadNumber;
		std::string m_gamepadName;
	};

	class GamepadConnectedEvent : public GamepadEvent
	{
	public:
		GamepadConnectedEvent(int gamepadNumber, const std::string& gamepadName) :
			GamepadEvent(gamepadNumber, gamepadName) {}

		virtual std::string ToString() const override
		{
			return "GamepadConnectedEvent: " + m_gamepadName + "(" + std::to_string(m_gamepadNumber) + ")";
		}
	};

	class GamepadDisconnectedEvent : public GamepadEvent
	{
	public:
		GamepadDisconnectedEvent(int gamepadNumber, const std::string& gamepadName) :
			GamepadEvent(gamepadNumber, gamepadName) {}

		virtual std::string ToString() const override
		{
			return "GamepadDisconnectedEvent: " + m_gamepadName + "(" + std::to_string(m_gamepadNumber) + ")";
		}
	};

	class GamepadButtonEvent : public GamepadEvent
	{
	public:
		GamepadButtonEvent(int gamepadNumber, const std::string& gamepadName, int buttonCode) :
			GamepadEvent(gamepadNumber, gamepadName), m_buttonCode(buttonCode) {}

		inline int GetButtonCode() const { return m_buttonCode; }

	protected:
		int m_buttonCode;
	};

	class GamepadButtonPressEvent : public GamepadButtonEvent
	{
	public:
		GamepadButtonPressEvent(int gamepadNumber, const std::string& gamepadName, int buttonCode) :
			GamepadButtonEvent(gamepadNumber, gamepadName, buttonCode) {}

		virtual std::string ToString() const override
		{
			return "GamepadButtonPressEvent: " + m_gamepadName + "(" + std::to_string(m_gamepadNumber) + ") " + std::to_string(m_buttonCode);
		}
	};

	class GamepadButtonReleaseEvent : public GamepadButtonEvent
	{
	public:
		GamepadButtonReleaseEvent(int gamepadNumber, const std::string& gamepadName, int buttonCode) :
			GamepadButtonEvent(gamepadNumber, gamepadName, buttonCode) {}

		virtual std::string ToString() const override
		{
			return "GamepadButtonReleaseEvent: " + m_gamepadName + "(" + std::to_string(m_gamepadNumber) + ") " + std::to_string(m_buttonCode);
		}
	};

	class GamepadAxisMoveEvent : public GamepadEvent
	{
	public:
		GamepadAxisMoveEvent(int gamepadNumber, const std::string& gamepadName, float xPos, float yPos) :
			GamepadEvent(gamepadNumber, gamepadName), m_xPos(xPos), m_yPos(yPos) {}

		inline float GetXPos() const { return m_xPos; }
		inline float GetYPos() const { return m_yPos; }

	protected:
		float m_xPos, m_yPos;
	};

	class GamepadAxisLeftMoveEvent : public GamepadAxisMoveEvent
	{
	public:
		GamepadAxisLeftMoveEvent(int gamepadNumber, const std::string& gamepadName, float xPos, float yPos) :
			GamepadAxisMoveEvent(gamepadNumber, gamepadName, xPos, yPos) {}

		virtual std::string ToString() const override
		{
			return "GamepadAxisLeftMoveEvent: " + m_gamepadName + "(" + std::to_string(m_gamepadNumber) + ") " + std::to_string(m_xPos) + ", " + std::to_string(m_yPos);
		}
	};

	class GamepadAxisRightMoveEvent : public GamepadAxisMoveEvent
	{
	public:
		GamepadAxisRightMoveEvent(int gamepadNumber, const std::string& gamepadName, float xPos, float yPos) :
			GamepadAxisMoveEvent(gamepadNumber, gamepadName, xPos, yPos) {}

		virtual std::string ToString() const override
		{
			return "GamepadAxisRightMoveEvent: " + m_gamepadName + "(" + std::to_string(m_gamepadNumber) + ") " + std::to_string(m_xPos) + ", " + std::to_string(m_yPos);
		}
	};

	class GamepadTriggerMoveEvent : public GamepadEvent
	{
	public:
		GamepadTriggerMoveEvent(int gamepadNumber, const std::string& gamepadName, float pos) :
			GamepadEvent(gamepadNumber, gamepadName), m_pos(pos) {}

		inline float GetPos() const { return m_pos; }

	protected:
		float m_pos;
	};

	class GamepadTriggerLeftMoveEvent : public GamepadTriggerMoveEvent
	{
	public:
		GamepadTriggerLeftMoveEvent(int gamepadNumber, const std::string& gamepadName, float pos) :
			GamepadTriggerMoveEvent(gamepadNumber, gamepadName, pos) {}

		virtual std::string ToString() const override
		{
			return "GamepadTriggerLeftMoveEvent: " + m_gamepadName + "(" + std::to_string(m_gamepadNumber) + ") " + std::to_string(m_pos);
		}
	};

	class GamepadTriggerRightMoveEvent : public GamepadTriggerMoveEvent
	{
	public:
		GamepadTriggerRightMoveEvent(int gamepadNumber, const std::string& gamepadName, float pos) :
			GamepadTriggerMoveEvent(gamepadNumber, gamepadName, pos) {}

		virtual std::string ToString() const override
		{
			return "GamepadTriggerRightMoveEvent: " + m_gamepadName + "(" + std::to_string(m_gamepadNumber) + ") " + std::to_string(m_pos);
		}
	};
}