#include "PreCompiled.h"
#include "Application.h"

#include "Renderer/ImGuiRenderer.h"
#include "Renderer/DebugRenderer.h"

using namespace RS;

void Application::Init()
{
	m_EngineLoop.Init(
		std::bind_front(&Application::FixedTick, this),
		std::bind_front(&Application::Tick, this));
}

void Application::Release()
{
	for (Scene*& scene : m_Scenes)
	{
		scene->End();
		delete scene;
	}

	m_EngineLoop.Release();
}

void Application::AddScene(Scene* scene)
{
	m_CurrentScene = (int32_t)m_Scenes.size();
	m_NameToSceneMap[scene->GetName()] = m_CurrentScene;
	m_Scenes.push_back(scene);
}

void Application::SelectScene(uint32 index)
{
	if (index >= 0 && index < m_Scenes.size())
	{
		// Unselect the previous scene.
		if (m_CurrentScene >= 0)
		{
			Scene* pPreScene = m_Scenes[m_CurrentScene];
			pPreScene->Unselected();
		}

		// Clear all debug primitives.
		DebugRenderer::Get()->Clear();

		// Select the new scene.
		m_CurrentScene = index;
		m_Scenes[m_CurrentScene]->Selected();
	}
	else
	{
		LOG_WARNING("Index out of bounds! Canceling selection.");
	}
}

void Application::Run()
{
	for (Scene*& scene : m_Scenes)
		scene->Start();

	if(m_CurrentScene >= 0)
		m_Scenes[m_CurrentScene]->Selected();

	m_EngineLoop.Run();
}

void Application::FixedTick()
{
	if (m_CurrentScene >= 0)
		m_Scenes[m_CurrentScene]->FixedTick();
}

void Application::Tick(float dt)
{
	if (m_CurrentScene >= 0)
		m_Scenes[m_CurrentScene]->Tick(dt);
	
	static bool m_ShowImGuiDemoWindow = true;
	ImGuiRenderer::Draw([&]()
	{
			if (m_ShowImGuiDemoWindow)
				ImGui::ShowDemoWindow(&m_ShowImGuiDemoWindow);
	});

	DrawSceneSelectionPanel();
}

void Application::DrawSceneSelectionPanel()
{
    ImGuiRenderer::Draw([&]()
	{
		static bool s_IsSceneSelectorActive = true;
        if (ImGui::Begin("Scene Selector", &s_IsSceneSelectorActive))
        {
			static int selection = m_CurrentScene;
			for (auto& [name, index] : m_NameToSceneMap)
			{
				ImGui::RadioButton(name.c_str(), &selection, index);
			}

			if (selection != m_CurrentScene)
				SelectScene(selection);
        }
        ImGui::End();
	});
}
