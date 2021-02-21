#include "PreCompiled.h"
#include "Pipeline.h"

#include "Renderer/Renderer.h"

using namespace RS;

void Pipeline::Init()
{
	// Fetch the device, device context from the DirectX API.
	m_pDevice = RenderAPI::Get()->GetDevice();
	m_pContext = RenderAPI::Get()->GetDeviceContext();

	m_ID = GenID();
}

void Pipeline::Release()
{
	if (m_pRasterizerState)
	{
		m_pRasterizerState->Release();
		m_pRasterizerState = nullptr;
	}

	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = nullptr;
	}

	if (m_pDepthStencilState)
	{
		m_pDepthStencilState->Release();
		m_pDepthStencilState = nullptr;
	}

	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
		m_pDepthStencilBuffer = nullptr;
	}
}

void Pipeline::Resize(uint32 width, uint32 height)
{
	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
		m_pDepthStencilBuffer = nullptr;
	}

	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = nullptr;
	}
	
	m_DepthStencilSave.depthBufferDesc.Width = width;
	m_DepthStencilSave.depthBufferDesc.Height = height;
	CreateDepthState();
	m_ShouldBindDepthStencilState = true;
	BindDepthStencilState();

	m_ShouldBindDepthStencilView = true;
	CreateDepthStencilView();

	m_ShouldBindRTVs = true;
	BindDepthAndRTVs();

	SetViewport(0.f, 0.f, (float)width, (float)height);
}

void Pipeline::Bind()
{
	BindDepthStencilState();
	BindDepthAndRTVs();
	BindRasterState();
}

void Pipeline::BindDepthStencilState()
{
	if (m_pDepthStencilState)
	{
		// Only bind depth stencil state if the current binded depth stencil state was not from the same pipeline or if it was updated.
		if (s_CurrentBindedStates.DepthStencilStateID != m_ID || m_ShouldBindDepthStencilState)
		{
			m_pContext->OMSetDepthStencilState(m_pDepthStencilState, 1);
			s_CurrentBindedStates.DepthStencilStateID = m_ID;
			m_ShouldBindDepthStencilState = false;
		}
	}
	else
	{
		// Only bind depth stencil state if the current binded depth stencil state was not from the same pipeline or if it was updated.
		Pipeline* pDefaultPipeline = Renderer::Get()->GetDefaultPipeline();
		ID3D11DepthStencilState* pDepthStencilState = pDefaultPipeline->GetDepthStencilState();
		if (s_CurrentBindedStates.DepthStencilStateID != pDefaultPipeline->GetID() || pDefaultPipeline->m_ShouldBindDepthStencilState)
		{
			m_pContext->OMSetDepthStencilState(pDepthStencilState, 1);
			s_CurrentBindedStates.DepthStencilStateID = pDefaultPipeline->GetID();
			pDefaultPipeline->m_ShouldBindDepthStencilState = false;
		}
	}
}

void Pipeline::BindRasterState()
{
	if (m_pRasterizerState)
	{
		// Only bind rasterizer state if the current binded rasterizer state was not from the same pipeline or if it was updated.
		if (s_CurrentBindedStates.RasterizerID != m_ID || m_ShouldBindRasterizerState)
		{
			m_pContext->RSSetState(m_pRasterizerState);
			s_CurrentBindedStates.RasterizerID = m_ID;
			m_ShouldBindRasterizerState = false;
		}
	}
	else
	{
		// Only bind rasterizer state if the current binded rasterizer state was not from the same pipeline or if it was updated.
		Pipeline* pDefaultPipeline = Renderer::Get()->GetDefaultPipeline();
		ID3D11RasterizerState* pRasterizerState = pDefaultPipeline->GetRasterState();
		if (s_CurrentBindedStates.RasterizerID != pDefaultPipeline->GetID() || pDefaultPipeline->m_ShouldBindRasterizerState)
		{
			m_pContext->RSSetState(pRasterizerState);
			s_CurrentBindedStates.RasterizerID = pDefaultPipeline->GetID();
			pDefaultPipeline->m_ShouldBindRasterizerState = false;
		}
	}
}

