#include <FireflyEngine.h>

class SandboxApp : public Firefly::Application
{
public:
	SandboxApp()
	{
	}

	~SandboxApp()
	{
	}
};

Firefly::Application* Firefly::InstantiateApplication()
{
	return new SandboxApp();
}