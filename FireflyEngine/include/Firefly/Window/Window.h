#pragma once

#include "Core/Core.h"
#include "Event/Event.h"
#include "Rendering/GraphicsContext.h"

namespace Firefly
{
    class Window
    {
    public:
        Window(const std::string& title, int width, int height);
        virtual ~Window();

        void SetEventCallback(const std::function<void(std::shared_ptr<Event>)>& eventCallback);
        void SetTitle(const std::string& title);
        void SetSize(int width, int height);

        const std::function<void(std::shared_ptr<Event>)>& GetEventCallback();
        const std::string& GetTitle() const;
        virtual int GetWidth() const = 0;
        virtual int GetHeight() const = 0;

        int ToFireflyKeyCode(int keyCode) const;
        int ToFireflyMouseButtonCode(int keyCode) const;
        int ToFireflyGamepadButtonCode(int keyCode) const;

        virtual void OnUpdate(float deltaTime) = 0;

    protected:
        virtual void OnSetTitle(const std::string& title) = 0;
        virtual void OnSetSize(int width, int height) = 0;
        virtual void SetupKeyCodeConversionMap() = 0;
        virtual void SetupMouseButtonCodeConversionMap() = 0;
        virtual void SetupGamepadButtonCodeConversionMap() = 0;

        std::function<void(std::shared_ptr<Event>)> m_eventCallback;
        std::shared_ptr<GraphicsContext> m_context;
        std::string m_title;
        std::unordered_map<int, int> m_keyCodeConversionMap; // SpecificKeyCode, FireflyKeyCode
        std::unordered_map<int, int> m_mouseButtonCodeConversionMap; // SpecificMouseButtonCode, FireflyMouseButtonCode
        std::unordered_map<int, int> m_gamepadButtonCodeConversionMap; // SpecificGamepadButtonCode, FireflyGamepadButtonCode
    };
}