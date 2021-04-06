#pragma once

#include "Renderer/RenderAPI.h"
#include "Renderer/PipelineDefines.h"

namespace RS
{
	class Pipeline
	{
	public:
		void Init();
		void Release();

		/*
		* Resizes the depth buffer. And sets the rtvs again. 
		* The RTVs need to be resized before this is called!
		*/
		void Resize(uint32 width, uint32 height);

		/*
		* Binds the raster sate, depth/stencil views and the RTVs.
		* If one of the stats/views was not set by the pipeline, it will use the renderer's default objects.
		*/
		void Bind(BindType bindType);

		/*
		* Only bind the Depth stencil state
		* If not the Depth stencil state was set, use the renderer's default state.
		*/
		void BindDepthStencilState();

		/*
		* Only bind the Rasterizer state
		* If not the Rasterizer state was set, use the renderer's default state.
		*/
		void BindRasterState();

		/*
		* Only bind the depth/stencil view and all RTVs
		* If not the depth/stencil view was set, use the renderer's default view. 
		* And if the RTVs was not set, use the renderer's defaut RTVs.
		*/
		void BindDepthAndRTVs(BindType bindType);

		/*
		* Set the depth stencil view and bind it to the pipeline.
		*/
		void SetDepthStencilView(D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilView);

		/*
		* Set the depth stencil state and depth stencil buffer and bind the state to the pipeline.
		*/
		void SetDepthState(D3D11_DEPTH_STENCIL_DESC depthStencilDesc, D3D11_TEXTURE2D_DESC depthBufferDesc);

		/*
		* Set the rasterizer state and bind it to the pipeline.
		*/
		void SetRasterState(D3D11_RASTERIZER_DESC rasterizerDesc);

		/*
		* Save the RTVs to the pipeline. This will not call the set function!
		*/
		void SetRenderTargetViews(std::vector<ID3D11RenderTargetView*> rtvs);

		/*
		* Save one RTV to the pipeline. This will clear the previous RTVs.
		* This will not call the set function!
		*/
		void SetRenderTargetView(ID3D11RenderTargetView* rtv);

		void SetViewport(float x, float y, float width, float height);

		ID3D11DepthStencilView* GetDepthStencilView();
		ID3D11RenderTargetView* GetRenderTargetView(uint32 index);
		std::vector<ID3D11RenderTargetView*>& GetRenderTargetViews();
		ID3D11RasterizerState* GetRasterState();
		ID3D11DepthStencilState* GetDepthStencilState();

		uint32 GetID() const;

	private:
		struct DepthStencilSave
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC	depthStencilViewDesc;
			D3D11_DEPTH_STENCIL_DESC		depthStencilDesc;
			D3D11_TEXTURE2D_DESC			depthBufferDesc;
		};

		struct Binded
		{
			uint32 RasterizerID			= 0;
			uint32 DepthStencilStateID	= 0;
			uint32 DepthStencilViewID	= 0;
			uint32 RTVsID				= 0;
		};

		void CreateDepthState();
		void CreateDepthStencilView();

		uint32 GenID();

	private:
		ID3D11Device*				m_pDevice				= nullptr;
		ID3D11DeviceContext*		m_pContext				= nullptr;

		// Rasterizer
		ID3D11RasterizerState*		m_pRasterizerState		= nullptr;

		std::vector<ID3D11RenderTargetView*> m_RenderTargetViews;

		// Depth and Stencil buffer
		ID3D11Texture2D*			m_pDepthStencilBuffer	= nullptr;
		ID3D11DepthStencilState*	m_pDepthStencilState	= nullptr;
		ID3D11DepthStencilView*		m_pDepthStencilView		= nullptr;

		DepthStencilSave			m_DepthStencilSave;

		// Used for binding.
		BindType					m_RTVAndDSVType					= BindType::BOTH;
		bool						m_ShouldBindRasterizerState		= true;
		bool						m_ShouldBindDepthStencilState	= true;
		bool						m_ShouldBindDepthStencilView	= true;
		bool						m_ShouldBindRTVs				= true;
		uint32						m_ID							= 0;
		inline static Binded		s_CurrentBindedStates;
	};
}