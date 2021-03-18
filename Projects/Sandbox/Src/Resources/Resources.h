#pragma once

#include "Renderer/RenderAPI.h"
#include "Resources/RefObject.h"

#include "Utils/Maths.h"
#include "Structures/AABB.h"

#include "Renderer/RenderAPI.h"

namespace RS
{
	using ResourceID = uint32;

	struct Resource : public RefObject
	{
		virtual ~Resource() = default;

		enum class Type
		{
			IMAGE = 0,
			TEXTURE,
			CUBE_MAP,
			SAMPLER,
			MODEL,
			MATERIAL
		};

		static std::string TypeToString(Type type);

		Type		type;
		ResourceID	key = 0;
	};

	struct ImageResource : public Resource
	{
		std::vector<uint8>	Data;
		uint32				Width	= 0;
		uint32				Height	= 0;
		DXGI_FORMAT			Format	= DXGI_FORMAT_UNKNOWN;
	};

	struct TextureResource : public Resource
	{
		ResourceID					ImageHandler	= 0;
		ID3D11Texture2D*			pTexture		= nullptr;
		ID3D11ShaderResourceView*	pTextureSRV		= nullptr;
	};

	struct CubeMapResource : public Resource
	{
		ResourceID					ImageHandlers[6]	= { 0 };	// Contains images of a cube in this order [x, -x, y, -y, z, -z]
		ID3D11Texture2D*			pTexture			= nullptr;
		ID3D11ShaderResourceView*	pTextureSRV			= nullptr;
	};

	struct SamplerResource : public Resource
	{
		ID3D11SamplerState* pSampler = nullptr;
	};

	/*
	* Used as a constant buffer when rendering with a material.
	*/
	struct MaterialBuffer
	{
		glm::vec4 Info = glm::vec4(0.f); // x: UseCombined, y: debug draw index, z: not used, w: not used
		/*
		Debug draw index:
			0: Normal rendering
			1: Only albedo
			2: Only normal
			3: Only ao
			4: Only metallic
			5: Only roughness
			6: Only combined metallic-roughness (If avaliable)
		*/
	};

	struct MaterialResource : public Resource
	{
		ResourceID		AlbedoTextureHandler			= 0;
		ResourceID		NormalTextureHandler			= 0;
		ResourceID		AOTextureHandler				= 0;
		ResourceID		MetallicTextureHandler			= 0;
		ResourceID		RoughnessTextureHandler			= 0;
		ResourceID		MetallicRoughnessTextureHandler = 0;
		std::string		Name							= "";
		MaterialBuffer	InfoBuffer						= {};
		ID3D11Buffer*	pConstantBuffer					= nullptr;
	};

	struct MeshObject
	{
		struct MeshData
		{
			glm::mat4 world = glm::mat4(1.f);
		};

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
		uint32				NumIndices		= 0;
		uint32				NumVertices		= 0;
		AABB				BoundingBox;

		ID3D11Buffer*		pVertexBuffer	= nullptr;
		ID3D11Buffer*		pIndexBuffer	= nullptr;
		ID3D11Buffer*		pMeshBuffer		= nullptr;

		ResourceID			MaterialHandler = 0;
	};

	struct ModelResource : public Resource
	{
		std::string					Name		= "";
		ModelResource*				pParent		= nullptr;
		glm::mat4					Transform	= glm::mat4(1.f);
		AABB						BoundingBox;
		std::vector<MeshObject>		Meshes;
		std::vector<ModelResource>	Children;
	};
}