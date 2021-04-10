#pragma once

#include "Renderer/RenderAPI.h"

#include "Renderer/Pipeline.h"
#include "Renderer/Shader.h"

#include "Core/ResourceManager.h"

#include "Renderer/RenderDefines.h"

namespace RS
{
	struct DisplayDescription;
	class Renderer
	{
	public:
		RS_DEFAULT_ABSTRACT_CLASS(Renderer);

		static std::shared_ptr<Renderer> Get();

		void Init(DisplayDescription& displayDescriptor);
		void Release();

		void Resize(uint32 width, uint32 height);

		// TODO: Allow for saving the previous pipeline state and later set it back.
		void BeginScene(float r, float g, float b, float a);
		void BindWindowRTV();
		void Present();

		struct DebugInfo
		{
			bool	DrawAABBs	= false;
			uint32	ID			= 0;
			uint32	RenderMode	= 0; // 0: Normal rendering, 1: Albedo, 2: Normals, 3: AO, 4: Metallic, 5: Roughness, 6: Combined Metallic-Roughness
		};
		void Render(ModelResource& model, const glm::mat4& transform, DebugInfo debugInfo, RenderFlags flags);
		void RenderWithMaterial(ModelResource& model, const glm::mat4& transform, DebugInfo debugInfo);

		ID3D11RenderTargetView* GetRenderTarget();

		Pipeline* GetDefaultPipeline();

		// Useful function
		void ConvertTextureFormat(TextureResource* pTexture, DXGI_FORMAT newFormat);
		CubeMapResource* ConvertEquirectangularToCubemap(TextureResource* pTexture, uint32_t width, uint32_t height);
		CubeMapResource* CreateIrradianceMapFromEnvironmentMap(CubeMapResource* pEnvironmentMap, uint32_t width, uint32_t height);

	private:
		void CreateRTV();
		void ClearRTV();
		void CreateDepthStencilState(DisplayDescription& displayDescriptor);
		void CreateDepthStencilView();
		void CreateRasterizer();

		void InternalRender(ModelResource& model, const glm::mat4& transform, ID3D11DeviceContext* pContext, DebugInfo debugInfo, RenderFlags flags);
		void InternalRenderWithMaterial(ModelResource& model, const glm::mat4& transform, ID3D11DeviceContext* pContext, DebugInfo debugInfo);

		struct EquirectangularToCubemapFrameData
		{
			glm::mat4 View = glm::mat4(1.f);
			glm::mat4 Proj = glm::mat4(1.f);
		};

	private:
		ID3D11Device*						m_pDevice									= nullptr;
		ID3D11DeviceContext*				m_pContext									= nullptr;
		IDXGISwapChain1*					m_pSwapChain								= nullptr;

		// Color buffer
		ID3D11RenderTargetView*				m_pRenderTargetView							= nullptr;

		Pipeline							m_DefaultPipeline;

		Pipeline							m_TextureFormatConvertionPipeline;
		Shader								m_TextureFormatConvertionShader;
		ID3D11RenderTargetView*				m_TextureFormatConvertionRTV				= nullptr;

		Pipeline							m_EquirectangularToCubemapPipeline;
		Shader								m_EquirectangularToCubemapShader;
		ID3D11RenderTargetView*				m_EquirectangularToCubemapRTVs[6]			= { nullptr };
		ID3D11Buffer*						m_pEquirectangularToCubemapConstantBuffer	= nullptr;
		EquirectangularToCubemapFrameData	m_EquirectangularToCubemapFrameData;
		glm::mat4							m_EquirectangularToCubemapCaptureViews[6];

		Shader								m_IrradianceMapShader;
		ID3D11RenderTargetView*				m_IrradianceMapRTVs[6] = { nullptr };
	};
}