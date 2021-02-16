#pragma once

#include "Core/FrameStats.h"

#include <functional>

namespace RS
{
	class EngineLoop
	{
	public:
		void Init(std::function<void(void)> fixedTickCallback, std::function<void(float)> tickCallback);
		void Release();

		void Run();

	private:
		void FixedTick();
		void Tick(const FrameStats& frameStats);

		void DrawFrameStats(const FrameStats& frameStats);

	private:
		std::function<void(void)>	m_FixedTickCallback;
		std::function<void(float)>	m_TickCallback;
	};
}