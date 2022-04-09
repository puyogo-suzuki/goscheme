#pragma once

#if _MSC_VER
#define DEBUGBREAK __debugbreak()
#elif __GNUC__
#define DEBUGBREAK __builtin_trap()
#elif __clang__	
#define DEBUGBREAK __builtin_debugtrap()
#endif
