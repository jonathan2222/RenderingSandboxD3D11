#pragma once

#include "Resources/Resources.h"
#include "Core/ResourceManagerDefines.h"

namespace RS
{
	class ResourceLoader
	{
	public:
		static void LoadImageFromFile(ImageResource*& outImage, ImageLoadDesc& imageDescription);
		static void LoadImageFromMemory(ImageResource*& outImage, ImageLoadDesc& imageDescription);

	private:
		static DXGI_FORMAT GetFormatFromChannelCount(int nChannels);
	};
}