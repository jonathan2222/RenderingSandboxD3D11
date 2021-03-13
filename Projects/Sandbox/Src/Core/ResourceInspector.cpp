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
		uint32 refCount = s_ResourceManager->m_ResourcesRefCount[id];
		resources.push_back(std::make_pair(refCount, pResource));
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
						ResourceID refCount = resources[index].first;
						ResourceID key = resources[index].second->key;
						std::string resourceKeyString = GetKeyStringFromID(key);
						
						std::string resourceKeyStringEnding = resourceKeyString;
						size_t lastSlashPos = resourceKeyString.find_last_of("\\/");
						if(lastSlashPos != std::string::npos)
							resourceKeyStringEnding = resourceKeyString.substr(lastSlashPos);

						resourceKeyStringEnding = "[" + std::to_string(key) + "] " + resourceKeyStringEnding;

						if (type == Resource::Type::TEXTURE)
						{
							TextureResource* pResource = dynamic_cast<TextureResource*>(resources[index].second);
							if (ImGui::TreeNode((void*)(intptr_t)index, resourceKeyStringEnding.c_str()))
							{
								ImGui::Text("Ref. count: %d", refCount);
								ImGui::Text("Key: %s", resourceKeyString.c_str());
								DrawTextureResource(pResource);
								ImGui::TreePop();
							}
						}
						else if (type == Resource::Type::IMAGE)
						{
							ImageResource* pResource = dynamic_cast<ImageResource*>(resources[index].second);
							if (ImGui::TreeNode((void*)(intptr_t)index, resourceKeyStringEnding.c_str()))
							{
								ImGui::Text("Ref. count: %d", refCount);
								ImGui::Text("Key: %s", resourceKeyString.c_str());
								DrawImageResource(pResource);
								ImGui::TreePop();
							}
						}
						else if (type == Resource::Type::MATERIAL)
						{
							MaterialResource* pResource = dynamic_cast<MaterialResource*>(resources[index].second);
							std::string name = "[" + std::to_string(key) + "] " + pResource->Name;
							if (ImGui::TreeNode((void*)(intptr_t)index, name.c_str()))
							{
								ImGui::Text("Ref. count: %d", refCount);
								ImGui::Text("Key: %s", resourceKeyString.c_str());
								DrawMaterialResource(pResource);
								ImGui::TreePop();
							}
						}
						else if (type == Resource::Type::MODEL)
						{
							ModelResource* pResource = dynamic_cast<ModelResource*>(resources[index].second);
							std::string name = "[" + std::to_string(key) + "] " + pResource->Name;
							if (ImGui::TreeNode((void*)(intptr_t)index, name.c_str()))
							{
								ImGui::Text("Ref. count: %d", refCount);
								ImGui::Text("Key: %s", resourceKeyString.c_str());
								DrawModelResource(pResource);
								ImGui::TreePop();
							}
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

void ResourceInspector::DrawTextureResource(TextureResource* pTexture)
{
	ImageResource* pImageResource = s_ResourceManager->GetResource<ImageResource>(pTexture->ImageHandler);
	DrawImageResource(pImageResource);
	
	DrawTexture(pTexture, 200, 200);
}

void ResourceInspector::DrawImageResource(ImageResource* pImage)
{
	ImGui::Text("Width: %d", pImage->Width);
	ImGui::Text("Height: %d", pImage->Height);
	std::string formatStr = RenderUtils::FormatToString(pImage->Format);
	ImGui::Text("Format: %s", formatStr.c_str());
}

void ResourceInspector::DrawMaterialResource(MaterialResource* pMaterial)
{
	uint32 index = 0;
	auto DrawMaterialTexture = [&](const std::string& name, ResourceID id)->void
	{
		if (ImGui::TreeNode((void*)(intptr_t)index, name.c_str()))
		{
			TextureResource* pTextureResource = s_ResourceManager->GetResource<TextureResource>(id);
			DrawTextureResource(pTextureResource);

			ImGui::TreePop();
		}
		index++;
	};
	
	ImGui::Text("Name: %s", pMaterial->Name.c_str());
	DrawMaterialTexture("Albedo Texture", pMaterial->AlbedoTextureHandler);
	DrawMaterialTexture("Normal Texture", pMaterial->NormalTextureHandler);
	DrawMaterialTexture("Ambient Occlusion Texture", pMaterial->AOTextureHandler);
	DrawMaterialTexture("Metallic Texture", pMaterial->MetallicTextureHandler);
	DrawMaterialTexture("Roughness Texture", pMaterial->RoughnessTextureHandler);
	DrawMaterialTexture("Combined Metallic-Roughness Texture", pMaterial->MetallicRoughnessTextureHandler);
}

void ResourceInspector::DrawModelResource(ModelResource* pModel)
{
	ImGui::Text("Name: %s", pModel->Name.c_str());
	if (ImGui::TreeNode("Model"))
	{
		DrawModelRecursive(*pModel);
		ImGui::TreePop();
	}
}

void ResourceInspector::DrawModelRecursive(ModelResource& model)
{
	ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Name: %s", model.Name.c_str());
	if (model.pParent)
		ImGui::Text("Parent: %s", model.pParent->Name.c_str());
	else
		ImGui::Text("Parent: No Parent");
	DrawImGuiAABB(0, model.BoundingBox);

	if (model.Meshes.empty() == false)
	{
		if (ImGui::TreeNode((void*)(intptr_t)1, "Meshes"))
		{
			for (uint32 i = 0; i < (uint32)model.Meshes.size(); i++)
			{
				if (ImGui::TreeNode((void*)(intptr_t)i, "Mesh #%d", i))
				{
					MeshObject& mesh = model.Meshes[i];
					ImGui::Text("Is in RAM: ");
					float on = mesh.Vertices.empty() ? 0.f : 1.f;
					ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f - on, on, 0.f, 1.f), "%s", on > 0.5f ? "True" : "False");

					ImGui::Text("Is in GPU: ");
					on = mesh.pVertexBuffer ? 1.f : 0.f;
					ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f - on, on, 0.f, 1.f), "%s", on > 0.5f ? "True" : "False");

					ImGui::Text("Num Vertices: %d", mesh.NumVertices);
					ImGui::Text("Num Indices: %d", mesh.NumIndices);
					DrawImGuiAABB(0, mesh.BoundingBox);

					if (ImGui::TreeNode((void*)(intptr_t)1, "Material"))
					{
						MaterialResource* pMaterialResource = s_ResourceManager->GetResource<MaterialResource>(mesh.MaterialHandler);
						DrawMaterialResource(pMaterialResource);
						ImGui::TreePop();
					}

					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}

	if (model.Children.empty() == false)
	{
		uint32 index = model.Meshes.empty() ? 1 : 2;
		if (ImGui::TreeNode((void*)(intptr_t)index, "Children"))
		{
			for (int i = 0; i < (int)model.Children.size(); i++)
			{
				if (ImGui::TreeNode((void*)(intptr_t)i, model.Children[i].Name.c_str()))
				{
					DrawModelRecursive(model.Children[i]);
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}
}

void ResourceInspector::DrawImGuiAABB(int index, const AABB& aabb)
{
	if (ImGui::TreeNode((void*)(intptr_t)index, "Bounding Box"))
	{
		bool isSame = glm::all(glm::lessThan(glm::abs(aabb.min - aabb.max), glm::vec3(FLT_EPSILON)));
		ImVec4 color(0.f, 1.f, 0.f, 1.f);
		if (isSame) color = ImVec4(1.f, 0.f, 0.f, 1.f);
		ImGui::TextColored(color, "Size: (%f, %f, %f)", aabb.max.x - aabb.min.x, aabb.max.y - aabb.min.y, aabb.max.z - aabb.min.z);
		ImGui::TextColored(color, "Min: (%f, %f, %f)", aabb.min.x, aabb.min.y, aabb.min.z);
		ImGui::TextColored(color, "Max: (%f, %f, %f)", aabb.max.x, aabb.max.y, aabb.max.z);
		ImGui::TreePop();
	}
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

void ResourceInspector::DrawTexture(TextureResource* pTexture, uint32 width, uint32 height)
{
	ImVec2 size = ImVec2((float)width, (float)height);
	ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
	ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
	ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
	ImGui::Image((ImTextureID)pTexture->pTextureSRV, size, uv_min, uv_max, tint_col, border_col);

	// Display a larger image when the mouse is hovering the image.
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		float zoom = 4.0f;
		ImVec2 uv0 = ImVec2(0.f, 0.f);
		ImVec2 uv1 = ImVec2(1.f, 1.f);
		ImGui::Image((ImTextureID)pTexture->pTextureSRV, ImVec2(width * zoom, height * zoom), uv0, uv1, tint_col, border_col);
		ImGui::EndTooltip();
	}
}
