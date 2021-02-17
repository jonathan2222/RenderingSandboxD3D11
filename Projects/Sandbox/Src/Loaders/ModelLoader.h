#pragma once

#include "Utils/Maths.h"

namespace RS
{
	struct Model
	{
		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec2 UV;
		};

		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;
	};

	class ModelLoader
	{
	public:
		RS_DEFAULT_ABSTRACT_CLASS(ModelLoader);

		static Model* Load(const std::string& filePath);
	};
}