#pragma once

#include "Renderer/ImGuiRenderer.h"

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
		inline static ResourceManager* s_ResourceManager = nullptr;
	};
}