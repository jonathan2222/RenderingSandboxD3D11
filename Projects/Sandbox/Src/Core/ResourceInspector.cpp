#include "PreCompiled.h"
#include "ResourceInspector.h"

#include <unordered_set>

#include "Core/ResourceManager.h"
#include "Renderer/RenderUtils.h"

using namespace RS;

void ResourceInspector::Init(ResourceManager* pResourceManager)
{
	s_ResourceManager = pResourceManager;
}

void ResourceInspector::Release()
{
}

void ResourceInspector::Draw()
{
	// Process resources
	static std::unordered_map<Resource::Type, std::vector<std::pair<uint32, Resource*>>> s_TypeToResourcesMap;
	s_TypeToResourcesMap.clear();
	for (auto& [id, pResource] : s_ResourceManager->m_IDToResourceMap)
	{
		auto& resources = s_TypeToResourcesMap[pResource->type];
		resources.push_back(std::make_pair(id, pResource));
	}

	static bool s_ResourceInspectorWindow = true;
	ImGuiRenderer::Draw([&]()
	{
		if (ImGui::Begin("Resource Inspector", &s_ResourceInspectorWindow))
		{
			for (auto& [type, resources] : s_TypeToResourcesMap)
			{
				std::string typeStr = Resource::TypeToString(type);
				typeStr += " [" + std::to_string(resources.size()) + "]";
				if (ImGui::TreeNode(typeStr.c_str()))
				{
					for (uint32 index = 0; index < (uint32)resources.size(); index++)
					{
						ResourceID id = resources[index].first;
						ResourceID key = resources[index].second->key;
						std::string resourceKeyString = "[" + std::to_string(key) + "] " + GetKeyStringFromID(id);
						if (ImGui::TreeNode((void*)(intptr_t)index, resourceKeyString.c_str()))
						{
							if (type == Resource::Type::TEXTURE)
							{
								TextureResource* pResource = dynamic_cast<TextureResource*>(resources[index].second);
								DrawTextureResource(id, pResource);
							}

							ImGui::TreePop();
						}

						// Make a tooltip for when the tree node is hovered. In this tooltop display the key.
						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
							ImGui::Text("%s", resourceKeyString.c_str());
							ImGui::PopTextWrapPos();
							ImGui::EndTooltip();
						}
					}
					ImGui::TreePop();
				}
			}
		}
		ImGui::End();

		/*
		if (ImGui::Begin("Resource Inspector", &s_ResourceManagerWindow))
		{
			const ResourceManager::Stats& stats = s_ResourceManager->GetStats();
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

		}
		ImGui::End();
		*/
	});
}

void ResourceInspector::DrawTextureResource(ResourceID id, TextureResource* pTexture)
{
	ImageResource* pImageResource = s_ResourceManager->GetResource<ImageResource>(pTexture->ImageHandler);
	DrawImageResource(pTexture->ImageHandler, pImageResource);
	
}

void ResourceInspector::DrawImageResource(ResourceID id, ImageResource* pImage)
{
	ImGui::Text("Width: %d", pImage->Width);
	ImGui::Text("Height: %d", pImage->Height);
	std::string formatStr = RenderUtils::FormatToString(pImage->Format);
	ImGui::Text("Format: %s", formatStr.c_str());
}

std::string ResourceInspector::GetKeyStringFromID(ResourceID id)
{
	for (auto& [key, resourceID] : s_ResourceManager->m_StringToResourceIDMap)
	{
		if (resourceID == id)
			return key;
	}
	return "";
}
