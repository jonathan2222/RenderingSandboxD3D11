#pragma once

#include "Core/FrameStats.h"

namespace RS
{
	class EngineLoop
	{
	public:
		void Init();
		void Release();

		void Run();

	private:
		void FixedTick();
		void Tick(const FrameStats& frameStats);

		void DrawFrameStats(const FrameStats& frameStats);
	};
}