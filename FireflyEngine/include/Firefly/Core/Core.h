#pragma once

#include <EngineConfig.h>
#include "Core/Logger.h"

#ifdef DEBUG
#define FIREFLY_ASSERT(condition, ...) {if(!(condition)) {Firefly::Logger::Critical("Assert", __VA_ARGS__); __debugbreak();}}
#else
#define FIREFLY_ASSERT(condition, ...)
#endif