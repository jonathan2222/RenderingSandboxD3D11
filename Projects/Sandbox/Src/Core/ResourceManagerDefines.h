#pragma once

namespace RS
{
	struct ImageLoadDesc
	{
		enum class Channels : uint32
		{
			DEFAULT = FLAG(0),
			R = FLAG(1),
			RG = FLAG(2),
			RGB = FLAG(3),
			RGBA = FLAG(4)
		};

		struct FileData
		{
			std::string		Path				= "";
			bool			UseDefaultFolder	= true;
		} File;

		struct MemoryData
		{
			const uint8*	pData			= nullptr;
			bool			IsCompressed	= false;
			uint32			Size			= 0;		// This is only used for compressed data.
			uint32			Width			= 0;		// This is only used for uncompressed data.
			uint32			Height			= 0;		// This is only used for uncompressed data.
		} Memory;

		std::string	Name				= ""; // Used as a key, this should be unique!
		Channels	NumChannels			= Channels::DEFAULT;
		bool		IsFromFile			= true;
	};

	struct SamplerLoadDesc
	{
		D3D11_FILTER				Filter			= D3D11_FILTER_ANISOTROPIC;
		D3D11_TEXTURE_ADDRESS_MODE	AddressU		= D3D11_TEXTURE_ADDRESS_WRAP;
		D3D11_TEXTURE_ADDRESS_MODE	AddressV		= D3D11_TEXTURE_ADDRESS_WRAP;
		D3D11_TEXTURE_ADDRESS_MODE	AddressW		= D3D11_TEXTURE_ADDRESS_WRAP;
		FLOAT						MipLODBias		= 0.f;
		UINT						MaxAnisotropy	= 16;
		D3D11_COMPARISON_FUNC		ComparisonFunc	= D3D11_COMPARISON_GREATER_EQUAL;
		FLOAT						MinLOD			= 0;
		FLOAT						MaxLOD			= D3D11_FLOAT32_MAX;
		FLOAT						BorderColor[4]	= {0.f, 0.f, 0.f, 0.f};
	};

	struct TextureLoadDesc
	{
		ImageLoadDesc ImageDesc;
		bool GenerateMipmaps	= false;
		bool UseAsRTV			= false;
	};

	struct CubeMapLoadDesc
	{
		ImageLoadDesc ImageDescs[6]; // [x, -x, y, -y, z, -z]
		bool GenerateMipmaps =  false;

		bool EmptyInitialization	= false; // Do not se default values, this will disable mipmaps and set the Image handlers to 0.
		uint32_t Width				= 0; // Used for empty initialization.
		uint32_t Height				= 0; // Used for empty initialization.		
		DXGI_FORMAT Format			= DXGI_FORMAT_R8G8B8A8_UNORM; // Used for empty initialization.
	};

	struct ModelLoadDesc
	{
		enum class Loader : uint32
		{
			DEFAULT = FLAG(0),
			TINYOBJ = FLAG(1),
			ASSIMP = FLAG(2)
		};

		using LoaderFlags = uint32;
		enum LoaderFlag : LoaderFlags
		{
			/*
				Will create buffers on the GPU to hold the vertices and indices data.
			*/
			LOADER_FLAG_UPLOAD_MESH_DATA_TO_GUP = FLAG(0),
			/*
				Will not save the vertices and indices data after the mesh has been loaded.
				- If LOADER_FLAG_UPLOAD_MESH_DATA_TO_GUP was not set, this will just create a model with no mesh data (An empty model).
				- If LOADER_FLAG_UPLOAD_MESH_DATA_TO_GUP was set, it will save the data in the buffers on the GPU before clearing them.
			*/
			LOADER_FLAG_NO_MESH_DATA_IN_RAM = FLAG(1),
			/*
				Generate AABBs for all models in the hierarchy.
			*/
			LOADER_FLAG_GENERATE_BOUNDING_BOX = FLAG(2),
			/*
				Set the winding order to clock wise, default is counter clock wise.
			*/
			LOADER_FLAG_WINDING_ORDER_CW = FLAG(3),
			/*
				Set the UV origin to the top left corner, default is bottom left.
			*/
			LOADER_FLAG_USE_UV_TOP_LEFT = FLAG(4)

		};

		std::string	FilePath = "";
		Loader		Loader = Loader::DEFAULT;
		LoaderFlags	Flags = LOADER_FLAG_UPLOAD_MESH_DATA_TO_GUP | LOADER_FLAG_NO_MESH_DATA_IN_RAM | LOADER_FLAG_GENERATE_BOUNDING_BOX | LOADER_FLAG_USE_UV_TOP_LEFT;
	};
}