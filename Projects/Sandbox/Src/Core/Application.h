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
		void SelectScene(const std::string& name);

		void Run();

	private:
		void FixedTick();
		void Tick(float dt);

	private:
		bool m_ShowImGuiDemoWindow = false;
		std::vector<Scene*>	m_Scenes;
		std::unordered_map<std::string, uint32_t> m_NameToSceneMap;
		int32_t m_CurrentScene = -1;

		EngineLoop m_EngineLoop;
	};
}