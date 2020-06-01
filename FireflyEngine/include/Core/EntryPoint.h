#pragma once

namespace Firefly
{
	extern Application* InstantiateApplication();
}

#ifdef FIREFLY_WINDOWS
	int main(int argc, char** argv)
	{
		Firefly::Logger::Init();

		auto app = Firefly::InstantiateApplication();
		app->Run();
		delete app;

		return 0;
	}
#endif