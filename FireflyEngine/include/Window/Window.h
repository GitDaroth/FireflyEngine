#pragma once

#include "Core/Core.h"
#include "Event/Event.h"
#include "Rendering/GraphicsContext.h"

namespace Firefly
{
	class Window
	{
	public:
		Window(const std::string& title = "Firefly Engine", int width = 1280, int height = 720);
		virtual ~Window();

		void SetEventCallback(const std::function<void(std::shared_ptr<Event>)>& eventCallback);
		void SetTitle(const std::string& title);
		void SetSize(int width, int height);
		void EnableVSync(bool enabled);

		const std::function<void(std::shared_ptr<Event>)>& GetEventCallback();
		const std::string& GetTitle() const;
		int GetWidth() const;
		int GetHeight() const;
		bool IsVSyncEnabled() const;

		int ToFireflyKeyCode(int keyCode) const;
		int ToFireflyMouseButtonCode(int keyCode) const;

		virtual void OnUpdate() = 0;

	protected:
		virtual void OnSetTitle(const std::string& title) = 0;
		virtual void OnSetSize(int width, int height) = 0;
		virtual void OnEnableVSync(bool enabled) = 0;
		virtual void SetupKeyCodeConversionMap() = 0;
		virtual void SetupMouseButtonCodeConversionMap() = 0;

		std::function<void(std::shared_ptr<Event>)> m_eventCallback;
		std::shared_ptr<GraphicsContext> m_context;
		std::string m_title;
		int m_width;
		int m_height;
		bool m_isVSyncEnabled;
		std::unordered_map<int, int> m_keyCodeConversionMap; // SpecificKeyCode, FireflyKeyCode
		std::unordered_map<int, int> m_mouseButtonCodeConversionMap; // SpecificMouseButtonCode, FireflyMouseButtonCode
	};
}