#pragma once

#include "Renderer/RenderAPI.h"
#include "Resources/RefObject.h"
#include "Resources/Resources.h"

namespace RS
{
	class ResourceManager
	{
	public:
		struct Stats
		{
			std::unordered_map<Resource::Type, uint32>* pResourcesRefCount = nullptr;
		};

	public:
		RS_DEFAULT_ABSTRACT_CLASS(ResourceManager);

		static std::shared_ptr<ResourceManager> Get();

		void Init();
		void Release();

		/*
				Load a texture from memory.
				Arguments:
					* fileName: The path to the file, including the name of the image file with its extention.
					* nChannels: How many channels it will use. It can be 0, 1, 2, 3 or 4.
						If it is set to 0, it will use the same number of channels as the image file on disk.
				Returns:
					A pointer to the texture structure.
		*/
		TextureResource* LoadTextureResource(const std::string& fileName, int nChannels);

		/*
		* Give the resource back to the system. If it was the last reference, it will destroy it.
		*/
		void FreeResource(Resource* pResource);

		Stats GetStats();

	private:
		/*
		* Will add a new resource if it does not exist, else it will return the already existing resource.
		*/
		std::pair<Resource*, bool> AddResource(const std::string& key, Resource::Type type);
		bool RemoveResource(Resource* pResource);

		/*
			Free image data and deallocate the image structure.
			Arguments:
				* image: A pointer to the image structure.
		*/
		void FreeTexture(TextureResource* pTexture);

		DXGI_FORMAT GetFormatFromChannelCount(int nChannels) const;

		void UpdateStats(Resource* pResrouce);

	private:
		std::unordered_map<std::string, Resource*> m_Resources;

		// Stats
		std::unordered_map<Resource::Type, uint32> m_ResourcesRefCount;
	};
}