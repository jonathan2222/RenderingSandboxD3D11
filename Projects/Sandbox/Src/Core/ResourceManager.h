#pragma once

#include "Renderer/RenderAPI.h"

namespace RS
{
	class ResourceManager
	{
	public:
		struct Image
		{
			void*		Data	= nullptr;
			uint32		Width	= 0;
			uint32		Height	= 0;
			DXGI_FORMAT Format	= DXGI_FORMAT_UNKNOWN;
		};

	public:
		RS_DEFAULT_ABSTRACT_CLASS(ResourceManager);

		static std::shared_ptr<ResourceManager> Get();

		/*
				Load a image from memory.
				Arguments:
					* fileName: The path to the file, including the name of the image file with its extention.
					* nChannels: How many channels it will use. It can be 0, 1, 2, 3 or 4.
						If it is set to 0, it will use the same number of channels as the image file on disk.
				Returns:
					A pointer to the image structure.
			*/
		Image* LoadTexture(const std::string& fileName, int nChannels);

		/*
			Free image data and deallocate the image structure.
			Arguments:
				* image: A pointer to the image structure.
		*/
		void FreeTexture(Image* image);

	private:
		DXGI_FORMAT GetFormatFromChannelCount(int nChannels) const;

	};
}