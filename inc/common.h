#pragma once

#if _MSC_VER
#define DEBUGBREAK __debugbreak()
#else
#define DEBUGBREAK __builtin_debugtrap()
#endif