void Pipeline::BindDepthAndRTVs()
{
	Pipeline* pDefaultPipeline = Renderer::Get()->GetDefaultPipeline();
	if (m_RenderTargetViews.empty() == false)
	{
		if (m_pDepthStencilView)
		{
			// Only bind the RTVs if the current binded RTVs were not from the same pipeline or if they were updated.
			if (s_CurrentBindedStates.RTVsID != m_ID || s_CurrentBindedStates.DepthStencilViewID != m_ID 
				|| m_ShouldBindRTVs || m_ShouldBindDepthStencilView)
			{
				m_pContext->OMSetRenderTargets((UINT)m_RenderTargetViews.size(), m_RenderTargetViews.data(), m_pDepthStencilView);
				s_CurrentBindedStates.RTVsID				= m_ID;
				s_CurrentBindedStates.DepthStencilViewID	= m_ID;
				m_ShouldBindRTVs							= false;
				m_ShouldBindDepthStencilView				= false;
			}
		}
		else
		{
			// Only bind the RTVs if the current binded RTVs were not from the same pipeline or if they were updated.
			ID3D11DepthStencilView* pDefaultDepthStencilView = pDefaultPipeline->GetDepthStencilView();
			if (s_CurrentBindedStates.RTVsID != m_ID || s_CurrentBindedStates.DepthStencilViewID != pDefaultPipeline->GetID()
				|| m_ShouldBindRTVs || pDefaultPipeline->m_ShouldBindDepthStencilView)
			{
				m_pContext->OMSetRenderTargets((UINT)m_RenderTargetViews.size(), m_RenderTargetViews.data(), pDefaultDepthStencilView);
				s_CurrentBindedStates.RTVsID					= m_ID;
				s_CurrentBindedStates.DepthStencilViewID		= pDefaultPipeline->GetID();
				m_ShouldBindRTVs								= false;
				pDefaultPipeline->m_ShouldBindDepthStencilView	= false;
			}
		}
	}
	else if (m_pDepthStencilView)
	{
		// Only bind the RTVs if the current binded RTVs were not from the same pipeline or if they were updated.
		std::vector<ID3D11RenderTargetView*> pDefaultRTVs = Renderer::Get()->GetDefaultPipeline()->GetRenderTargetViews();
		if (s_CurrentBindedStates.RTVsID != pDefaultPipeline->GetID() || s_CurrentBindedStates.DepthStencilViewID != m_ID
			|| pDefaultPipeline->m_ShouldBindRTVs || m_ShouldBindDepthStencilView)
		{
			m_pContext->OMSetRenderTargets((UINT)pDefaultRTVs.size(), pDefaultRTVs.data(), m_pDepthStencilView);
			s_CurrentBindedStates.RTVsID = pDefaultPipeline->GetID();
			s_CurrentBindedStates.DepthStencilViewID = m_ID;
			pDefaultPipeline->m_ShouldBindRTVs = false;
			m_ShouldBindDepthStencilView = false;
		}
	}
	else
	{
		Renderer::Get()->GetDefaultPipeline()->BindDepthAndRTVs();
	}
}

void Pipeline::SetDepthStencilView(D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc)
{
	if (m_pDepthStencilBuffer == nullptr)
	{
		LOG_ERROR("Trying to set depth stencil view without having created a depth stencil buffer!");
		return;
	}

	m_ShouldBindDepthStencilView = true;
	m_DepthStencilSave.depthStencilViewDesc = depthStencilViewDesc;
	CreateDepthStencilView();
}

void Pipeline::SetDepthState(D3D11_DEPTH_STENCIL_DESC depthStencilDesc, D3D11_TEXTURE2D_DESC depthBufferDesc)
{
	m_DepthStencilSave.depthStencilDesc = depthStencilDesc;
	m_DepthStencilSave.depthBufferDesc = depthBufferDesc;

	CreateDepthState();

	m_ShouldBindDepthStencilState = true;
	BindDepthStencilState();
}

void Pipeline::SetRasterState(D3D11_RASTERIZER_DESC rasterizerDesc)
{
	if (m_pRasterizerState)
	{
		m_pRasterizerState->Release();
		m_pRasterizerState = nullptr;
	}

	HRESULT result = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerState);
	RS_D311_ASSERT_CHECK(result, "Failed to create the rasterizer state!");

	m_ShouldBindRasterizerState = true;
	BindRasterState();
}

void RS::Pipeline::SetRenderTargetViews(std::vector<ID3D11RenderTargetView*> rtvs)
{
	m_RenderTargetViews = rtvs;

	m_ShouldBindRTVs = true;
}

void Pipeline::SetRenderTargetView(ID3D11RenderTargetView* rtv)
{
	m_RenderTargetViews.clear();
	m_RenderTargetViews.push_back(rtv);

	m_ShouldBindRTVs = true;
}

void Pipeline::SetViewport(float x, float y, float width, float height)
{
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = x;
	viewport.TopLeftY = y;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MaxDepth = 1.f;
	viewport.MinDepth = 0.f;
	m_pContext->RSSetViewports(1, &viewport);
}

ID3D11DepthStencilView* Pipeline::GetDepthStencilView()
{
    return m_pDepthStencilView;
}

ID3D11RenderTargetView* Pipeline::GetRenderTargetView(uint32 index)
{
    return m_RenderTargetViews[index];
}

std::vector<ID3D11RenderTargetView*>& Pipeline::GetRenderTargetViews()
{
	return m_RenderTargetViews;
}

ID3D11RasterizerState* RS::Pipeline::GetRasterState()
{
	return m_pRasterizerState;
}

ID3D11DepthStencilState* RS::Pipeline::GetDepthStencilState()
{
	return m_pDepthStencilState;
}

uint32 Pipeline::GetID() const
{
	return m_ID;
}

void Pipeline::CreateDepthState()
{
	if (m_pDepthStencilState)
	{
		m_pDepthStencilState->Release();
		m_pDepthStencilState = nullptr;
	}

	// Create the depth stencil state.
	HRESULT result = m_pDevice->CreateDepthStencilState(&m_DepthStencilSave.depthStencilDesc, &m_pDepthStencilState);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to create the depth stencil state!");

	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
		m_pDepthStencilBuffer = nullptr;
	}

	// Create the texture for the depth buffer using the filled out description.
	result = m_pDevice->CreateTexture2D(&m_DepthStencilSave.depthBufferDesc, NULL, &m_pDepthStencilBuffer);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to create the depth buffer texture!");
}

void Pipeline::CreateDepthStencilView()
{
	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = nullptr;
	}

	// Create the depth stencil view.
	HRESULT result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &m_DepthStencilSave.depthStencilViewDesc, &m_pDepthStencilView);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to create the depth stencil view!");

	m_ShouldBindDepthStencilView = true;
}

uint32 Pipeline::GenID()
{
	static uint32 s_Generator = 0;
	return ++s_Generator;
}
