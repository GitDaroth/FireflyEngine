#pragma once

namespace Firefly
{
	extern Game* InstantiateGame();
}

#ifdef FIREFLY_WINDOWS
	int main(int argc, char** argv)
	{
		Firefly::Logger::Init();

		auto game = Firefly::InstantiateGame();
		game->Run();
		delete game;

		return 0;
	}
#endif