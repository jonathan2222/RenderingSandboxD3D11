#include "PreCompiled.h"
#include "ResourceInspector.h"

#include <unordered_set>

using namespace RS;

void ResourceInspector::Init(std::shared_ptr<ResourceManager>& resourceManager)
{
	s_ResourceManager = resourceManager;
}

void ResourceInspector::Release()
{
}

void ResourceInspector::Draw()
{
	ImGuiRenderer::Draw([&]()
	{
		static bool s_ResourceManagerWindow = true;
		if (ImGui::Begin("Resource Manager", &s_ResourceManagerWindow))
		{
			const ResourceManager::Stats& stats = ResourceManager::Get()->GetStats();
			ImGui::Indent();
			if (ImGui::TreeNode("Types"))
			{
				for (auto& [type, refCount] : *stats.pTypeResourcesRefCount)
				{
					std::string typeStr = Resource::TypeToString(type);
					ImGui::Text("%s: %d", typeStr.c_str(), refCount);
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Resources"))
			{
				std::unordered_set<ResourceID> idSet;
				for (auto& [keyStr, id] : *stats.pStringToResourceIDMap)
				{
					idSet.insert(id);
					uint32 refCount = (*stats.pResourcesRefCount)[id];
					std::string idStr = std::to_string(id);
					ImGui::Text("[%s] %s: %d", idStr.c_str(), keyStr.c_str(), refCount);
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
						ImGui::Text("[%s] %s: %d", idStr.c_str(), keyStr.c_str(), refCount);
						ImGui::PopTextWrapPos();
						ImGui::EndTooltip();
					}
				}

				// Add the resources that do not have a name associated to it.
				for (ResourceID id : stats.ResourceIDs)
				{
					if (idSet.find(id) == idSet.end())
					{
						idSet.insert(id);
						std::string keyStr = "_";
						uint32 refCount = (*stats.pResourcesRefCount)[id];
						std::string idStr = std::to_string(id);
						ImGui::Text("[%s] %s: %d", idStr.c_str(), keyStr.c_str(), refCount);
						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
							ImGui::Text("[%s] %s: %d", idStr.c_str(), keyStr.c_str(), refCount);
							ImGui::PopTextWrapPos();
							ImGui::EndTooltip();
						}
					}
				}

				ImGui::TreePop();
			}
			ImGui::Unindent();

			ImGui::End();
		}
	});
}
