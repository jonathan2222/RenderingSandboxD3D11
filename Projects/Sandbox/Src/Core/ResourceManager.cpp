#include "PreCompiled.h"
#include "ResourceManager.h"

#include "Loaders/ModelLoader.h"

#include "Renderer/ImGuiRenderer.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Renderer.h"

#include "Renderer/D3D11/D3D11Helper.h"

#include "Loaders/ResourceLoader.h"

#include <unordered_set>

using namespace RS;

std::shared_ptr<ResourceManager> ResourceManager::Get()
{
    static std::shared_ptr<ResourceManager> resourceManager = std::make_shared<ResourceManager>();
    return resourceManager;
}

void ResourceManager::Init()
{
	// Load default textures!
	{
		// Load a white texture
		{
			std::vector<uint8> whitePixel = { (uint8)0xFF, (uint8)0xFF, (uint8)0xFF, (uint8)0xFF };
			TextureLoadDesc loadDesc = {};
			loadDesc.ImageDesc.Memory.pData			= whitePixel.data();
			loadDesc.ImageDesc.Memory.Size			= (uint32)whitePixel.size();
			loadDesc.ImageDesc.Memory.Width			= 1;
			loadDesc.ImageDesc.Memory.Height		= 1;
			loadDesc.ImageDesc.Memory.IsCompressed	= false;
			loadDesc.ImageDesc.IsFromFile			= false;
			loadDesc.ImageDesc.NumChannels			= ImageLoadDesc::Channels::RGBA;
			loadDesc.ImageDesc.Name					= "RS_WHITE_PIXEL";
			loadDesc.GenerateMipmaps				= false; // Dose not need mipmaps because this is a 1x1 pixel image.
			auto [pTexture, ID] = LoadTextureResource(loadDesc);
			DefaultTextureOnePixelWhite = ID;
		}

		// Load a black texture
		{
			std::vector<uint8> blackPixel = { (uint8)0x00, (uint8)0x00, (uint8)0x00, (uint8)0xFF };
			TextureLoadDesc loadDesc = {};
			loadDesc.ImageDesc.Memory.pData			= blackPixel.data();
			loadDesc.ImageDesc.Memory.Size			= (uint32)blackPixel.size();
			loadDesc.ImageDesc.Memory.Width			= 1;
			loadDesc.ImageDesc.Memory.Height		= 1;
			loadDesc.ImageDesc.Memory.IsCompressed	= false;
			loadDesc.ImageDesc.IsFromFile			= false;
			loadDesc.ImageDesc.NumChannels			= ImageLoadDesc::Channels::RGBA;
			loadDesc.ImageDesc.Name					= "RS_BLACK_PIXEL";
			loadDesc.GenerateMipmaps				= false; // Dose not need mipmaps because this is a 1x1 pixel image.
			auto [pTexture, ID] = LoadTextureResource(loadDesc);
			DefaultTextureOnePixelBlack = ID;
		}

		// Load a Normal texture
		{
			std::vector<uint8> normalPixel = { (uint8)128, (uint8)128, (uint8)255, (uint8)0xFF };
			TextureLoadDesc loadDesc = {};
			loadDesc.ImageDesc.Memory.pData			= normalPixel.data();
			loadDesc.ImageDesc.Memory.Size			= (uint32)normalPixel.size();
			loadDesc.ImageDesc.Memory.Width			= 1;
			loadDesc.ImageDesc.Memory.Height		= 1;
			loadDesc.ImageDesc.Memory.IsCompressed	= false;
			loadDesc.ImageDesc.IsFromFile			= false;
			loadDesc.ImageDesc.NumChannels			= ImageLoadDesc::Channels::RGBA;
			loadDesc.ImageDesc.Name					= "RS_NORMAL_PIXEL";
			loadDesc.GenerateMipmaps				= false; // Dose not need mipmaps because this is a 1x1 pixel image.
			auto [pTexture, ID] = LoadTextureResource(loadDesc);
			DefaultTextureOnePixelNormal = ID;
		}

		// Load a Anisotropic Sampler
		{
			SamplerLoadDesc loadDesc = {};
			loadDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			loadDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			loadDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			loadDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			loadDesc.MipLODBias = 0.f;
			loadDesc.MaxAnisotropy = 16;
			loadDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
			loadDesc.MinLOD = 0.f;
			loadDesc.MaxLOD = D3D11_FLOAT32_MAX;
			loadDesc.BorderColor[0] = 0.f;
			loadDesc.BorderColor[1] = 0.f;
			loadDesc.BorderColor[2] = 0.f;
			loadDesc.BorderColor[3] = 0.f;
			auto [pSampler, ID] = LoadSamplerResource(loadDesc);
			DefaultSamplerAnisotropic = ID;
		}

		// Load a Linear Sampler
		{
			SamplerLoadDesc loadDesc = {};
			loadDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			loadDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			loadDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			loadDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			loadDesc.MipLODBias = 0.f;
			loadDesc.MaxAnisotropy = 0;
			loadDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
			loadDesc.MinLOD = 0.f;
			loadDesc.MaxLOD = D3D11_FLOAT32_MAX;
			loadDesc.BorderColor[0] = 0.f;
			loadDesc.BorderColor[1] = 0.f;
			loadDesc.BorderColor[2] = 0.f;
			loadDesc.BorderColor[3] = 0.f;
			auto [pSampler, ID] = LoadSamplerResource(loadDesc);
			DefaultSamplerLinear = ID;
		}

		// Load a Nearest Sampler
		{
			SamplerLoadDesc loadDesc = {};
			loadDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			loadDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			loadDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			loadDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			loadDesc.MipLODBias = 0.f;
			loadDesc.MaxAnisotropy = 16;
			loadDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
			loadDesc.MinLOD = 0.f;
			loadDesc.MaxLOD = D3D11_FLOAT32_MAX;
			loadDesc.BorderColor[0] = 0.f;
			loadDesc.BorderColor[1] = 0.f;
			loadDesc.BorderColor[2] = 0.f;
			loadDesc.BorderColor[3] = 0.f;
			auto [pSampler, ID] = LoadSamplerResource(loadDesc);
			DefaultSamplerNearest = ID;
		}
	}
}

