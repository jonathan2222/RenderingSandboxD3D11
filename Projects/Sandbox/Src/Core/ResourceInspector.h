#pragma once

#include "Core/ResourceManager.h"
#include "Renderer/ImGuiRenderer.h"

namespace RS
{
	class ResourceInspector
	{
	public:
		static void Init(std::shared_ptr<ResourceManager>& resourceManager);
		static void Release();

		static void Draw();

	private:
		inline static std::shared_ptr<ResourceManager> s_ResourceManager = nullptr;
	};
}