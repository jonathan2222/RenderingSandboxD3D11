#pragma once

#include "Utils/Maths.h"

#include "Core/ResourceManager.h"
#include "Core/ResourceManagerDefines.h"

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
		static bool RecursiveLoadMeshes(const aiScene*& pScene, aiNode* pNode, ModelResource* pParent, glm::mat4 accTransform, ModelLoadDesc::LoaderFlags flags);
		static void FillMesh(const aiScene*& pScene, MeshResource& outMesh, aiMesh*& pMesh, ModelLoadDesc::LoaderFlags flags);
	};
}