void ResourceManager::Release()
{
	// Remove all resources which was not freed.
	for (auto& [key, pResource] : m_IDToResourceMap)
	{
		RemoveResource(pResource, true);
		delete pResource;
		pResource = nullptr;
	}
	m_IDToResourceMap.clear();
	m_StringToResourceIDMap.clear();
	m_TypeResourcesRefCount.clear();
	m_ResourcesRefCount.clear();
}

std::pair<ImageResource*, ResourceID> ResourceManager::LoadImageResource(ImageLoadDesc& imageDescription)
{
	std::string fullKey = GetImageResourceStringKey(imageDescription);
	ResourceID id = GetAndAddIDFromString(fullKey);
	auto [pImage, isNew] = AddResource<ImageResource>(id, Resource::Type::IMAGE);

	// Only load the iamge if it has not been loaded.
	if (isNew)
	{
		if(imageDescription.IsFromFile)
			ResourceLoader::LoadImageFromFile(pImage, imageDescription);
		else
			ResourceLoader::LoadImageFromMemory(pImage, imageDescription);
	}

	return { pImage, id };
}

ImageResource* RS::ResourceManager::LoadImageResource(ResourceID id)
{
	ImageResource* pImage = GetResource<ImageResource>(id);
	if (pImage)
	{
		pImage->AddRef();
		UpdateStats(pImage, true);
	}
	return pImage;
}

std::pair<SamplerResource*, ResourceID> ResourceManager::LoadSamplerResource(SamplerLoadDesc samplerLoadDesc)
{
	std::string fullKey = GetSamplerResourceStringKey(samplerLoadDesc);
	ResourceID id = GetAndAddIDFromString(fullKey);
	auto [pSampler, isNew] = AddResource<SamplerResource>(id, Resource::Type::SAMPLER);

	// Only load the sampler if it has not been loaded.
	if (isNew)
	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter			= samplerLoadDesc.Filter;
		samplerDesc.AddressU		= samplerLoadDesc.AddressU;
		samplerDesc.AddressV		= samplerLoadDesc.AddressV;
		samplerDesc.AddressW		= samplerLoadDesc.AddressW;
		samplerDesc.MipLODBias		= samplerLoadDesc.MipLODBias;
		samplerDesc.MaxAnisotropy	= samplerLoadDesc.MaxAnisotropy;
		samplerDesc.ComparisonFunc	= samplerLoadDesc.ComparisonFunc;
		samplerDesc.MinLOD			= samplerLoadDesc.MinLOD;
		samplerDesc.MaxLOD			= samplerLoadDesc.MaxLOD;
		memcpy(samplerDesc.BorderColor, samplerLoadDesc.BorderColor, sizeof(FLOAT) * 4);

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateSamplerState(&samplerDesc, &pSampler->pSampler);
		RS_D311_ASSERT_CHECK(result, "Failed to create sampler!");
	}

	return { pSampler, id };
}

