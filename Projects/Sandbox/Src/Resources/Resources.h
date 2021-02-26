#pragma once

#include "Renderer/RenderAPI.h"
#include "Resources/RefObject.h"

#include "Utils/Maths.h"

namespace RS
{
	struct Resource : public RefObject
	{
		virtual ~Resource() = default;

		enum class Type
		{
			IMAGE = 0,
			MODEL
		};

		static std::string TypeToString(Type type);

		Type type;
		std::string key;
	};

	struct ImageResource : public Resource
	{
		void*		Data	= nullptr;
		uint32		Width	= 0;
		uint32		Height	= 0;
		DXGI_FORMAT Format	= DXGI_FORMAT_UNKNOWN;
	};

	struct MeshResource : public Resource
	{
		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec3 Tangent;
			glm::vec3 Bitangent;
			glm::vec2 UV;
		};

		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;
	};

	struct ModelResource : public Resource
	{
		MeshResource				Mesh;
		std::vector<ModelResource>	Children;
		glm::mat4					Transform;
	};
}