#pragma once

#include "Renderer/RenderAPI.h"
#include "Resources/RefObject.h"

namespace RS
{
	struct Resource : public RefObject
	{
		virtual ~Resource() = default;

		enum class Type
		{
			TEXTURE
		};

		static std::string TypeToString(Type type);

		Type type;
		std::string key;
	};

	struct TextureResource : public Resource
	{
		void*		Data	= nullptr;
		uint32		Width	= 0;
		uint32		Height	= 0;
		DXGI_FORMAT Format	= DXGI_FORMAT_UNKNOWN;
	};
}