SamplerResource* ResourceManager::LoadSamplerResource(ResourceID id)
{
	SamplerResource* pSampler = GetResource<SamplerResource>(id);
	if (pSampler)
	{
		pSampler->AddRef();
		UpdateStats(pSampler, true);
	}
	return pSampler;
}

std::pair<TextureResource*, ResourceID> ResourceManager::LoadTextureResource(TextureLoadDesc& textureDescription)
{
	std::string fullKey = GetTextureResourceStringKey(textureDescription);
	ResourceID id = GetAndAddIDFromString(fullKey);
	auto [pTexture, isNewTexture] = AddResource<TextureResource>(id, Resource::Type::TEXTURE);

	// Only load the texture if it has not been loaded.
	if (isNewTexture)
	{
		bool isEmpty = !textureDescription.ImageDesc.IsFromFile && (textureDescription.ImageDesc.Memory.pData == nullptr);

		auto [pImage, imageId] = LoadImageResource(textureDescription.ImageDesc);
		pTexture->ImageHandler = pImage->key;
		pTexture->Format = pImage->Format;

		{
			D3D11_TEXTURE2D_DESC textureDesc = D3D11Helper::GetTexture2DDesc(pImage->Width, pImage->Height, pImage->Format);
			textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			if (textureDescription.GenerateMipmaps)
			{
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
				textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
				textureDesc.MipLevels = (uint32)glm::ceil(glm::max(glm::log2(glm::min((float)textureDesc.Width, (float)textureDesc.Height)), 1.f));
			}

			if (isEmpty)
			{
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
			}

			pTexture->UseAsRTV = textureDescription.UseAsRTV;
			if (pTexture->UseAsRTV)
			{
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			}

			pTexture->NumMipLevels = textureDesc.MipLevels;
			
			std::vector<D3D11_SUBRESOURCE_DATA> subData;
			D3D11_SUBRESOURCE_DATA* pSubData = nullptr;
			if (!isEmpty)
			{
				subData = D3D11Helper::FillTexture2DSubdata(textureDesc, pImage->Data.data());
				pSubData = subData.data();
			}
			HRESULT result = RenderAPI::Get()->GetDevice()->CreateTexture2D(&textureDesc, pSubData, &pTexture->pTexture);
			RS_D311_ASSERT_CHECK(result, "Failed to create texture!");

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format						= textureDesc.Format;
			srvDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip	= 0;
			srvDesc.Texture2D.MipLevels			= textureDescription.GenerateMipmaps ? -1 : 1;
			result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pTexture->pTexture, &srvDesc, &pTexture->pTextureSRV);
			RS_D311_ASSERT_CHECK(result, "Failed to create texture RSV!");

			if (textureDescription.GenerateMipmaps)
				GenerateTextureMipmaps(pTexture);
		}
	}

	return { pTexture, id };
}

TextureResource* ResourceManager::LoadTextureResource(ResourceID id)
{
	TextureResource* pTexture = GetResource<TextureResource>(id);
	if (pTexture)
	{
		pTexture->AddRef();
		UpdateStats(pTexture, true);
		LoadImageResource(pTexture->ImageHandler);
	}
	return pTexture;
}

