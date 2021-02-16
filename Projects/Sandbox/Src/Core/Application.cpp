#include "PreCompiled.h"
#include "Application.h"

#include "Renderer/ImGuiRenderer.h"

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
	m_Scenes.push_back(scene);
}

void Application::SelectScene(const std::string& name)
{
	auto it = m_NameToSceneMap.find(name);
	if (it == m_NameToSceneMap.end())
	{
		uint32_t index = 0;
		bool wasFound = false;
		for (; index < m_Scenes.size(); index++)
		{
			if (m_Scenes[index]->GetName().compare(name) == 0)
			{
				wasFound = true;
			}
		}

		if (!wasFound)
		{
			LOG_WARNING("Scene '{}' could not be selected!", name);
			return;
		}

		m_NameToSceneMap[name] = index;
		m_CurrentScene = index;
	}
	else
	{
		m_CurrentScene = it->second;
	}

	m_Scenes[m_CurrentScene]->Selected();
}

void Application::Run()
{
	for (Scene*& scene : m_Scenes)
		scene->Start();

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
	
	ImGuiRenderer::Draw([&]()
		{
			if (m_ShowImGuiDemoWindow)
				ImGui::ShowDemoWindow(&m_ShowImGuiDemoWindow);
		});
}
