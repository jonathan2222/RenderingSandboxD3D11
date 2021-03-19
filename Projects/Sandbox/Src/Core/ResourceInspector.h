#pragma once

#include "Renderer/ImGuiRenderer.h"
#include "Resources/Resources.h"

namespace RS
{
	class ResourceManager;
	class ResourceInspector
	{
	public:
		static void Init(ResourceManager* pResourceManager);
		static void Release();

		static void Draw();

	private:
		static void DrawSamplerResource(SamplerResource* pSampler);
		static void DrawTextureResource(TextureResource* pTexture);
		static void DrawCubeMapResource(CubeMapResource* pCubeMap);
		static void DrawImageResource(ImageResource* pImage);
		static void DrawMaterialResource(MaterialResource* pMaterial);
		static void DrawModelResource(ModelResource* pModel);
		static void DrawModelRecursive(ModelResource& model);
		static void DrawImGuiAABB(int index, const AABB& aabb);

		static std::string GetKeyStringFromID(ResourceID id);
		static void DrawTextureSRV(ID3D11ShaderResourceView* pTextureSRV, uint32 width, uint32 height);
		static void DrawCubeMap(CubeMapResource* pTexture, uint32 width, uint32 height);

	private:
		inline static ResourceManager* s_ResourceManager = nullptr;
	};
}