std::pair<CubeMapResource*, ResourceID> ResourceManager::LoadCubeMapResource(CubeMapLoadDesc& cubeMapDescription)
{
	std::string fullKey = GetCubeMapResourceStringKey(cubeMapDescription);
	ResourceID id = GetAndAddIDFromString(fullKey);
	auto [pTexture, isNewTexture] = AddResource<CubeMapResource>(id, Resource::Type::CUBE_MAP);

	// Only load the texture if it has not been loaded.
	if (isNewTexture)
	{
		ImageResource* pImageResources[6];
		for (uint32 i = 0; i < 6; i++)
		{
			if (cubeMapDescription.EmptyInitialization)
			{
				pTexture->ImageHandlers[i]	= NULL_RESOURCE;
				pImageResources[i]			= nullptr;
			}
			else
			{
				auto [pImage, imageId]		= LoadImageResource(cubeMapDescription.ImageDescs[i]);
				pImageResources[i]			= pImage;
				pTexture->ImageHandlers[i]	= pImage->key;
			}
		}

		{
			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width				= cubeMapDescription.EmptyInitialization ? cubeMapDescription.Width		: pImageResources[0]->Width;
			textureDesc.Height				= cubeMapDescription.EmptyInitialization ? cubeMapDescription.Height	: pImageResources[0]->Height;
			textureDesc.Format				= cubeMapDescription.EmptyInitialization ? cubeMapDescription.Format	: pImageResources[0]->Format;
			textureDesc.MipLevels			= 1;
			textureDesc.ArraySize			= 6;
			textureDesc.SampleDesc.Count	= 1;
			textureDesc.SampleDesc.Quality	= 0;
			textureDesc.Usage				= (cubeMapDescription.GenerateMipmaps || cubeMapDescription.EmptyInitialization) ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
			textureDesc.CPUAccessFlags		= 0;
			textureDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
			textureDesc.MiscFlags			= D3D11_RESOURCE_MISC_TEXTURECUBE;

			if (textureDesc.Usage == D3D11_USAGE_DEFAULT)
				textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;

			if (cubeMapDescription.GenerateMipmaps)
			{
				textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
				textureDesc.MipLevels = (uint32)glm::ceil(glm::max(glm::log2(glm::min((float)textureDesc.Width, (float)textureDesc.Height)), 1.f));
			}

			pTexture->NumMipLevels = textureDesc.MipLevels;

			uint32 pixelSize = RenderUtils::GetSizeOfFormat(textureDesc.Format);

			std::vector<D3D11_SUBRESOURCE_DATA> subData;
			if (!cubeMapDescription.EmptyInitialization)
			{
				if (cubeMapDescription.GenerateMipmaps)
				{
					for (uint32 i = 0; i < 6; i++)
					{
						uint32 width = pImageResources[i]->Width;
						for (uint32 mip = 0; mip < textureDesc.MipLevels; mip++)
						{
							D3D11_SUBRESOURCE_DATA data = {};
							data.pSysMem = pImageResources[i]->Data.data();
							data.SysMemPitch = width * pixelSize;
							data.SysMemSlicePitch = 0;
							subData.push_back(data);

							width /= 2;
						}
					}
				}
				else
				{
					for (uint32 i = 0; i < 6; i++)
					{
						D3D11_SUBRESOURCE_DATA data = {};
						data.pSysMem = pImageResources[i]->Data.data();
						data.SysMemPitch = pImageResources[i]->Width * pixelSize;
						data.SysMemSlicePitch = 0; // This is not used, only used for 3D textures!
						subData.push_back(data);
					}
				}
			}

			HRESULT result = RenderAPI::Get()->GetDevice()->CreateTexture2D(&textureDesc, cubeMapDescription.EmptyInitialization ? nullptr : subData.data(), &pTexture->pTexture);
			RS_D311_ASSERT_CHECK(result, "Failed to create cube map texture!");

			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = textureDesc.Format;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.TextureCube.MipLevels = cubeMapDescription.GenerateMipmaps ? -1 : 1;
				srvDesc.TextureCube.MostDetailedMip = 0;
				result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pTexture->pTexture, &srvDesc, &pTexture->pTextureSRV);
				RS_D311_ASSERT_CHECK(result, "Failed to create cube map texture RSV!");
			}

			if (!cubeMapDescription.EmptyInitialization && cubeMapDescription.GenerateMipmaps)
				GenerateCubeMapMipmaps(pTexture);
			else
			{
				pTexture->DebugMipmapSRVs.resize(6);
				for (uint32 side = 0; side < 6; side++)
				{
					pTexture->DebugMipmapSRVs[side].resize(1);
					D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Format = textureDesc.Format;
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					srvDesc.Texture2DArray.MipLevels = 1;
					srvDesc.Texture2DArray.MostDetailedMip = 0;
					srvDesc.Texture2DArray.ArraySize = 1;
					srvDesc.Texture2DArray.FirstArraySlice = side;
					result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pTexture->pTexture, &srvDesc, &pTexture->DebugMipmapSRVs[side][0]);
					if (FAILED(result))
						LOG_WARNING("Failed to create debug texture SRV for one of the sides on the cubemap!");
				}
			}
		}
	}

	return { pTexture, id };
}

