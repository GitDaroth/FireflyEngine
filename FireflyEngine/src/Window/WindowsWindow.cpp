#include "pch.h"
#include "Window/WindowsWindow.h"

#include "Rendering/RenderingAPI.h"
#include "Event/WindowEvent.h"
#include "Input/Input.h"

#include "Rendering/Vulkan/VulkanContext.h"

namespace Firefly
{
	bool WindowsWindow::s_isWindowInitialized = false;
	int WindowsWindow::s_windowCount = 0;

	WindowsWindow::WindowsWindow(const std::string& title, int width, int height) :
		Window(title, width, height)
	{
		if (!s_isWindowInitialized)
		{
			int success = glfwInit();
			FIREFLY_ASSERT(success, "Unable to initialize GLFW!");
			s_isWindowInitialized = true;

			glfwSetErrorCallback([](int code, const char* description)
			{
				Logger::Error("FireflyEngine", "GLFW error [{0}]: {1}", code, description);
			});
		}

		//glfwWindowHint(GLFW_SAMPLES, 8);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // tell GLFW not to use OpenGL for context creation
		m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		FIREFLY_ASSERT(m_window, "Unable to create window with GLFW!");
		s_windowCount++;

		//m_context = RenderingAPI::CreateContext();
		//m_context->Init(m_window);
		VulkanContext::GetSingleton()->Init(m_window);

		SetupWindowEvents();
		SetupInputEvents();
	}

	WindowsWindow::~WindowsWindow()
	{
		glfwDestroyWindow(m_window);
		s_windowCount--;
		if (s_windowCount == 0)
			glfwTerminate();
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		PollGamepadEvents();
		VulkanContext::GetSingleton()->Draw();
		//m_context->SwapBuffers();
	}

	void WindowsWindow::OnSetTitle(const std::string& title)
	{
		glfwSetWindowTitle(m_window, title.c_str());
	}

	void WindowsWindow::OnSetSize(int width, int height)
	{
		int windowWidth, windowHeight;
		glfwGetWindowSize(m_window, &windowWidth, &windowHeight);
		if(width != windowWidth || height != windowHeight)
			glfwSetWindowSize(m_window, width, height);
	}

	void WindowsWindow::OnEnableVSync(bool enabled)
	{
		//if (enabled)
		//	glfwSwapInterval(1);
		//else
		//	glfwSwapInterval(0);
	}

