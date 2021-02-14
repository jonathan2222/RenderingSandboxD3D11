#pragma once

#include "Renderer/RenderAPI.h"

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
		void EndScene();

		ID3D11RenderTargetView* GetRenderTarget();

	private:
		void CreateRTV();
		void ClearRTV();
		void CreateDepthBuffer(DisplayDescription& displayDescriptor);
		void CreateDepthStencilState();
		void CreateDepthStencilView();
		void CreateRasterizer();
		void CreateAndSetViewport(float width, float height);

	private:
		ID3D11Device*				m_pDevice				= nullptr;
		ID3D11DeviceContext*		m_pContext				= nullptr;
		IDXGISwapChain1*			m_pSwapChain			= nullptr;

		// Color buffer
		ID3D11RenderTargetView*		m_pRenderTargetView		= nullptr;

		// Depth and Stencil buffer
		ID3D11Texture2D*			m_pDepthStencilBuffer	= nullptr;
		ID3D11DepthStencilState*	m_pDepthStencilState	= nullptr;
		ID3D11DepthStencilView*		m_pDepthStencilView		= nullptr;

		// Rasterizer
		ID3D11RasterizerState*		m_pRasterizerState		= nullptr;
	};
}