CubeMapResource* ResourceManager::LoadCubeMapResource(ResourceID id)
{
	CubeMapResource* pTexture = GetResource<CubeMapResource>(id);
	if (pTexture)
	{
		pTexture->AddRef();
		UpdateStats(pTexture, true);
		for (uint32 i = 0; i < 6; i++)
			LoadImageResource(pTexture->ImageHandlers[i]);
	}
	return pTexture;
}

void ResourceManager::GenerateMipmaps(Resource* pResource)
{
	switch (pResource->type)
	{
	case Resource::Type::TEXTURE:
		GenerateTextureMipmaps(dynamic_cast<TextureResource*>(pResource));
		break;
	case Resource::Type::CUBE_MAP:
		GenerateCubeMapMipmaps(dynamic_cast<CubeMapResource*>(pResource));
		break;
	case Resource::Type::MATERIAL:
		GenerateMaterialMipmaps(dynamic_cast<MaterialResource*>(pResource));
		break;
	case Resource::Type::MODEL:
		GenerateModelMipmaps(dynamic_cast<ModelResource*>(pResource));
		break;
	default:
		break;
	}
}

std::pair<ModelResource*, ResourceID> ResourceManager::LoadModelResource(ModelLoadDesc& modelDescription)
{
	std::string fullKey = modelDescription.FilePath + Resource::TypeToString(Resource::Type::MODEL);
	ResourceID id = GetAndAddIDFromString(fullKey);
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

ModelResource* ResourceManager::LoadModelResource(ResourceID id)
{
	ModelResource* pModel = GetResource<ModelResource>(id);
	// TODO: Recursive add ref to each mesh's material handler!
	if(pModel)
		pModel->AddRef();
	UpdateStats(pModel, true);
	return pModel;
}

bool ResourceManager::HasResource(ResourceID id) const
{
	return m_IDToResourceMap.find(id) != m_IDToResourceMap.end();
}

ResourceID ResourceManager::GetIDFromString(const std::string& str) const
{
	auto it = m_StringToResourceIDMap.find(str);
	if (it == m_StringToResourceIDMap.end())
		return 0;
	return it->second;
}

bool ResourceManager::AddStringToIDAssociation(const std::string& str, ResourceID id)
{
	if (HasResource(id))
	{
		m_StringToResourceIDMap[str] = id;
		return true;
	}
	return false;
}

void ResourceManager::FreeResource(Resource* pResource)
{
	pResource->RemoveRef();
	UpdateStats(pResource, false);
	if (pResource->GetRefCount() == 0)
	{
		// Remove resource if it is the last pointer.
		if (RemoveResource(pResource, false))
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
	std::vector<ResourceID> resourceIDs;
	for (auto [id, pResource] : m_IDToResourceMap)
		resourceIDs.push_back(id);
	stats.ResourceIDs = resourceIDs;
	return stats;
}

std::string ResourceManager::GetResourceName(ResourceID id)
{
	for(auto& nameEntry : m_StringToResourceIDMap)
		if (nameEntry.second == id)
			return nameEntry.first;

	LOG_WARNING("Could not fetch name from resource id, resource does not exist or does not have a name!");
	return "";
}

bool ResourceManager::RemoveResource(Resource* pResource, bool fullRemoval)
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
			FreeImage(pImage, fullRemoval);
		}
		break;
		case RS::Resource::Type::TEXTURE:
		{
			TextureResource* pTexture = dynamic_cast<TextureResource*>(pResource);
			FreeTexture(pTexture, fullRemoval);
		}
		break;
		case RS::Resource::Type::CUBE_MAP:
		{
			CubeMapResource* pTexture = dynamic_cast<CubeMapResource*>(pResource);
			FreeCubeMap(pTexture, fullRemoval);
		}
		break;
		case RS::Resource::Type::SAMPLER:
		{
			SamplerResource* pSampler = dynamic_cast<SamplerResource*>(pResource);
			FreeSampler(pSampler, fullRemoval);
		}
		break;
		case RS::Resource::Type::MODEL:
		{
			ModelResource* pModel = dynamic_cast<ModelResource*>(pResource);
			FreeModelRecursive(pModel, fullRemoval);
		}
		break;
		case RS::Resource::Type::MATERIAL:
		{
			MaterialResource* pMaterial = dynamic_cast<MaterialResource*>(pResource);
			FreeMaterial(pMaterial, fullRemoval);
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

void ResourceManager::FreeImage(ImageResource* pImage, bool fullRemoval)
{
	RS_UNREFERENCED_VARIABLE(fullRemoval);

	if (pImage)
	{
		pImage->Data.clear();
		pImage->Width	= 0;
		pImage->Height	= 0;
		pImage->Format	= DXGI_FORMAT_UNKNOWN;
	}
	else
		LOG_WARNING("Trying to free an image pointer which is NULL!");
}

void ResourceManager::FreeSampler(SamplerResource* pSampler, bool fullRemoval)
{
	RS_UNREFERENCED_VARIABLE(fullRemoval);

	if (pSampler->pSampler)
	{
		pSampler->pSampler->Release();
		pSampler->pSampler = nullptr;
	}
}

void ResourceManager::FreeTexture(TextureResource* pTexture, bool fullRemoval)
{
	// This will only release it, if it is the last handler.
	ImageResource* pImage = GetResource<ImageResource>(pTexture->ImageHandler);
	if (pImage)
		FreeImage(pImage, fullRemoval);
	else if(fullRemoval == false)
		LOG_ERROR("Trying to free a texture resource {}, without an image resource!", pTexture->key);
	pTexture->ImageHandler = 0;

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

	for (auto& mipSRV : pTexture->DebugMipmapSRVs)
	{
		if (mipSRV)
			mipSRV->Release();
	}
	pTexture->DebugMipmapSRVs.clear();
}

void ResourceManager::FreeCubeMap(CubeMapResource* pTexture, bool fullRemoval)
{
	// This will only release it, if it is the last handler.
	for (uint32 i = 0; i < 6; i++)
	{
		if (pTexture->ImageHandlers[i] != NULL_RESOURCE)
		{
			ImageResource* pImage = GetResource<ImageResource>(pTexture->ImageHandlers[i]);
			if (pImage)
				FreeImage(pImage, fullRemoval);
			else if (fullRemoval == false)
				LOG_ERROR("Trying to free a cube map resource {}, without one or more image resources!", pTexture->key);
			pTexture->ImageHandlers[0] = 0;
		}

		if (pTexture->DebugMipmapSRVs.size() == 6)
		{
			for (auto& mipSRV : pTexture->DebugMipmapSRVs[i])
			{
				if (mipSRV)
					mipSRV->Release();
			}
		}
	}
	pTexture->DebugMipmapSRVs.clear();

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
}

void ResourceManager::FreeMaterial(MaterialResource* pMaterial, bool fullRemoval)
{
	auto FreeTex = [&](ResourceID& textureID)->bool 
	{
		TextureResource* pTexture = GetResource<TextureResource>(textureID);
		if (pTexture)
		{
			FreeTexture(pTexture, fullRemoval);
			textureID = 0;
			return true;
		}
		else if (fullRemoval == false)
		{
			textureID = 0;
			return false;
		}
		else
		{
			textureID = 0;
			return true;
		}
	};

	if (!FreeTex(pMaterial->AlbedoTextureHandler))
		LOG_ERROR("Trying to free a material resource {} without an albedo texture resource!", pMaterial->key);

	if (!FreeTex(pMaterial->NormalTextureHandler))
		LOG_ERROR("Trying to free a material resource {} without a normal texture resource!", pMaterial->key);

	if (!FreeTex(pMaterial->AOTextureHandler))
		LOG_ERROR("Trying to free a material resource {} without an ambient occlusion texture resource!", pMaterial->key);

	if (!FreeTex(pMaterial->MetallicTextureHandler))
		LOG_ERROR("Trying to free a material resource {} without a metallic texture resource!", pMaterial->key);

	if (!FreeTex(pMaterial->RoughnessTextureHandler))
		LOG_ERROR("Trying to free a material resource {} without a roughness texture resource!", pMaterial->key);

	if (!FreeTex(pMaterial->MetallicRoughnessTextureHandler))
		LOG_ERROR("Trying to free a material resource {} without a combined metallic-roughness texture resource!", pMaterial->key);

	if (pMaterial->pConstantBuffer)
	{
		pMaterial->pConstantBuffer->Release();
		pMaterial->pConstantBuffer = nullptr;
	}
}

void ResourceManager::FreeModelRecursive(ModelResource* pModel, bool fullRemoval)
{
	pModel->Transform = glm::mat4(1.f);
	
	for (MeshObject& mesh : pModel->Meshes)
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
		FreeModelRecursive(&model, fullRemoval);

	pModel->Children.clear();
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

std::string	ResourceManager::GetImageResourceStringKey(ImageLoadDesc& imageDescription)
{
	if (imageDescription.Name.empty())
	{
		if (imageDescription.IsFromFile)
		{
			imageDescription.Name = imageDescription.File.Path;
			LOG_WARNING("The image created from file was not given a name! Using the path as the name.");
		}
		else
		{
			LOG_ERROR("The image created from memory was not given a name!");
		}
	}
	return imageDescription.Name + "." + 
		std::to_string(imageDescription.IsFromFile) + "." +
		Resource::TypeToString(Resource::Type::IMAGE);
}

std::string ResourceManager::GetSamplerResourceStringKey(SamplerLoadDesc& samplerDescription)
{
	// TODO: Make this better! Try to compress the key and add all attributes which identify this resource!
	std::string key = std::to_string(samplerDescription.Filter) + "." +
		std::to_string(samplerDescription.AddressU) + "." +
		std::to_string(samplerDescription.AddressV) + "." +
		std::to_string(samplerDescription.AddressW) + "." +
		std::to_string(samplerDescription.ComparisonFunc) + "." +
		Resource::TypeToString(Resource::Type::SAMPLER);

	return key;
}

std::string ResourceManager::GetTextureResourceStringKey(TextureLoadDesc& textureDescription)
{
	if (textureDescription.ImageDesc.Name.empty())
	{
		if (textureDescription.ImageDesc.IsFromFile)
		{
			textureDescription.ImageDesc.Name = textureDescription.ImageDesc.File.Path;
			LOG_WARNING("The texture created from file was not given a name! Using the path as the name.");
		}
		else
		{
			LOG_ERROR("The texture created from memory was not given a name!");
		}
	}
	return textureDescription.ImageDesc.Name + "." + 
		std::to_string(textureDescription.ImageDesc.IsFromFile) + "." +
		Resource::TypeToString(Resource::Type::TEXTURE);
}

std::string ResourceManager::GetCubeMapResourceStringKey(CubeMapLoadDesc& cubeMapDescription)
{
	if(cubeMapDescription.ImageDescs == nullptr)
		LOG_ERROR("The cube map created from file did not have any image descriptions!");

	if (cubeMapDescription.ImageDescs[0].Name.empty())
	{
		if (cubeMapDescription.ImageDescs[0].IsFromFile)
		{
			cubeMapDescription.ImageDescs[0].Name = cubeMapDescription.ImageDescs[0].File.Path;
			LOG_WARNING("The cube map created from file was not given a name! Using the path as the name.");
		}
		else
		{
			LOG_ERROR("The cube map created from memory was not given a name!");
		}
	}
	return cubeMapDescription.ImageDescs[0].Name + "." +
		std::to_string(cubeMapDescription.ImageDescs[0].IsFromFile) + "." +
		Resource::TypeToString(Resource::Type::CUBE_MAP);
}

void ResourceManager::GenerateTextureMipmaps(TextureResource* pResource)
{
	if (pResource->pTexture == nullptr)
	{
		LOG_WARNING("Failed generating mipmaps, texture pointer has not been created!");
		return;
	}
	
	D3D11_TEXTURE2D_DESC textureDesc = {};
	pResource->pTexture->GetDesc(&textureDesc);
	pResource->NumMipLevels = (uint32)glm::ceil(glm::max(glm::log2(glm::min((float)textureDesc.Width, (float)textureDesc.Height)), 1.f));
	
	RenderAPI::Get()->GetDeviceContext()->GenerateMips(pResource->pTextureSRV);

	// Debug SRVs for each mip level.
	for (auto& srv : pResource->DebugMipmapSRVs)
	{
		srv->Release();
		srv = nullptr;
	}

	if (pResource->NumMipLevels > 1)
		pResource->DebugMipmapSRVs.resize(pResource->NumMipLevels - 1);
	for (uint32 mip = 1; mip < pResource->NumMipLevels; mip++)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = mip;
		HRESULT result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pResource->pTexture, &srvDesc, &pResource->DebugMipmapSRVs[mip - 1]);
		if (FAILED(result))
		{
			LOG_WARNING("Failed to create debug texture SRV for one of the mip levels in the texture when generating mipmaps!");
			return;
		}
	}
}

void ResourceManager::GenerateCubeMapMipmaps(CubeMapResource* pResource)
{
	if (pResource->pTexture == nullptr)
	{
		LOG_WARNING("Failed generating mipmaps, texture pointer has not been created!");
		return;
	}

	D3D11_TEXTURE2D_DESC textureDesc = {};
	pResource->pTexture->GetDesc(&textureDesc);

	RenderAPI::Get()->GetDeviceContext()->GenerateMips(pResource->pTextureSRV);

	// Debug SRVs for each side of the cube and for each mip level.
	for (auto& srvs : pResource->DebugMipmapSRVs)
	{
		for (auto& srv : srvs)
		{
			srv->Release();
			srv = nullptr;
		}
	}

	pResource->DebugMipmapSRVs.resize(6);
	for (uint32 side = 0; side < 6; side++)
	{
		pResource->DebugMipmapSRVs[side].resize(pResource->NumMipLevels);
		for (uint32 mip = 0; mip < pResource->NumMipLevels; mip++)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = textureDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.MipLevels = 1;
			srvDesc.Texture2DArray.MostDetailedMip = mip;
			srvDesc.Texture2DArray.ArraySize = 1;
			srvDesc.Texture2DArray.FirstArraySlice = side;
			HRESULT result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pResource->pTexture, &srvDesc, &pResource->DebugMipmapSRVs[side][mip]);
			if (FAILED(result))
			{
				LOG_WARNING("Failed to create debug texture SRV for one of the sides on the cube map when generating mipmaps!");
				return;
			}
		}
	}
}

