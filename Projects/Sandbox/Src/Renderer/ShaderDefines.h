#pragma once

namespace RS
{
	using ShaderTypeFlags = uint32;
	enum ShaderTypeFlag : ShaderTypeFlags
	{
		NONE		= 0,
		VERTEX		= FLAG(0),
		FRAGMENT	= FLAG(1),
		GEOMETRY	= FLAG(2),
		COMPUTE		= FLAG(3),
		TESS_HULL	= FLAG(4),
		TESS_DOMAIN = FLAG(5)
	};

	static std::string ShaderTypeToStringArr[] =
	{
		NameToStr(ShaderTypeFlag::VERTEX),
		NameToStr(ShaderTypeFlag::FRAGMENT),
		NameToStr(ShaderTypeFlag::GEOMETRY),
		NameToStr(ShaderTypeFlag::COMPUTE),
		NameToStr(ShaderTypeFlag::TESS_HULL),
		NameToStr(ShaderTypeFlag::TESS_DOMAIN),
	};

	static std::string ShaderTypeToTarget(RS::ShaderTypeFlag type)
	{
		switch (type)
		{
		case RS::ShaderTypeFlag::VERTEX:
			return "vs_5_0";
			break;
		case RS::ShaderTypeFlag::FRAGMENT:
			return "ps_5_0";
			break;
		case RS::ShaderTypeFlag::GEOMETRY:
			return "gs_5_0";
			break;
		case RS::ShaderTypeFlag::COMPUTE:
			return "cs_5_0";
			break;
		case RS::ShaderTypeFlag::TESS_HULL:
			return "hs_5_0";
			break;
		case RS::ShaderTypeFlag::TESS_DOMAIN:
			return "ds_5_0";
			break;
		default:
			{
				LOG_WARNING("Shader [{}] type not supported!", (uint32)type);
				return "none";
			}
			break;
		}
	}
}

#define RS_SHADER_VERTEX_AND_FRAGMENT RS::ShaderTypeFlag::VERTEX | RS::ShaderTypeFlag::FRAGMENT