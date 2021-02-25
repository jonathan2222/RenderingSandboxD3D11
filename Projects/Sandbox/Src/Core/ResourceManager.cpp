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

TextureResource* ResourceManager::LoadTextureResource(const std::string& fileName, int nChannels)
{
	auto [pTexture, isNew] = AddResource<TextureResource>(fileName, Resource::Type::TEXTURE);

	// Only load the texture if it has not been loaded.
	if (isNew)
	{
		std::string path = std::string(RS_TEXTURE_PATH) + fileName;
		int width = 0, height = 0, channelCount = 0;
		if (nChannels < 0 || nChannels > 4)
		{
			pTexture->Data = nullptr;
			LOG_WARNING("Unable to load texture [{0}]: Requested number of channels is not supported! Requested {1}.", path.c_str(), nChannels);
		}
		else
		{
			pTexture->Data = (void*)stbi_load(path.c_str(), &width, &height, &channelCount, nChannels);
			pTexture->Format = GetFormatFromChannelCount(nChannels == 0 ? channelCount : nChannels);
		}

		pTexture->Width = (unsigned int)width;
		pTexture->Height = (unsigned int)height;

		if (!pTexture->Data)
			LOG_WARNING("Unable to load texture [{0}]: file not found!", path.c_str());
	}

	return pTexture;
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
		case RS::Resource::Type::TEXTURE:
		{
			TextureResource* pTexture = dynamic_cast<TextureResource*>(pResource);
			FreeTexture(pTexture);
		}
		break;
		case RS::Resource::Type::MODEL:
		{
			ModelResource* pModel = dynamic_cast<ModelResource*>(pResource);
			pModel->Vertices.clear();
			pModel->Indices.clear();
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

void ResourceManager::FreeTexture(TextureResource* pTexture)
{
	if (pTexture)
	{
		if (pTexture->Data)
			stbi_image_free(pTexture->Data);
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
