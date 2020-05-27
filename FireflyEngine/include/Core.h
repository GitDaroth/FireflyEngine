#pragma once

#ifdef FIREFLY_WINDOWS
	#ifdef FIREFLY_ENGINE
		#define FIREFLY_API __declspec(dllexport)
	#else
		#define FIREFLY_API __declspec(dllimport)
	#endif
#endif

#ifdef DEBUG_MODE
	#define FIREFLY_ASSERT(condition, ...) {if(!condition) {Firefly::Logger::Critical("Assert", __VA_ARGS__); __debugbreak();}}
#else
	#define FIREFLY_ASSERT(condition, ...)
#endif