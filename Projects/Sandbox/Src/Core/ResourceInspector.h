#pragma once

#include "Renderer/ImGuiRenderer.h"
#include "Resources/Resources.h"

namespace RS
{
	class ResourceManager;
	class ResourceInspector
	{
	public:
		static void Init(ResourceManager* pResourceManager);
		static void Release();

		static void Draw();

	private:
		static void DrawTextureResource(ResourceID id, TextureResource* pTexture);
		static void DrawImageResource(ResourceID id, ImageResource* pImage);

		static std::string GetKeyStringFromID(ResourceID id);

	private:
		inline static ResourceManager* s_ResourceManager = nullptr;
	};
}