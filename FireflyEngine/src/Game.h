#pragma once

#include "Core.h"

namespace Firefly
{
	class FIREFLY_API Game
	{
	public:
		Game();
		virtual ~Game();

		void Run();
	};
}