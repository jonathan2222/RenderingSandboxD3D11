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

		static bool WantKeyInput();

		static void Resize();

		static float GetGuiScale();

	private:
		static void BeginFrame();
		static void EndFrame();
		static void InternalResize();
		static void ReScale(uint32 width, uint32 height);

	private:
		static std::vector<std::function<void(void)>> s_DrawCalls;
		static std::mutex s_Mutex;
		inline static bool s_ShouldRescale = false;
		inline static float s_Scale = 1.f;
	};
}