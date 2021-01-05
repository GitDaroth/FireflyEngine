#include "Core/EntryPoint.h"

#include "Core/Engine.h"

#ifdef FIREFLY_WINDOWS
	int main(int argc, char** argv)
	{
		Firefly::Engine engine;

		engine.Init();
		engine.Run();
		engine.Shutdown();

		return 0;
	}
#endif