	void WindowsWindow::SetupWindowEvents()
	{
		glfwSetWindowUserPointer(m_window, this);

		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* glfwWindow, int width, int height)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			window->SetSize(width, height);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event = std::make_shared<WindowResizeEvent>(width, height);
			eventCallback(event);
		});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* glfwWindow)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event = std::make_shared<WindowCloseEvent>();
			eventCallback(event);
		});

		glfwSetWindowMaximizeCallback(m_window, [](GLFWwindow* glfwWindow, int maximized)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event;
			if (maximized == GLFW_TRUE)
				event = std::make_shared<WindowMaximizeEvent>();
			else
				event = std::make_shared<WindowRestoreEvent>();
			eventCallback(event);
		});

		glfwSetWindowIconifyCallback(m_window, [](GLFWwindow* glfwWindow, int iconified)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event;
			if (iconified == GLFW_TRUE)
				event = std::make_shared<WindowMinimizeEvent>();
			else
				event = std::make_shared<WindowRestoreEvent>();
			eventCallback(event);
		});

		glfwSetWindowPosCallback(m_window, [](GLFWwindow* glfwWindow, int xPos, int yPos)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event = std::make_shared<WindowMoveEvent>(xPos, yPos);
			eventCallback(event);
		});
	}

	void WindowsWindow::SetupInputEvents()
	{
		glfwSetKeyCallback(m_window, [](GLFWwindow* glfwWindow, int key, int scancode, int action, int mods)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			int keyCode = window->ToFireflyKeyCode(key);
			std::shared_ptr<Event> event;
			switch (action)
			{
				case GLFW_PRESS:
					event = std::make_shared<KeyPressEvent>(keyCode);
					break;
				case GLFW_RELEASE:
					event = std::make_shared<KeyReleaseEvent>(keyCode);
					break;
				case GLFW_REPEAT:
					event = std::make_shared<KeyRepeatEvent>(keyCode);
					break;
			}
			eventCallback(event);
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* glfwWindow, int button, int action, int mods)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			int buttonCode = window->ToFireflyMouseButtonCode(button);
			std::shared_ptr<Event> event;
			switch (action)
			{
				case GLFW_PRESS:
					event = std::make_shared<MouseButtonPressEvent>(buttonCode);
					break;
				case GLFW_RELEASE:
					event = std::make_shared<MouseButtonReleaseEvent>(buttonCode);
					break;
			}
			eventCallback(event);
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow* glfwWindow, double xOffset, double yOffset)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event = std::make_shared<MouseScrollEvent>((float)xOffset, (float)yOffset);
			eventCallback(event);
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* glfwWindow, double xPos, double yPos)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event = std::make_shared<MouseMoveEvent>((int)xPos, (int)yPos);
			eventCallback(event);
		});
	}

	void WindowsWindow::PollGamepadEvents()
	{
		for (int gamepadNumber = 0; gamepadNumber < 16; gamepadNumber++)
		{
			if (glfwJoystickIsGamepad(gamepadNumber) && !Input::IsGamepadConnected(gamepadNumber))
			{
				std::string gamepadName = glfwGetGamepadName(gamepadNumber);
				std::shared_ptr<Event> event = std::make_shared<GamepadConnectedEvent>(gamepadNumber, gamepadName);
				m_eventCallback(event);
			}
			else if (!glfwJoystickIsGamepad(gamepadNumber) && Input::IsGamepadConnected(gamepadNumber))
			{
				std::shared_ptr<Event> event = std::make_shared<GamepadDisconnectedEvent>(gamepadNumber, Input::GetGamepadName(gamepadNumber));
				m_eventCallback(event);
			}

			if (!glfwJoystickIsGamepad(gamepadNumber))
				continue;

			std::string gamepadName = glfwGetGamepadName(gamepadNumber);
			GLFWgamepadstate state;
			if (glfwGetGamepadState(gamepadNumber, &state))
			{
				for (int button = 0; button < 15; button++) // there are 15 buttons
				{
					auto action = state.buttons[button];
					int buttonCode = ToFireflyGamepadButtonCode(button);
					switch (action)
					{
					case GLFW_PRESS:
						if (!Input::IsGamepadButtonPressed(gamepadNumber, buttonCode))
						{
							std::shared_ptr<Event> event = std::make_shared<GamepadButtonPressEvent>(gamepadNumber, gamepadName, buttonCode);
							m_eventCallback(event);
						}
						break;
					case GLFW_RELEASE:
						if (Input::IsGamepadButtonPressed(gamepadNumber, buttonCode))
						{
							std::shared_ptr<Event> event = std::make_shared<GamepadButtonReleaseEvent>(gamepadNumber, gamepadName, buttonCode);
							m_eventCallback(event);
						}
						break;
					}
				}

				float epsilon = 0.01f;
				float axisLeftX = ApplyGamepadAxisDeadZone(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]);
				float axisLeftY = ApplyGamepadAxisDeadZone(state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
				bool hasChangedAxisLeftX = fabsf(axisLeftX - Input::GetGamepadAxisLeftX(gamepadNumber)) >= epsilon;
				bool hasChangedAxisLeftY = fabsf(axisLeftY - Input::GetGamepadAxisLeftY(gamepadNumber)) >= epsilon;
				if (hasChangedAxisLeftX || hasChangedAxisLeftY)
				{
					std::shared_ptr<Event> event = std::make_shared<GamepadAxisLeftMoveEvent>(gamepadNumber, gamepadName, axisLeftX, axisLeftY);
					m_eventCallback(event);
				}

				float axisRightX = ApplyGamepadAxisDeadZone(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]);
				float axisRightY = ApplyGamepadAxisDeadZone(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
				bool hasChangedAxisRightX = fabsf(axisRightX - Input::GetGamepadAxisRightX(gamepadNumber)) >= epsilon;
				bool hasChangedAxisRightY = fabsf(axisRightY - Input::GetGamepadAxisRightY(gamepadNumber)) >= epsilon;
				if (hasChangedAxisRightX || hasChangedAxisRightY)
				{
					std::shared_ptr<Event> event = std::make_shared<GamepadAxisRightMoveEvent>(gamepadNumber, gamepadName, axisRightX, axisRightY);
					m_eventCallback(event);
				}

				float triggerLeft = state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
				bool hasChangedTriggerLeft = fabsf(triggerLeft - Input::GetGamepadTriggerLeft(gamepadNumber)) >= epsilon;
				if (hasChangedTriggerLeft)
				{
					std::shared_ptr<Event> event = std::make_shared<GamepadTriggerLeftMoveEvent>(gamepadNumber, gamepadName, triggerLeft);
					m_eventCallback(event);
				}

				float triggerRight = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];
				bool hasChangedTriggerRight = fabsf(triggerRight - Input::GetGamepadTriggerRight(gamepadNumber)) >= epsilon;
				if (hasChangedTriggerRight)
				{
					std::shared_ptr<Event> event = std::make_shared<GamepadTriggerRightMoveEvent>(gamepadNumber, gamepadName, triggerRight);
					m_eventCallback(event);
				}
			}
		}
	}

	float WindowsWindow::ApplyGamepadAxisDeadZone(float axisValue)
	{
		float deadZone = 0.15f;

		float signAxis = axisValue < 0.f ? -1.f : 1.f;
		axisValue = std::max(fabsf(axisValue), deadZone);
		// map [deadZone,1] to [0,1] and [-1,-deadZone] to [-1,0]
		axisValue = (axisValue - deadZone) / (1.f - deadZone);
		return axisValue < 0.01f ? 0.f : signAxis * axisValue;
	}

	void WindowsWindow::SetupKeyCodeConversionMap()
	{
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_UNKNOWN, FIREFLY_KEY_UNKNOWN));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_SPACE, FIREFLY_KEY_SPACE));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_APOSTROPHE, FIREFLY_KEY_APOSTROPHE));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_COMMA, FIREFLY_KEY_COMMA));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_MINUS, FIREFLY_KEY_MINUS));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_PERIOD, FIREFLY_KEY_PERIOD));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_SLASH, FIREFLY_KEY_SLASH));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_0, FIREFLY_KEY_0));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_1, FIREFLY_KEY_1));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_2, FIREFLY_KEY_2));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_3, FIREFLY_KEY_3));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_4, FIREFLY_KEY_4));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_5, FIREFLY_KEY_5));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_6, FIREFLY_KEY_6));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_7, FIREFLY_KEY_7));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_8, FIREFLY_KEY_8));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_9, FIREFLY_KEY_9));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_SEMICOLON, FIREFLY_KEY_SEMICOLON));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_EQUAL, FIREFLY_KEY_EQUAL));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_A, FIREFLY_KEY_A));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_B, FIREFLY_KEY_B));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_C, FIREFLY_KEY_C));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_D, FIREFLY_KEY_D));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_E, FIREFLY_KEY_E));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F, FIREFLY_KEY_F));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_G, FIREFLY_KEY_G));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_H, FIREFLY_KEY_H));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_I, FIREFLY_KEY_I));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_J, FIREFLY_KEY_J));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_K, FIREFLY_KEY_K));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_L, FIREFLY_KEY_L));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_M, FIREFLY_KEY_M));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_N, FIREFLY_KEY_N));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_O, FIREFLY_KEY_O));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_P, FIREFLY_KEY_P));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_Q, FIREFLY_KEY_Q));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_R, FIREFLY_KEY_R));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_S, FIREFLY_KEY_S));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_T, FIREFLY_KEY_T));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_U, FIREFLY_KEY_U));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_V, FIREFLY_KEY_V));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_W, FIREFLY_KEY_W));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_X, FIREFLY_KEY_X));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_Y, FIREFLY_KEY_Y));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_Z, FIREFLY_KEY_Z));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_LEFT_BRACKET, FIREFLY_KEY_LEFT_BRACKET));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_BACKSLASH, FIREFLY_KEY_BACKSLASH));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_RIGHT_BRACKET, FIREFLY_KEY_RIGHT_BRACKET));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_GRAVE_ACCENT, FIREFLY_KEY_GRAVE_ACCENT));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_WORLD_1, FIREFLY_KEY_WORLD_1));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_WORLD_2, FIREFLY_KEY_WORLD_2));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_ESCAPE, FIREFLY_KEY_ESCAPE));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_ENTER, FIREFLY_KEY_ENTER));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_TAB, FIREFLY_KEY_TAB));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_BACKSPACE, FIREFLY_KEY_BACKSPACE));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_INSERT, FIREFLY_KEY_INSERT));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_DELETE, FIREFLY_KEY_DELETE));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_RIGHT, FIREFLY_KEY_RIGHT));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_LEFT, FIREFLY_KEY_LEFT));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_DOWN, FIREFLY_KEY_DOWN));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_UP, FIREFLY_KEY_UP));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_PAGE_UP, FIREFLY_KEY_PAGE_UP));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_PAGE_DOWN, FIREFLY_KEY_PAGE_DOWN));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_HOME, FIREFLY_KEY_HOME));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_END, FIREFLY_KEY_END));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_CAPS_LOCK, FIREFLY_KEY_CAPS_LOCK));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_SCROLL_LOCK, FIREFLY_KEY_SCROLL_LOCK));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_NUM_LOCK, FIREFLY_KEY_NUM_LOCK));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_PRINT_SCREEN, FIREFLY_KEY_PRINT_SCREEN));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_PAUSE, FIREFLY_KEY_PAUSE));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F1, FIREFLY_KEY_F1));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F2, FIREFLY_KEY_F2));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F3, FIREFLY_KEY_F3));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F4, FIREFLY_KEY_F4));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F5, FIREFLY_KEY_F5));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F6, FIREFLY_KEY_F6));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F7, FIREFLY_KEY_F7));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F8, FIREFLY_KEY_F8));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F9, FIREFLY_KEY_F9));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F10, FIREFLY_KEY_F10));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F11, FIREFLY_KEY_F11));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F12, FIREFLY_KEY_F12));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F13, FIREFLY_KEY_F13));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F14, FIREFLY_KEY_F14));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F15, FIREFLY_KEY_F15));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F16, FIREFLY_KEY_F16));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F17, FIREFLY_KEY_F17));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F18, FIREFLY_KEY_F18));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F19, FIREFLY_KEY_F19));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F20, FIREFLY_KEY_F20));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F21, FIREFLY_KEY_F21));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F22, FIREFLY_KEY_F22));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F23, FIREFLY_KEY_F23));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F24, FIREFLY_KEY_F24));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_F25, FIREFLY_KEY_F25));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_0, FIREFLY_KEY_KP_0));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_1, FIREFLY_KEY_KP_1));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_2, FIREFLY_KEY_KP_2));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_3, FIREFLY_KEY_KP_3));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_4, FIREFLY_KEY_KP_4));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_5, FIREFLY_KEY_KP_5));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_6, FIREFLY_KEY_KP_6));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_7, FIREFLY_KEY_KP_7));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_8, FIREFLY_KEY_KP_8));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_9, FIREFLY_KEY_KP_9));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_DECIMAL, FIREFLY_KEY_KP_DECIMAL));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_DIVIDE, FIREFLY_KEY_KP_DIVIDE));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_MULTIPLY, FIREFLY_KEY_KP_MULTIPLY));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_SUBTRACT, FIREFLY_KEY_KP_SUBTRACT));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_ADD, FIREFLY_KEY_KP_ADD));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_ENTER, FIREFLY_KEY_KP_ENTER));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_KP_EQUAL, FIREFLY_KEY_KP_EQUAL));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_LEFT_SHIFT, FIREFLY_KEY_LEFT_SHIFT));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_LEFT_CONTROL, FIREFLY_KEY_LEFT_CONTROL));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_LEFT_ALT, FIREFLY_KEY_LEFT_ALT));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_LEFT_SUPER, FIREFLY_KEY_LEFT_SUPER));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_RIGHT_SHIFT, FIREFLY_KEY_RIGHT_SHIFT));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_RIGHT_CONTROL, FIREFLY_KEY_RIGHT_CONTROL));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_RIGHT_ALT, FIREFLY_KEY_RIGHT_ALT));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_RIGHT_SUPER, FIREFLY_KEY_RIGHT_SUPER));
		m_keyCodeConversionMap.insert(std::pair<int,int>(GLFW_KEY_MENU, FIREFLY_KEY_MENU));
	}

	void WindowsWindow::SetupMouseButtonCodeConversionMap()
	{
		m_mouseButtonCodeConversionMap.insert(std::pair<int,int>(GLFW_MOUSE_BUTTON_1, FIREFLY_MOUSE_BUTTON_1));
		m_mouseButtonCodeConversionMap.insert(std::pair<int,int>(GLFW_MOUSE_BUTTON_2, FIREFLY_MOUSE_BUTTON_2));
		m_mouseButtonCodeConversionMap.insert(std::pair<int,int>(GLFW_MOUSE_BUTTON_3, FIREFLY_MOUSE_BUTTON_3));
		m_mouseButtonCodeConversionMap.insert(std::pair<int,int>(GLFW_MOUSE_BUTTON_4, FIREFLY_MOUSE_BUTTON_4));
		m_mouseButtonCodeConversionMap.insert(std::pair<int,int>(GLFW_MOUSE_BUTTON_5, FIREFLY_MOUSE_BUTTON_5));
		m_mouseButtonCodeConversionMap.insert(std::pair<int,int>(GLFW_MOUSE_BUTTON_6, FIREFLY_MOUSE_BUTTON_6));
		m_mouseButtonCodeConversionMap.insert(std::pair<int,int>(GLFW_MOUSE_BUTTON_7, FIREFLY_MOUSE_BUTTON_7));
		m_mouseButtonCodeConversionMap.insert(std::pair<int,int>(GLFW_MOUSE_BUTTON_8, FIREFLY_MOUSE_BUTTON_8));
		m_mouseButtonCodeConversionMap.insert(std::pair<int,int>(GLFW_MOUSE_BUTTON_LEFT, FIREFLY_MOUSE_BUTTON_LEFT));
		m_mouseButtonCodeConversionMap.insert(std::pair<int,int>(GLFW_MOUSE_BUTTON_RIGHT, FIREFLY_MOUSE_BUTTON_RIGHT));
		m_mouseButtonCodeConversionMap.insert(std::pair<int,int>(GLFW_MOUSE_BUTTON_MIDDLE, FIREFLY_MOUSE_BUTTON_MIDDLE));
	}

	void WindowsWindow::SetupGamepadButtonCodeConversionMap()
	{
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_A, FIREFLY_GAMEPAD_BUTTON_A));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_B, FIREFLY_GAMEPAD_BUTTON_B));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_X, FIREFLY_GAMEPAD_BUTTON_X));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_Y, FIREFLY_GAMEPAD_BUTTON_Y));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, FIREFLY_GAMEPAD_BUTTON_LEFT_BUMPER));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, FIREFLY_GAMEPAD_BUTTON_RIGHT_BUMPER));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_BACK, FIREFLY_GAMEPAD_BUTTON_BACK));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_START, FIREFLY_GAMEPAD_BUTTON_START));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_GUIDE, FIREFLY_GAMEPAD_BUTTON_GUIDE));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_LEFT_THUMB, FIREFLY_GAMEPAD_BUTTON_LEFT_THUMB));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, FIREFLY_GAMEPAD_BUTTON_RIGHT_THUMB));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_DPAD_UP, FIREFLY_GAMEPAD_BUTTON_DPAD_UP));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, FIREFLY_GAMEPAD_BUTTON_DPAD_RIGHT));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, FIREFLY_GAMEPAD_BUTTON_DPAD_DOWN));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, FIREFLY_GAMEPAD_BUTTON_DPAD_LEFT));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_CROSS, FIREFLY_GAMEPAD_BUTTON_CROSS));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_CIRCLE, FIREFLY_GAMEPAD_BUTTON_CIRCLE));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_SQUARE, FIREFLY_GAMEPAD_BUTTON_SQUARE));
		m_gamepadButtonCodeConversionMap.insert(std::pair<int, int>(GLFW_GAMEPAD_BUTTON_TRIANGLE, FIREFLY_GAMEPAD_BUTTON_TRIANGLE));
	}
}