void ResourceManager::GenerateMaterialMipmaps(MaterialResource* pResource)
{
	TextureResource* pTexture = nullptr;
	if (pResource->AlbedoTextureHandler != NULL_RESOURCE)
	{
		pTexture = GetResource<TextureResource>(pResource->AlbedoTextureHandler);
		GenerateTextureMipmaps(pTexture);
	}

	if (pResource->AOTextureHandler != NULL_RESOURCE)
	{
		pTexture = GetResource<TextureResource>(pResource->AOTextureHandler);
		GenerateTextureMipmaps(pTexture);
	}

	if (pResource->NormalTextureHandler != NULL_RESOURCE)
	{
		pTexture = GetResource<TextureResource>(pResource->NormalTextureHandler);
		GenerateTextureMipmaps(pTexture);
	}

	if (pResource->MetallicTextureHandler != NULL_RESOURCE)
	{
		pTexture = GetResource<TextureResource>(pResource->MetallicTextureHandler);
		GenerateTextureMipmaps(pTexture);
	}

	if (pResource->RoughnessTextureHandler != NULL_RESOURCE)
	{
		pTexture = GetResource<TextureResource>(pResource->RoughnessTextureHandler);
		GenerateTextureMipmaps(pTexture);
	}

	if (pResource->MetallicRoughnessTextureHandler != NULL_RESOURCE)
	{
		pTexture = GetResource<TextureResource>(pResource->MetallicRoughnessTextureHandler);
		GenerateTextureMipmaps(pTexture);
	}
}

void ResourceManager::GenerateModelMipmaps(ModelResource* pResource)
{
	for (MeshObject& mesh : pResource->Meshes)
	{
		if (mesh.MaterialHandler != NULL_RESOURCE)
		{
			MaterialResource* pMaterial = GetResource<MaterialResource>(mesh.MaterialHandler);
			GenerateMaterialMipmaps(pMaterial);
		}
	}

	for (ModelResource& child : pResource->Children)
		GenerateModelMipmaps(&child);
}

ResourceID ResourceManager::GetAndAddIDFromString(const std::string& key)
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
	// GetNextID can never generate a NULL_RESOURCE.
	static ResourceID s_Generator = NULL_RESOURCE;
	return ++s_Generator;
}
