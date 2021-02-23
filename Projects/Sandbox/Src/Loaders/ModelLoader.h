#pragma once

#include "Utils/Maths.h"

#include "Core/ResourceManager.h"

namespace RS
{
	class ModelLoader
	{
	public:
		RS_DEFAULT_ABSTRACT_CLASS(ModelLoader);

		static bool Load(const std::string& filePath, ModelResource*& outModel);
	};
}