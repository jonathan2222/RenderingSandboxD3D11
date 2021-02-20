#pragma once

#include "Renderer/RenderAPI.h"

#include "Renderer/Pipeline.h"

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

		void BeginScene(float r, float g, float b, float a);
		void BindWindowRTV();
		void Present();

		ID3D11RenderTargetView* GetRenderTarget();

		Pipeline* GetDefaultPipeline();

	private:
		void CreateRTV();
		void ClearRTV();
		void CreateDepthStencilState(DisplayDescription& displayDescriptor);
		void CreateDepthStencilView();
		void CreateRasterizer();

	private:
		ID3D11Device*				m_pDevice				= nullptr;
		ID3D11DeviceContext*		m_pContext				= nullptr;
		IDXGISwapChain1*			m_pSwapChain			= nullptr;

		// Color buffer
		ID3D11RenderTargetView*		m_pRenderTargetView		= nullptr;

		Pipeline					m_DefaultPipeline;
	};
}