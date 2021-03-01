#pragma once

#include "Utils/Maths.h"

#include "Core/ResourceManager.h"
#include "Core/ResourceManagerDefines.h"

#include <assimp/material.h>

struct aiMesh;
struct aiNode;
struct aiScene;
namespace RS
{
	class ModelLoader
	{
	public:
		RS_DEFAULT_ABSTRACT_CLASS(ModelLoader);

		static bool Load(const std::string& filePath, ModelResource*& outModel, ModelLoadDesc::LoaderFlags flags);

		static bool LoadWithAssimp(const std::string& filePath, ModelResource* outModel, ModelLoadDesc::LoaderFlags flags);

	private:
		static bool RecursiveLoadMeshes(const aiScene*& pScene, aiNode* pNode, ModelResource* pParent, glm::mat4 accTransform, ModelLoadDesc::LoaderFlags flags, const std::string& modelPath);
		static void FillMesh(const aiScene*& pScene, MeshObject& outMesh, aiMesh*& pMesh, ModelLoadDesc::LoaderFlags flags, const std::string& modelPath);
		static void LoadMaterial(const aiScene*& pScene, MeshObject& outMesh, aiMesh*& pMesh, ModelLoadDesc::LoaderFlags flags, const std::string& modelPath);
		static ResourceID LoadTextureResource(aiTextureType type, const aiScene*& pScene, aiMaterial* pMaterial, ResourceID defaultTextureID, const std::string& modelPath);
	};
}