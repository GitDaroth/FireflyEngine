#pragma once

namespace Firefly
{
	class Window;

	class GraphicsContext
	{
	public:
		virtual void Init(std::shared_ptr<Window> window) = 0;
		virtual void Destroy() = 0;
	};
}