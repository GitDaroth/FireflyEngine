#pragma once

#include "Game.h"

namespace Firefly
{
	extern Game* InstantiateGame();
}

#ifdef FIREFLY_WINDOWS
	int main(int argc, char** argv)
	{
		auto game = Firefly::InstantiateGame();
		game->Run();
		delete game;

		return 0;
	}
#endif