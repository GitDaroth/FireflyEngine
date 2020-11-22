#include "SandboxApp.h"

Firefly::Application* Firefly::InstantiateApplication()
{
	return new SandboxApp();
}