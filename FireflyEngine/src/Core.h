#pragma once

#ifdef FIREFLY_WINDOWS
	#ifdef FIREFLY_ENGINE
		#define FIREFLY_API __declspec(dllexport)
	#else
		#define FIREFLY_API __declspec(dllimport)
	#endif
#endif