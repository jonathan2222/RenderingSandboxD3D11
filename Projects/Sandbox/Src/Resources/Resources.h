#pragma once

#include "Renderer/RenderAPI.h"
#include "Resources/RefObject.h"

#include "Utils/Maths.h"
#include "Structures/AABB.h"

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
			glm::vec3 Position	= glm::vec3(0.f);
			glm::vec3 Normal	= glm::vec3(0.f);
			glm::vec3 Tangent	= glm::vec3(0.f);
			glm::vec3 Bitangent = glm::vec3(0.f);
			glm::vec2 UV		= glm::vec2(0.f);
		};

		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;
		AABB				BoundingBox;
	};

	struct ModelResource : public Resource
	{
		std::string					Name		= "";
		ModelResource*				pParent		= nullptr;
		glm::mat4					Transform	= glm::mat4(1.f);
		AABB						BoundingBox;
		std::vector<MeshResource>	Meshes;
		std::vector<ModelResource>	Children;
	};
}