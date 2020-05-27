#pragma once

namespace Firefly
{
	extern Game* InstantiateGame();
}

#ifdef FIREFLY_WINDOWS
	int main(int argc, char** argv)
	{
		Firefly::Logger::Init();
		std::string a = "FireflyEngine";
		Firefly::Logger::Trace("FireflyEngine", "Hello from {0}!", a);
		Firefly::Logger::Debug("FireflyEngine", "Hello from {0}!", a);
		Firefly::Logger::Info("FireflyEngine", "Hello from {0}!", a);
		Firefly::Logger::Warn("FireflyEngine", "Hello from {0}!", a);
		Firefly::Logger::Error("FireflyEngine", "Hello from {0}!", a);

		auto game = Firefly::InstantiateGame();
		game->Run();
		delete game;

		return 0;
	}
#endif