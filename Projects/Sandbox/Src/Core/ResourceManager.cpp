#include "PreCompiled.h"
#include "ResourceManager.h"

#include "Loaders/ModelLoader.h"
#include "Renderer/ImGuiRenderer.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Renderer.h"
#include <unordered_set>

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
			LoadImageFromFile(pImage, imageDescription);
		else
			LoadImageFromMemory(pImage, imageDescription);
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
		auto [pImage, imageId] = LoadImageResource(textureDescription.ImageDesc);
		pTexture->ImageHandler = pImage->key;
		pTexture->Format = pImage->Format;

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

			if (textureDescription.GenerateMipmaps)
			{
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
				textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
				textureDesc.MipLevels = (uint32)glm::ceil(glm::max(glm::log2(glm::min((float)textureDesc.Width, (float)textureDesc.Height)), 1.f));
			}

			pTexture->UseAsRTV = textureDescription.UseAsRTV;
			if (pTexture->UseAsRTV)
			{
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			}

			pTexture->NumMipLevels = textureDesc.MipLevels;
			
			uint32 pixelSize = RenderUtils::GetSizeOfFormat(textureDesc.Format);
			std::vector<D3D11_SUBRESOURCE_DATA> subData;
			if (textureDescription.GenerateMipmaps)
			{
				uint32 width = pImage->Width;
				for (uint32 mip = 0; mip < textureDesc.MipLevels; mip++)
				{
					D3D11_SUBRESOURCE_DATA data = {};
					data.pSysMem = pImage->Data.data();
					data.SysMemPitch = width * pixelSize;
					data.SysMemSlicePitch = 0;
					subData.push_back(data);

					width /= 2;
				}
			}
			else
			{
				D3D11_SUBRESOURCE_DATA data = {};
				data.pSysMem = pImage->Data.data();
				data.SysMemPitch = pImage->Width * pixelSize;
				data.SysMemSlicePitch = pImage->Width * pImage->Height * pixelSize; // This is not used, only used for 3D textures!
				subData.push_back(data);
			}

			HRESULT result = RenderAPI::Get()->GetDevice()->CreateTexture2D(&textureDesc, subData.data(), &pTexture->pTexture);
			RS_D311_ASSERT_CHECK(result, "Failed to create texture!");

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format						= textureDesc.Format;
			srvDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip	= 0;
			srvDesc.Texture2D.MipLevels			= textureDescription.GenerateMipmaps ? -1 : 1;
			result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pTexture->pTexture, &srvDesc, &pTexture->pTextureSRV);
			RS_D311_ASSERT_CHECK(result, "Failed to create texture RSV!");

			if (textureDescription.GenerateMipmaps)
				RenderAPI::Get()->GetDeviceContext()->GenerateMips(pTexture->pTextureSRV);

			// Debug SRVs for each mip level.
			if(pTexture->NumMipLevels > 1)
				pTexture->DebugMipmapSRVs.resize(pTexture->NumMipLevels-1);
			for (uint32 mip = 1; mip < pTexture->NumMipLevels; mip++)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = textureDesc.Format;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = 1;
				srvDesc.Texture2D.MostDetailedMip = mip;
				result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pTexture->pTexture, &srvDesc, &pTexture->DebugMipmapSRVs[mip-1]);
				RS_D311_ASSERT_CHECK(result, "Failed to create debug texture SRV for one of the mip levels in the texture!");
			}
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
			auto [pImage, imageId]		= LoadImageResource(cubeMapDescription.ImageDescs[i]);
			pImageResources[i]			= pImage;
			pTexture->ImageHandlers[i]	= pImage->key;
		}

		{
			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = pImageResources[0]->Width;
			textureDesc.Height = pImageResources[0]->Height;
			textureDesc.Format = pImageResources[0]->Format;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 6;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = cubeMapDescription.GenerateMipmaps ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

			if (cubeMapDescription.GenerateMipmaps)
			{
				textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
				textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
				textureDesc.MipLevels = (uint32)glm::ceil(glm::max(glm::log2(glm::min((float)textureDesc.Width, (float)textureDesc.Height)), 1.f));
			}

			pTexture->NumMipLevels = textureDesc.MipLevels;

			uint32 pixelSize = RenderUtils::GetSizeOfFormat(textureDesc.Format);

			std::vector<D3D11_SUBRESOURCE_DATA> subData;
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

			HRESULT result = RenderAPI::Get()->GetDevice()->CreateTexture2D(&textureDesc, subData.data(), &pTexture->pTexture);
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

			if (cubeMapDescription.GenerateMipmaps)
				RenderAPI::Get()->GetDeviceContext()->GenerateMips(pTexture->pTextureSRV);

			// Debug SRVs for each side of the cube and for each mip level.
			pTexture->DebugMipmapSRVs.resize(6);
			for (uint32 side = 0; side < 6; side++)
			{
				pTexture->DebugMipmapSRVs[side].resize(pTexture->NumMipLevels);
				for (uint32 mip = 0; mip < pTexture->NumMipLevels; mip++)
				{
					D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Format = textureDesc.Format;
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					srvDesc.Texture2DArray.MipLevels = 1;
					srvDesc.Texture2DArray.MostDetailedMip = mip;
					srvDesc.Texture2DArray.ArraySize = 1;
					srvDesc.Texture2DArray.FirstArraySlice = side;
					result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pTexture->pTexture, &srvDesc, &pTexture->DebugMipmapSRVs[side][mip]);
					RS_D311_ASSERT_CHECK(result, "Failed to create debug texture SRV for one of the sides on the cube map!");
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

void ResourceManager::LoadImageFromFile(ImageResource*& outImage, ImageLoadDesc& imageDescription)
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
		std::string extension = imageDescription.File.Path.substr(dotPos+1);
		isHDR = extension == "hdr";
	}

	std::string path = imageDescription.File.Path;
	if(imageDescription.File.UseDefaultFolder)
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

void ResourceManager::LoadImageFromMemory(ImageResource*& outImage, ImageLoadDesc& imageDescription)
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
		LOG_WARNING("Unable to load image from memory: Data was nullptr!");
	}

	outImage->Width		= (unsigned int)width;
	outImage->Height	= (unsigned int)height;
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
		ImageResource* pImage = GetResource<ImageResource>(pTexture->ImageHandlers[i]);
		if (pImage)
			FreeImage(pImage, fullRemoval);
		else if (fullRemoval == false)
			LOG_ERROR("Trying to free a cube map resource {}, without one or more image resources!", pTexture->key);
		pTexture->ImageHandlers[0] = 0;

		for (auto& mipSRV : pTexture->DebugMipmapSRVs[i])
		{
			if (mipSRV)
				mipSRV->Release();
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

DXGI_FORMAT ResourceManager::GetFormatFromChannelCount(int nChannels) const
{
	switch (nChannels)
	{
	case 1: return DXGI_FORMAT_R8_UNORM; break;
	case 2: return DXGI_FORMAT_R8G8_UNORM; break;
	case 3:
		LOG_WARNING("A texture format with 3 channels are not supported!");
	case 4: return DXGI_FORMAT_R8G8B8A8_UNORM; break;
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
	static ResourceID s_Generator = 0;
	return ++s_Generator;
}
