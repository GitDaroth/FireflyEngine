#pragma once

namespace Firefly
{
	class GraphicsContext
	{
	public:
		GraphicsContext() {};
		virtual ~GraphicsContext() {};

		virtual void Init(void* window) = 0;
		virtual void SwapBuffers() = 0;
	};
}