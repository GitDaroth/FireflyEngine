#include "Window.h"
#include <GLFW/glfw3.h>

namespace Firefly
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const std::string& title = "Firefly Engine", int width = 1280, int height = 720);
		virtual ~WindowsWindow();

		virtual void OnUpdate() override;
		virtual void OnSetTitle(const std::string& title) override;
		virtual void OnSetSize(int width, int height) override;
		virtual void OnEnableVSync(bool enabled) override;

	private:
		GLFWwindow* m_window;
		static bool s_isWindowInitialized;
		static int s_windowCount;
	};
}