#pragma once

namespace RS
{
	using RenderFlags = uint32;
	enum RenderFlag : RenderFlags
	{
		RENDER_FLAG_NO_TEXTURES					= FLAG(0),
		RENDER_FLAG_ALBEDO_TEXTURE				= FLAG(1),
		RENDER_FLAG_NORMAL_TEXTURE				= FLAG(2),
		RENDER_FLAG_AO_TEXTURE					= FLAG(3),
		RENDER_FLAG_METALLIC_TEXTURE			= FLAG(4),
		RENDER_FLAG_ROUGHNESS_TEXTURE			= FLAG(5)
	};
}