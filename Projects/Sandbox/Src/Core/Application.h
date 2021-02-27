#pragma once

#include "Core/Scene.h"
#include "Core/EngineLoop.h"

namespace RS
{
	class Application
	{
	public:
		void Init();

		void Release();

		void AddScene(Scene* scene);
		void SelectScene(uint32 index);

		void Run();

	private:
		void FixedTick();
		void Tick(float dt);

		void DrawSceneSelectionPanel();

	private:
		std::vector<Scene*>	m_Scenes;
		std::unordered_map<std::string, uint32_t> m_NameToSceneMap;
		int32_t m_CurrentScene = -1;

		EngineLoop m_EngineLoop;
	};
}