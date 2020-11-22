#pragma once

#include "Core/Application.h"

namespace Firefly
{
	extern Application* InstantiateApplication();
}

#ifdef FIREFLY_WINDOWS
	int main(int argc, char** argv);
#endif