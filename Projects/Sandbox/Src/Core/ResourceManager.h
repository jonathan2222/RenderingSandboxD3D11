#pragma once

#include "Renderer/RenderAPI.h"
#include "Resources/RefObject.h"
#include "Resources/Resources.h"

#include "Core/ResourceManagerDefines.h"

namespace RS
{
	class ResourceManager
	{
	public:
		struct Stats
		{
			std::unordered_map<Resource::Type, uint32>*		pTypeResourcesRefCount	= nullptr;
			std::unordered_map<ResourceID, uint32>*			pResourcesRefCount		= nullptr;
			std::unordered_map<std::string, ResourceID>*	pStringToResourceIDMap	= nullptr;
		};

	public:
		RS_DEFAULT_ABSTRACT_CLASS(ResourceManager);

		static std::shared_ptr<ResourceManager> Get();

		void Init();
		void Release();

		/*
				Load a image from memory.
				Arguments:
					* fileName: The path to the file, including the name of the image file with its extention.
					* nChannels: How many channels it will use. It can be 0, 1, 2, 3 or 4.
						If it is set to 0, it will use the same number of channels as the image file on disk.
				Returns:
					A pointer to the texture structure.
		*/
		std::pair<ImageResource*, ResourceID> LoadImageResource(ImageLoadDesc& imageDescription);

		std::pair<TextureResource*, ResourceID> LoadTextureResource(TextureLoadDesc& textureDescription);
		
		std::pair<ModelResource*, ResourceID> LoadModelResource(ModelLoadDesc& modelDescription);

		/*
		* Returns a resource from a handler
		*/
		template<typename ResourceT>
		ResourceT* GetResource(ResourceID id) const;

		/*
		* Give the resource back to the system. If it was the last reference, it will destroy it.
		*/
		void FreeResource(Resource* pResource);

		Stats GetStats();

	private:
		/*
		* Will add a new resource if it does not exist, else it will return the already existing resource.
		*/
		template<typename ResourceT>
		std::pair<ResourceT*, bool> AddResource(ResourceID key, Resource::Type type);

		void LoadImageInternal(ImageResource*& outImage, ImageLoadDesc& imageDescription);

		bool RemoveResource(Resource* pResource);

		/*
			Free image data and deallocate the image structure.
			Arguments:
				* image: A pointer to the image structure.
		*/
		void FreeImage(ImageResource* pImage);
		void FreeTexture(TextureResource* pTexture);
		void FreeModelRecursive(ModelResource* pModel);

		DXGI_FORMAT GetFormatFromChannelCount(int nChannels) const;

		void UpdateStats(Resource* pResrouce, bool add);

		ResourceID GetIDFromString(const std::string& key);

		ResourceID GetNextID() const;

	private:
		std::unordered_map<std::string, ResourceID> m_StringToResourceIDMap;
		std::unordered_map<ResourceID, Resource*>	m_IDToResourceMap;

		// Stats
		std::unordered_map<Resource::Type, uint32>	m_TypeResourcesRefCount;
		std::unordered_map<ResourceID, uint32>		m_ResourcesRefCount;
	};

	template<typename ResourceT>
	inline ResourceT* ResourceManager::GetResource(ResourceID id) const
	{
		auto it = m_IDToResourceMap.find(id);
		if (it == m_IDToResourceMap.end())
			return nullptr;
		return dynamic_cast<ResourceT*>(it->second);
	}

	template<typename ResourceT>
	inline std::pair<ResourceT*, bool> ResourceManager::AddResource(ResourceID id, Resource::Type type)
	{
		bool isNew = false;
		ResourceT* pResource = GetResource<ResourceT>(id);

		if (pResource == nullptr)
		{
			pResource = new ResourceT();
			pResource->type = type;
			pResource->key = id;
			m_IDToResourceMap[id] = pResource;
			isNew = true;
		}

		pResource->AddRef();
		UpdateStats(pResource, true);
		return { pResource, isNew };
	}
}