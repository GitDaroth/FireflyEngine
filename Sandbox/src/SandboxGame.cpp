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
	return new SandboxGame();
}