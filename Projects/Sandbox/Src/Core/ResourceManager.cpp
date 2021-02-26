#include "PreCompiled.h"
#include "ResourceManager.h"

#include "Loaders/ModelLoader.h"

#pragma warning( push )
#pragma warning( disable : 6011 )
#pragma warning( disable : 6262 )
#pragma warning( disable : 6308 )
#pragma warning( disable : 6387 )
#pragma warning( disable : 26451 )
#pragma warning( disable : 28182 )
//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning( pop )


using namespace RS;

std::shared_ptr<ResourceManager> ResourceManager::Get()
{
    static std::shared_ptr<ResourceManager> resourceManager = std::make_shared<ResourceManager>();
    return resourceManager;
}

void ResourceManager::Init()
{
}

void ResourceManager::Release()
{
	// Remove all resources which was not freed.
	for (auto [key, pResource] : m_Resources)
	{
		RemoveResource(pResource);
		delete pResource;
	}
	m_Resources.clear();
}

ImageResource* ResourceManager::LoadImageResource(ImageLoadDesc& imageDescription)
{
	auto [pImage, isNew] = AddResource<ImageResource>(imageDescription.FilePath, Resource::Type::IMAGE);

	// Only load the iamge if it has not been loaded.
	if (isNew)
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

		std::string path = std::string(RS_TEXTURE_PATH) + imageDescription.FilePath;
		int width = 0, height = 0, channelCount = 0;
		if (nChannels < 0 || nChannels > 4)
		{
			pImage->Data = nullptr;
			LOG_WARNING("Unable to load image [{0}]: Requested number of channels is not supported! Requested {1}.", path.c_str(), nChannels);
		}
		else
		{
			pImage->Data = (void*)stbi_load(path.c_str(), &width, &height, &channelCount, nChannels);
			pImage->Format = GetFormatFromChannelCount(nChannels == 0 ? channelCount : nChannels);
		}

		pImage->Width = (unsigned int)width;
		pImage->Height = (unsigned int)height;

		if (!pImage->Data)
			LOG_WARNING("Unable to load image [{0}]: File not found!", path.c_str());
	}

	return pImage;
}

ModelResource* ResourceManager::LoadModelResource(const std::string& filePath)
{
	auto [pModel, isNew] = AddResource<ModelResource>(filePath, Resource::Type::MODEL);

	// Only load the texture if it has not been loaded.
	if (isNew)
	{
		ModelLoader::Load(filePath, pModel);
	}

	return pModel;
}

void ResourceManager::FreeResource(Resource* pResource)
{
	pResource->RemoveRef();
	UpdateStats(pResource);
	if (pResource->GetRefCount() == 0)
	{
		// Remove resource if it is the last pointer.
		if (RemoveResource(pResource))
		{
			m_Resources.erase(pResource->key);
			delete pResource;
		}
	}
}

ResourceManager::Stats ResourceManager::GetStats()
{
	Stats stats;
	stats.pResourcesRefCount = &m_ResourcesRefCount;
	return stats;
}

bool ResourceManager::RemoveResource(Resource* pResource)
{
	auto it = m_Resources.find(pResource->key);
	if (it != m_Resources.end())
	{
		Resource::Type type = pResource->type;
		switch (type)
		{
		case RS::Resource::Type::IMAGE:
		{
			ImageResource* pImage = dynamic_cast<ImageResource*>(pResource);
			FreeImage(pImage);
		}
		break;
		case RS::Resource::Type::MODEL:
		{
			ModelResource* pModel = dynamic_cast<ModelResource*>(pResource);
			pModel->Meshes.clear();
		}
		break;
		default:
			LOG_WARNING("Trying to free a resource which has an unsupported type!");
			break;
		}

		return true;
	}
	else
	{
		LOG_WARNING("Could not remove resource! Resource was not found!");
		return false;
	}
}

void ResourceManager::FreeImage(ImageResource* pImage)
{
	if (pImage)
	{
		if (pImage->Data)
			stbi_image_free(pImage->Data);
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

void ResourceManager::UpdateStats(Resource* pResrouce)
{
	auto it = m_ResourcesRefCount.find(pResrouce->type);
	if (it == m_ResourcesRefCount.end())
	{
		m_ResourcesRefCount[pResrouce->type] = pResrouce->GetRefCount();
	}
	else
	{
		it->second = pResrouce->GetRefCount();
	}
}
