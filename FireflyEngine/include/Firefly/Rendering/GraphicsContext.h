#pragma once

#include <memory>

namespace Firefly
{
	class Window;

	class GraphicsContext
	{
	public:
		void Init(std::shared_ptr<Window> window);
		virtual void Destroy() = 0;

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

	protected:
		virtual void OnInit(std::shared_ptr<Window> window) = 0;

		std::shared_ptr<Window> m_window;
	};
}