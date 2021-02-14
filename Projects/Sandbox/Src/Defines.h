#pragma once

#include <cassert>
#include <vector>

#define RS_CONFIG_DEVELOPMENT defined(RS_CONFIG_DEBUG) || defined(RS_CONFIG_RELEASE)

/*#ifdef RS_CONFIG_DEVELOPMENT
	#define RS_ASSERT(exp, ...) {if(!(exp)){LOG_CRITICAL(__VA_ARGS__);} assert(exp); }
#else
	#define RS_ASSERT(exp, ...) {assert(exp);}
#endif*/

#define RS_CONFIG_FILE_PATH "../../Assets/Config/EngineConfig.json"
#define RS_SHADER_PATH "../../Assets/Shaders/"
#define RS_TEXTURE_PATH "../../Assets/Textures/"

#define RS_UNREFERENCED_VARIABLE(v) (void)v
#define FLAG(x) (1 << x)

#include "ClassTemplates.h"

typedef int8_t		int8;
typedef int16_t		int16;
typedef int32_t		int32;
typedef int64_t		int64;

typedef uint8_t		uint8;
typedef uint16_t	uint16;
typedef uint32_t	uint32;
typedef uint64_t	uint64;