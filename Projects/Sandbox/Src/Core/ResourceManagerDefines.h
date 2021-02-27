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

		std::string		FilePath = "";
		Channels		NumChannels = Channels::DEFAULT;
	};

	struct TextureLoadDesc
	{
		ImageLoadDesc ImageDesc;
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
		LoaderFlags	Flags = LOADER_FLAG_UPLOAD_MESH_DATA_TO_GUP | LOADER_FLAG_NO_MESH_DATA_IN_RAM | LOADER_FLAG_GENERATE_BOUNDING_BOX;
	};
}