#include "PreCompiled.h"
#include "ResourceManager.h"

#pragma warning( push )
#pragma warning( disable : 6011 )
#pragma warning( disable : 6262 )
#pragma warning( disable : 6308 )
#pragma warning( disable : 6387 )
#pragma warning( disable : 26451 )
#pragma warning( disable : 28182 )
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning( pop )

using namespace RS;

std::shared_ptr<ResourceManager> ResourceManager::Get()
{
    static std::shared_ptr<ResourceManager> resourceManager = std::make_shared<ResourceManager>();
    return resourceManager;
}

ResourceManager::Image* ResourceManager::LoadTexture(const std::string& fileName, int nChannels)
{
	std::string path = std::string(RS_TEXTURE_PATH) + fileName;
	Image* img = new Image();
	int width = 0, height = 0, channelCount = 0;
	if (nChannels < 0 || nChannels > 4)
	{
		img->Data = nullptr;
		LOG_WARNING("Unable to load texture [{0}]: Requested number of channels is not supported! Requested {1}.", path.c_str(), nChannels);
	}
	else
	{
		img->Data = (void*)stbi_load(path.c_str(), &width, &height, &channelCount, nChannels);
		img->Format = GetFormatFromChannelCount(nChannels == 0 ? channelCount : nChannels);
	}

	img->Width = (unsigned int)width;
	img->Height = (unsigned int)height;

	if (!img->Data)
		LOG_WARNING("Unable to load texture [{0}]: file not found!", path.c_str());

	return img;
}

void ResourceManager::FreeTexture(Image* image)
{
	if (image)
	{
		if (image->Data)
			stbi_image_free(image->Data);
		delete image;
	}
	else
		LOG_WARNING("Trying to free an image pointer which is NULL!");
}

DXGI_FORMAT ResourceManager::GetFormatFromChannelCount(int nChannels) const
{
	switch (nChannels)
	{
	case 1: return DXGI_FORMAT_R8_UNORM; break;
	case 2: return DXGI_FORMAT_R8G8_UNORM; break;
	case 3:
		LOG_WARNING("A texture format with 3 channels are not supported!");
	case 4:
	default:
		return DXGI_FORMAT_R8_UNORM; break;
	}
}
