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
	for (auto [key, pResource] : m_IDToResourceMap)
	{
		RemoveResource(pResource);
		delete pResource;
	}
	m_IDToResourceMap.clear();
}

std::pair<ImageResource*, ResourceID> ResourceManager::LoadImageResource(ImageLoadDesc& imageDescription)
{
	std::string fullKey = imageDescription.FilePath + Resource::TypeToString(Resource::Type::IMAGE);
	ResourceID id = GetIDFromString(fullKey);
	auto [pImage, isNew] = AddResource<ImageResource>(id, Resource::Type::IMAGE);

	// Only load the iamge if it has not been loaded.
	if (isNew)
		LoadImageInternal(pImage, imageDescription);

	return { pImage, id };
}

std::pair<TextureResource*, ResourceID> ResourceManager::LoadTextureResource(TextureLoadDesc& textureDescription)
{
	std::string fullKey = textureDescription.ImageDesc.FilePath + Resource::TypeToString(Resource::Type::TEXTURE);
	ResourceID id = GetIDFromString(fullKey);
	auto [pTexture, isNewTexture] = AddResource<TextureResource>(id, Resource::Type::TEXTURE);

	// Only load the texture if it has not been loaded.
	if (isNewTexture)
	{
		auto [pImage, isNewImage] = LoadImageResource(textureDescription.ImageDesc);
		pTexture->ImageHandler = pImage->key;

		{
			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width				= pImage->Width;
			textureDesc.Height				= pImage->Height;
			textureDesc.Format				= pImage->Format;
			textureDesc.MipLevels			= 1;
			textureDesc.ArraySize			= 1;
			textureDesc.SampleDesc.Count	= 1;
			textureDesc.SampleDesc.Quality	= 0;
			textureDesc.Usage				= D3D11_USAGE_IMMUTABLE;
			textureDesc.CPUAccessFlags		= 0;
			textureDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
			textureDesc.MiscFlags			= 0;
			
			D3D11_SUBRESOURCE_DATA data = {};
			data.pSysMem				= pImage->Data;
			data.SysMemPitch			= pImage->Width * 4;
			data.SysMemSlicePitch		= 0;

			HRESULT result = RenderAPI::Get()->GetDevice()->CreateTexture2D(&textureDesc, &data, &pTexture->pTexture);
			RS_D311_ASSERT_CHECK(result, "Failed to create texture!");

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format						= textureDesc.Format;
			srvDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip	= 0;
			srvDesc.Texture2D.MipLevels			= 1;
			result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pTexture->pTexture, &srvDesc, &pTexture->pTextureSRV);
			RS_D311_ASSERT_CHECK(result, "Failed to create texture RSV!");

			D3D11_SAMPLER_DESC samplerDesc = {};
			samplerDesc.Filter				= D3D11_FILTER_ANISOTROPIC;
			samplerDesc.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MipLODBias			= 0.f;
			samplerDesc.MaxAnisotropy		= 16;
			samplerDesc.ComparisonFunc		= D3D11_COMPARISON_GREATER_EQUAL;
			samplerDesc.MinLOD				= 0.f;
			samplerDesc.MaxLOD				= D3D11_FLOAT32_MAX;
			samplerDesc.BorderColor[0]		= 0.f;
			samplerDesc.BorderColor[1]		= 0.f;
			samplerDesc.BorderColor[2]		= 0.f;
			samplerDesc.BorderColor[3]		= 0.f;

			result = RenderAPI::Get()->GetDevice()->CreateSamplerState(&samplerDesc, &pTexture->pSampler);
			RS_D311_ASSERT_CHECK(result, "Failed to create sampler!");
		}
	}

	return { pTexture, id };
}

std::pair<ModelResource*, ResourceID> ResourceManager::LoadModelResource(ModelLoadDesc& modelDescription)
{
	std::string fullKey = modelDescription.FilePath + Resource::TypeToString(Resource::Type::MODEL);
	ResourceID id = GetIDFromString(fullKey);
	auto [pModel, isNew] = AddResource<ModelResource>(id, Resource::Type::MODEL);

	// Only load the model if it has not been loaded.
	if (isNew)
	{
		if(modelDescription.Loader == ModelLoadDesc::Loader::TINYOBJ)
			ModelLoader::Load(modelDescription.FilePath, pModel, modelDescription.Flags);
		else if (modelDescription.Loader == ModelLoadDesc::Loader::ASSIMP 
			|| modelDescription.Loader == ModelLoadDesc::Loader::DEFAULT)
			ModelLoader::LoadWithAssimp(modelDescription.FilePath, pModel, modelDescription.Flags);
	}

	return { pModel, id };
}

void ResourceManager::FreeResource(Resource* pResource)
{
	pResource->RemoveRef();
	UpdateStats(pResource, false);
	if (pResource->GetRefCount() == 0)
	{
		// Remove resource if it is the last pointer.
		if (RemoveResource(pResource))
		{
			for (auto& [key, id] : m_StringToResourceIDMap)
			{
				if (id == pResource->key)
				{
					m_StringToResourceIDMap.erase(key);
					break;
				}
			}
			m_IDToResourceMap.erase(pResource->key);
			delete pResource;
		}
	}
}

