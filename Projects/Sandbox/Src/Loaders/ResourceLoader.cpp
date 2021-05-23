#include "PreCompiled.h"
#include "ResourceLoader.h"

#pragma warning( push )
#pragma warning( disable : 6011 )
#pragma warning( disable : 6262 )
#pragma warning( disable : 6308 )
#pragma warning( disable : 6387 )
#pragma warning( disable : 26451 )
#pragma warning( disable : 28182 )
#include <stb_image.h>
#pragma warning( pop )

#include "Renderer/RenderUtils.h"

using namespace RS;

void ResourceLoader::LoadImageFromFile(ImageResource*& outImage, ImageLoadDesc& imageDescription)
{
	int nChannels = 0;
	switch (imageDescription.NumChannels)
	{
	case ImageLoadDesc::Channels::R:		nChannels = 1; break;
	case ImageLoadDesc::Channels::RG:		nChannels = 2; break;
	case ImageLoadDesc::Channels::RGB:		nChannels = 3; break;
	case ImageLoadDesc::Channels::RGBA:		nChannels = 4; break;
	case ImageLoadDesc::Channels::DEFAULT:
	default:
		nChannels = 0;
		break;
	}

	bool isHDR = false;
	size_t dotPos = imageDescription.File.Path.rfind('.');
	if (dotPos == std::string::npos)
		LOG_WARNING("File extension is missing from path {}!", imageDescription.File.Path.c_str());
	else
	{
		std::string extension = imageDescription.File.Path.substr(dotPos + 1);
		isHDR = extension == "hdr";
	}

	std::string path = imageDescription.File.Path;
	if (imageDescription.File.UseDefaultFolder)
		path = std::string(RS_TEXTURE_PATH) + imageDescription.File.Path;
	int width = 0, height = 0, channelCount = 0;
	if (nChannels < 0 || nChannels > 4)
	{
		outImage->Data.clear();
		LOG_WARNING("Unable to load image [{0}]: Requested number of channels is not supported! Requested {1}.", path.c_str(), nChannels);
	}
	else
	{
		if (isHDR)
		{
			// Loading HDR images will ignore channelCount and instead always use a R16G16B16A16_FLOAT format.
			float* pPixels = stbi_loadf(path.c_str(), &width, &height, &channelCount, 4);
			if (!pPixels)
				LOG_WARNING("Unable to load HDR image [{0}]: File not found!", path.c_str());
			else
			{
				outImage->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				size_t size = (size_t)width * (size_t)height * (size_t)RenderUtils::GetSizeOfFormat(outImage->Format);
				outImage->Data.resize(size, 0);
				memcpy(outImage->Data.data(), pPixels, size);
				stbi_image_free(pPixels);
				pPixels = nullptr;
			}
		}
		else
		{
			uint8* pPixels = (uint8*)stbi_load(path.c_str(), &width, &height, &channelCount, nChannels);
			if (!pPixels)
				LOG_WARNING("Unable to load image [{0}]: File not found!", path.c_str());
			else
			{
				size_t size = (size_t)width * (size_t)height * (size_t)(nChannels == 0 ? channelCount : nChannels);
				outImage->Data.resize(size, 0);
				memcpy(outImage->Data.data(), pPixels, size);
				stbi_image_free(pPixels);
				pPixels = nullptr;

				outImage->Format = GetFormatFromChannelCount(nChannels == 0 ? channelCount : nChannels);
			}
		}
	}

	outImage->Width = (unsigned int)width;
	outImage->Height = (unsigned int)height;
}

void ResourceLoader::LoadImageFromMemory(ImageResource*& outImage, ImageLoadDesc& imageDescription)
{
	int nChannels = 0;
	switch (imageDescription.NumChannels)
	{
	case ImageLoadDesc::Channels::R:		nChannels = 1; break;
	case ImageLoadDesc::Channels::RG:		nChannels = 2; break;
	case ImageLoadDesc::Channels::RGB:		nChannels = 3; break;
	case ImageLoadDesc::Channels::RGBA:		nChannels = 4; break;
	case ImageLoadDesc::Channels::DEFAULT:
	default:
		nChannels = 0;
		break;
	}

	uint8* pPixels = nullptr;
	int width = 0, height = 0, channelCount = 0;
	if (imageDescription.Memory.pData)
	{
		if (imageDescription.Memory.IsCompressed)
		{
			pPixels = (uint8*)stbi_load_from_memory(imageDescription.Memory.pData, imageDescription.Memory.Size, &width, &height, &channelCount, nChannels);
			if (!pPixels)
				LOG_WARNING("Unable to load image from memory: Something went wrong!");
			else
			{
				size_t size = (size_t)(width * height * (nChannels == 0 ? channelCount : nChannels));
				outImage->Data.resize(size, 0);
				memcpy(outImage->Data.data(), pPixels, size);
				stbi_image_free(pPixels);
				pPixels = nullptr;
			}
		}
		else
		{
			outImage->Data.resize((size_t)imageDescription.Memory.Size, 0);
			memcpy(outImage->Data.data(), imageDescription.Memory.pData, (size_t)imageDescription.Memory.Size);
			width = imageDescription.Memory.Width;
			height = imageDescription.Memory.Height;
		}
		outImage->Format = GetFormatFromChannelCount(nChannels == 0 ? channelCount : nChannels);
	}
	else
	{
		outImage->Data.clear();
		width = imageDescription.Memory.Width;
		height = imageDescription.Memory.Height;
		outImage->Format = GetFormatFromChannelCount(nChannels == 0 ? channelCount : nChannels);
	}

	outImage->Width = (unsigned int)width;
	outImage->Height = (unsigned int)height;
}

DXGI_FORMAT ResourceLoader::GetFormatFromChannelCount(int nChannels)
{
	switch (nChannels)
	{
	case 1: return DXGI_FORMAT_R8_UNORM; break;
	case 2: return DXGI_FORMAT_R8G8_UNORM; break;
	case 3:
		LOG_WARNING("A texture format with 3 channels are not supported!");
		[[fallthrough]];
	case 4: return DXGI_FORMAT_R8G8B8A8_UNORM; break;
	default:
		return DXGI_FORMAT_R8_UNORM; break;
	}
}
