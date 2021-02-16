#include "PreCompiled.h"
#include "Scene.h"

using namespace RS;

Scene::Scene(const std::string& name) : m_Name(name)
{
}

void Scene::SetName(const std::string& name)
{
	m_Name = name;
}

std::string Scene::GetName()
{
	if (m_Name.empty())
		m_Name = "Scene #" + std::to_string(GenerateIndex());
	return m_Name;
}

uint32_t Scene::GenerateIndex() const
{
	static uint32_t s_Generator = 0;
	return s_Generator++;
}
