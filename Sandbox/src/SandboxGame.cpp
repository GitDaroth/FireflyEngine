#include <FireflyEngine.h>

class SandboxGame : public Firefly::Game
{
public:
	SandboxGame()
	{
	}

	~SandboxGame()
	{
	}
};

Firefly::Game* Firefly::InstantiateGame()
{
	Firefly::Logger::Critical("Sandbox", "Hello from Sandbox!");
	return new SandboxGame();
}