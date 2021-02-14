#pragma once

#include "Core/Display.h"
#include <imgui.h>
#include <mutex>
#include <functional>

namespace RS
{
	class ImGuiRenderer
	{
	public:
		static void Init(Display* pDisplay);
		static void Release();

		static void Draw(std::function<void(void)> callback);

		static void Render();

	private:
		static void BeginFrame();
		static void EndFrame();

	private:
		static std::vector<std::function<void(void)>> s_DrawCalls;
		static std::mutex s_Mutex;
	};
}