ResourceManager::Stats ResourceManager::GetStats()
{
	Stats stats;
	stats.pTypeResourcesRefCount = &m_TypeResourcesRefCount;
	stats.pResourcesRefCount = &m_ResourcesRefCount;
	stats.pStringToResourceIDMap = &m_StringToResourceIDMap;
	return stats;
}

void ResourceManager::LoadImageInternal(ImageResource*& outImage, ImageLoadDesc& imageDescription)
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
		outImage->Data = nullptr;
		LOG_WARNING("Unable to load image [{0}]: Requested number of channels is not supported! Requested {1}.", path.c_str(), nChannels);
	}
	else
	{
		outImage->Data = (void*)stbi_load(path.c_str(), &width, &height, &channelCount, nChannels);
		outImage->Format = GetFormatFromChannelCount(nChannels == 0 ? channelCount : nChannels);
	}

	outImage->Width = (unsigned int)width;
	outImage->Height = (unsigned int)height;

	if (!outImage->Data)
		LOG_WARNING("Unable to load image [{0}]: File not found!", path.c_str());
}

bool ResourceManager::RemoveResource(Resource* pResource)
{
	auto it = m_IDToResourceMap.find(pResource->key);
	if (it != m_IDToResourceMap.end())
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
		case RS::Resource::Type::TEXTURE:
		{
			ImageResource* pImage = dynamic_cast<ImageResource*>(pResource);
			FreeImage(pImage);
		}
		break;
		case RS::Resource::Type::MODEL:
		{
			ModelResource* pModel = dynamic_cast<ModelResource*>(pResource);
			FreeModelRecursive(pModel);
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
		pImage->Width	= 0;
		pImage->Height	= 0;
		pImage->Format	= DXGI_FORMAT_UNKNOWN;
	}
	else
		LOG_WARNING("Trying to free an image pointer which is NULL!");
}

void ResourceManager::FreeTexture(TextureResource* pTexture)
{
	ImageResource* pImage = GetResource<ImageResource>(pTexture->ImageHandler);
	FreeResource(pImage); // This will only release it, if it is the last handler.
	
	if (pTexture->pTexture)
	{
		pTexture->pTexture->Release();
		pTexture->pTexture = nullptr;
	}

	if (pTexture->pTextureSRV)
	{
		pTexture->pTextureSRV->Release();
		pTexture->pTextureSRV = nullptr;
	}

	if (pTexture->pSampler)
	{
		pTexture->pSampler->Release();
		pTexture->pSampler = nullptr;
	}
}

void ResourceManager::FreeModelRecursive(ModelResource* pModel)
{
	pModel->Transform = glm::mat4(1.f);
	
	for (MeshResource& mesh : pModel->Meshes)
	{
		if (mesh.pVertexBuffer)
		{
			mesh.pVertexBuffer->Release();
			mesh.pVertexBuffer = nullptr;
		}

		if (mesh.pIndexBuffer)
		{
			mesh.pIndexBuffer->Release();
			mesh.pIndexBuffer = nullptr;
		}

		if (mesh.pMeshBuffer)
		{
			mesh.pMeshBuffer->Release();
			mesh.pMeshBuffer = nullptr;
		}

		mesh.Vertices.clear();
		mesh.Indices.clear();
		mesh.NumVertices = 0;
		mesh.NumIndices = 0;
	}
	pModel->Meshes.clear();
	
	for (ModelResource& model : pModel->Children)
		FreeModelRecursive(&model);

	pModel->Children.clear();
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

void ResourceManager::UpdateStats(Resource* pResrouce, bool add)
{
	// Update type ref count stats
	{
		auto it = m_TypeResourcesRefCount.find(pResrouce->type);
		if (it == m_TypeResourcesRefCount.end())
		{
			m_TypeResourcesRefCount[pResrouce->type] = 1;
		}
		else
		{
			if (add)
				it->second++;
			else
				it->second--;
		}
	}

	// Update resource ref count stats
	{
		auto it = m_ResourcesRefCount.find(pResrouce->key);
		if (it == m_ResourcesRefCount.end())
			m_ResourcesRefCount[pResrouce->key] = pResrouce->GetRefCount();
		else
			it->second = pResrouce->GetRefCount();
	}
}

ResourceID ResourceManager::GetIDFromString(const std::string& key)
{
	auto it = m_StringToResourceIDMap.find(key);
	if (it == m_StringToResourceIDMap.end())
	{
		ResourceID id = GetNextID();
		m_StringToResourceIDMap[key] = id;
		return id;
	}
	return it->second;
}

ResourceID ResourceManager::GetNextID() const
{
	static ResourceID s_Generator = 0;
	return ++s_Generator;
}
