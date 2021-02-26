#pragma once

#include "Utils/Maths.h"

#include "Core/ResourceManager.h"

struct aiScene;
namespace RS
{
	class ModelLoader
	{
	public:
		RS_DEFAULT_ABSTRACT_CLASS(ModelLoader);

		static bool Load(const std::string& filePath, ModelResource*& outModel);

		static bool LoadWithAssimp(const std::string& filePath, ModelResource*& outModel);

	private:
		static bool RecursiveLoad(const aiScene*& pScene, ModelResource*& outModel);
	};
}