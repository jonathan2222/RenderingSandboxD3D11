#include "PreCompiled.h"
#include "ResourceManager.h"

#include "Loaders/ModelLoader.h"
#include "Renderer/ImGuiRenderer.h"
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
			loadDesc.ImageDesc.IsFromFile	= false;
			loadDesc.ImageDesc.NumChannels	= ImageLoadDesc::Channels::RGBA;
			loadDesc.ImageDesc.Name			= "RS_WHITE_PIXEL";
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
			loadDesc.ImageDesc.IsFromFile	= false;
			loadDesc.ImageDesc.NumChannels	= ImageLoadDesc::Channels::RGBA;
			loadDesc.ImageDesc.Name			= "RS_BLACK_PIXEL";
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
			loadDesc.ImageDesc.IsFromFile	= false;
			loadDesc.ImageDesc.NumChannels	= ImageLoadDesc::Channels::RGBA;
			loadDesc.ImageDesc.Name			= "RS_NORMAL_PIXEL";
			auto [pTexture, ID] = LoadTextureResource(loadDesc);
			DefaultTextureOnePixelNormal = ID;
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
			data.pSysMem				= pImage->Data.data();
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

TextureResource* ResourceManager::LoadTextureResource(ResourceID id)
{
	TextureResource* pTexture = GetResource<TextureResource>(id);
	if (pTexture)
	{
		pTexture->AddRef();
		UpdateStats(pTexture, true);
		TextureResource* pImage = GetResource<TextureResource>(pTexture->ImageHandler);
		if (pImage)
		{
			pImage->AddRef();
			UpdateStats(pImage, true);
		}
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

	std::string path = imageDescription.File.Path;
	if(imageDescription.File.UseDefaultFolder)
		path = std::string(RS_TEXTURE_PATH) + imageDescription.File.Path;
	int width = 0, height = 0, channelCount = 0;
	uint8* pPixels = nullptr;
	if (nChannels < 0 || nChannels > 4)
	{
		outImage->Data.clear();
		LOG_WARNING("Unable to load image [{0}]: Requested number of channels is not supported! Requested {1}.", path.c_str(), nChannels);
	}
	else
	{
		pPixels = (uint8*)stbi_load(path.c_str(), &width, &height, &channelCount, nChannels);
		if (!pPixels)
			LOG_WARNING("Unable to load image [{0}]: File not found!", path.c_str());
		else
		{
			size_t size = (size_t)(width * height * (nChannels == 0 ? channelCount : nChannels));
			outImage->Data.resize(size, 0);
			memcpy(outImage->Data.data(), pPixels, size);
			stbi_image_free(pPixels);
			pPixels = nullptr;

			outImage->Format = GetFormatFromChannelCount(nChannels == 0 ? channelCount : nChannels);
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

	if (pTexture->pSampler)
	{
		pTexture->pSampler->Release();
		pTexture->pSampler = nullptr;
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
