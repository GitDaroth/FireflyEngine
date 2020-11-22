#include "Core/EntryPoint.h"

#include "Core/Logger.h"

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