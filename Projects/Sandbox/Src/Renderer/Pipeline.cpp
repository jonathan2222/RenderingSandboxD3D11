#include "PreCompiled.h"
#include "Pipeline.h"

#include "Renderer/Renderer.h"

using namespace RS;

void Pipeline::Init()
{
	// Fetch the device, device context from the DirectX API.
	m_pDevice = RenderAPI::Get()->GetDevice();
	m_pContext = RenderAPI::Get()->GetDeviceContext();
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
	BindDepthStencilState();

	CreateDepthStencilView();

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
		m_pContext->OMSetDepthStencilState(m_pDepthStencilState, 1);
	else
	{
		ID3D11DepthStencilState* pDepthStencilState = Renderer::Get()->GetDefaultPipeline()->GetDepthStencilState();
		m_pContext->OMSetDepthStencilState(pDepthStencilState, 1);
	}
}

void Pipeline::BindRasterState()
{
	if (m_pRasterizerState)
		m_pContext->RSSetState(m_pRasterizerState);
	else
	{
		ID3D11RasterizerState* pRasterizerState = Renderer::Get()->GetDefaultPipeline()->GetRasterState();
		m_pContext->RSSetState(pRasterizerState);
	}
}

void Pipeline::BindDepthAndRTVs()
{
	if (m_RenderTargetViews.empty() == false)
	{
		if(m_pDepthStencilView)
			m_pContext->OMSetRenderTargets((UINT)m_RenderTargetViews.size(), m_RenderTargetViews.data(), m_pDepthStencilView);
		else
		{
			ID3D11DepthStencilView* pDefaultDepthStencilView = Renderer::Get()->GetDefaultPipeline()->GetDepthStencilView();
			m_pContext->OMSetRenderTargets((UINT)m_RenderTargetViews.size(), m_RenderTargetViews.data(), pDefaultDepthStencilView);
		}
	}
	else if (m_pDepthStencilView)
	{
		std::vector<ID3D11RenderTargetView*> pDefaultRTVs = Renderer::Get()->GetDefaultPipeline()->GetRenderTargetViews();
		m_pContext->OMSetRenderTargets((UINT)pDefaultRTVs.size(), pDefaultRTVs.data(), m_pDepthStencilView);
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

	m_DepthStencilSave.depthStencilViewDesc = depthStencilViewDesc;
	CreateDepthStencilView();
}

void Pipeline::SetDepthState(D3D11_DEPTH_STENCIL_DESC depthStencilDesc, D3D11_TEXTURE2D_DESC depthBufferDesc)
{
	m_DepthStencilSave.depthStencilDesc = depthStencilDesc;
	m_DepthStencilSave.depthBufferDesc = depthBufferDesc;

	CreateDepthState();
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

	BindRasterState();
}

void RS::Pipeline::SetRenderTargetViews(std::vector<ID3D11RenderTargetView*> rtvs)
{
	m_RenderTargetViews = rtvs;
}

void Pipeline::SetRenderTargetView(ID3D11RenderTargetView* rtv)
{
	m_RenderTargetViews.clear();
	m_RenderTargetViews.push_back(rtv);
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
}
