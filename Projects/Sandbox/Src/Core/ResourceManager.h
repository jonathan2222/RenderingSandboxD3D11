#pragma once

#include "Renderer/RenderAPI.h"
#include "Resources/RefObject.h"
#include "Resources/Resources.h"

#include "Core/ResourceManagerDefines.h"
#include "Core/ResourceInspector.h"

namespace RS
{
	class ResourceManager
	{
	public:
		friend class ResourceInspector;
		struct Stats
		{
			std::unordered_map<Resource::Type, uint32>*		pTypeResourcesRefCount	= nullptr;
			std::unordered_map<ResourceID, uint32>*			pResourcesRefCount		= nullptr;
			std::unordered_map<std::string, ResourceID>*	pStringToResourceIDMap	= nullptr;
			std::vector<ResourceID>							ResourceIDs;
		};

	public:
		RS_DEFAULT_ABSTRACT_CLASS(ResourceManager);

		static std::shared_ptr<ResourceManager> Get();

		void Init();
		void Release();

		/*
			Load a image either from a file or memory.
			Arguments:
				* ImageLoadDesc: All information to load the image, this can either be a pointer to data or a file path.
								 However, The image need a name!
			Returns:
				A pointer to the image structure and the resource ID.
		*/
		std::pair<ImageResource*, ResourceID> LoadImageResource(ImageLoadDesc& imageDescription);

		/*
		* This add a referense to the resource before it is returned.
		* Returns a pointer to the resource, nullptr if it was not found.
		*/
		ImageResource* LoadImageResource(ResourceID id);

		/*
			Load a texture with the use of the LoadImageResource function.
			Arguments:
				* TextureLoadDesc: All information to load the texture, this can either be a pointer to data or a file path.
									However, The texture need a name!
			Returns:
				A pointer to the texture structure and the resource ID.
		*/
		std::pair<TextureResource*, ResourceID> LoadTextureResource(TextureLoadDesc& textureDescription);
		
		/*
		* This add a referense to the resource before it is returned.
		* Returns a pointer to the resource, nullptr if it was not found.
		*/
		TextureResource* LoadTextureResource(ResourceID id);

		/*
			Load a model from a file path.
			Arguments:
				* ModelLoadDesc: All information to load the model.
			Returns:
				A pointer to the model structure and the resource ID.
		*/
		std::pair<ModelResource*, ResourceID> LoadModelResource(ModelLoadDesc& modelDescription);

		/*
		* This add a referense to the resource before it is returned.
		* Returns a pointer to the resource, nullptr if it was not found.
		*/
		ModelResource* LoadModelResource(ResourceID id);

		/*
		* Creates a new resource and adds a referense to it.
		* Returns the empty resource and its ID.
		* !Caution: Do not forget to load all the necessary resource handlers which this resource should point to!
		*/
		template<typename ResourceT>
		std::pair<ResourceT*, ResourceID> AddResource(Resource::Type type);

		/*
		* Returns a pointer of a resource from a handler, nullptr if it was not found.
		* This does not add a referens ot the resource!
		*/
		template<typename ResourceT>
		ResourceT* GetResource(ResourceID id) const;

		/*
		* Check if the resource exists in the system.
		*/
		bool HasResource(ResourceID id) const;

		/*
		* Returns the ID associated with a string.
		* If the string has no association or if the resource does not exists, return an ID of 0.
		*/
		ResourceID GetIDFromString(const std::string& str) const;

		/*
		* This will associate the string to a specified ID.
		* If the ID does not exist, it will return false else true.
		* If the ID already has an association, it will be overwritten.
		*/
		bool AddStringToIDAssociation(const std::string& str, ResourceID id);

		/*
		* Give the resource back to the system. If it was the last reference, it will destroy it.
		*/
		void FreeResource(Resource* pResource);

		Stats GetStats();

		// Default textures
		ResourceID	DefaultTextureOnePixelWhite		= 0;
		ResourceID	DefaultTextureOnePixelBlack		= 0;
		ResourceID	DefaultTextureOnePixelNormal	= 0;
	private:
		/*
		* Will add a new resource if it does not exist, else it will return the already existing resource.
		*/
		template<typename ResourceT>
		std::pair<ResourceT*, bool> AddResource(ResourceID key, Resource::Type type);

		void LoadImageFromFile(ImageResource*& outImage, ImageLoadDesc& imageDescription);
		void LoadImageFromMemory(ImageResource*& outImage, ImageLoadDesc& imageDescription);

		/*
		* Remove data from a single resource. The resource is still in the system!
		* FullRemoval: If true, the function will not give warnings if some resources are missing.
		*			This is good when the destructor need to remove all resources and the order can differ.
		*/
		bool RemoveResource(Resource* pResource, bool fullRemoval);

		/*
			Free image data and deallocate the image structure.
			Arguments:
				* image: A pointer to the image structure.
		*/
		void FreeImage(ImageResource* pImage, bool fullRemoval);
		void FreeTexture(TextureResource* pTexture, bool fullRemoval);
		void FreeMaterial(MaterialResource* pMaterial, bool fullRemoval);
		void FreeModelRecursive(ModelResource* pModel, bool fullRemoval);

		DXGI_FORMAT GetFormatFromChannelCount(int nChannels) const;

		void UpdateStats(Resource* pResrouce, bool add);

		std::string GetImageResourceStringKey(ImageLoadDesc& imageDescription);
		std::string GetTextureResourceStringKey(TextureLoadDesc& textureDescription);

		/*
		* This will return the ID associated with a string.
		* If the string has no association or if the resource does not exists, return an new generated ID.
		*/
		ResourceID GetAndAddIDFromString(const std::string& key);

		ResourceID GetNextID() const;

	private:
		std::unordered_map<std::string, ResourceID> m_StringToResourceIDMap;
		std::unordered_map<ResourceID, Resource*>	m_IDToResourceMap;

		// Stats
		std::unordered_map<Resource::Type, uint32>	m_TypeResourcesRefCount;
		std::unordered_map<ResourceID, uint32>		m_ResourcesRefCount;
	};

	template<typename ResourceT>
	inline std::pair<ResourceT*, ResourceID> ResourceManager::AddResource(Resource::Type type)
	{
		ResourceID id = GetNextID();
		ResourceT* pResource = new ResourceT();
		pResource->type = type;
		pResource->key = id;
		m_IDToResourceMap[id] = pResource;
		UpdateStats(pResource, true);
		return { pResource, id };
	}

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