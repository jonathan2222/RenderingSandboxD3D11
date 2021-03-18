#include "PreCompiled.h"
#include "Resources.h"

using namespace RS;

std::string Resource::TypeToString(Type type)
{
	std::string res;
	switch (type)
	{
	case Type::IMAGE:
		res = "IMAGE";
		break;
	case Type::TEXTURE:
		res = "TEXTURE";
		break;
	case Type::CUBE_MAP:
		res = "CUBE_MAP";
		break;
	case Type::SAMPLER:
		res = "SAMPLER";
		break;
	case Type::MODEL:
		res = "MODEL";
		break;
	case Type::MATERIAL:
		res = "MATERIAL";
		break;
	default:
		LOG_WARNING("Resource type is not supported!");
		res = "NULL";
		break;
	}
    return res;
}
