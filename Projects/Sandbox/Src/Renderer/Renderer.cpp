#include "PreCompiled.h"
#include "Renderer.h"

#include "Core/Display.h"

#include "Utils/Config.h"

using namespace RS;

std::shared_ptr<Renderer> Renderer::Get()
{
    static std::shared_ptr<Renderer> s_Renderer = std::make_shared<Renderer>();
    return s_Renderer;
}

void Renderer::Init(DisplayDescription& displayDescriptor)
{
	// Fetch the device, device context and the swap chain from the DirectX api.
	m_pDevice		= RenderAPI::Get()->GetDevice();
	m_pContext		= RenderAPI::Get()->GetDeviceContext();
	m_pSwapChain	= RenderAPI::Get()->GetSwapChain();

	CreateRTV();
	CreateDepthBuffer(displayDescriptor);

	// Set the depth stencil state.
	CreateDepthStencilState();
	m_pContext->OMSetDepthStencilState(m_pDepthStencilState, 1);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	CreateDepthStencilView();
	m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// Set the rasterizer state.
	CreateRasterizer();
	m_pContext->RSSetState(m_pRasterizerState);

	SetViewport(0.f, 0.f, (float)displayDescriptor.Width, (float)displayDescriptor.Height);
}

void Renderer::Release()
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

	ClearRTV();
}

void Renderer::Resize(uint32 width, uint32 height)
{
    ClearRTV();
    m_pSwapChain->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, 0);
    CreateRTV();
    SetViewport(0.f, 0.f, (float)width, (float)height);
}

void Renderer::BeginScene(float r, float g, float b, float a)
{
	float color[4] = { r, g, b, a };
	// Clear back buffer.
	m_pContext->ClearRenderTargetView(m_pRenderTargetView, color);
	// Clear depth and stencil buffer.
	m_pContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::SetViewport(float x, float y, float width, float height)
{
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX	= x;
	viewport.TopLeftY	= y;
	viewport.Width		= width;
	viewport.Height		= height;
	viewport.MaxDepth	= 1.f;
	viewport.MinDepth	= 0.f;
	m_pContext->RSSetViewports(1, &viewport);
}

void Renderer::BindWindowRTV()
{
	m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
}

void Renderer::EndScene()
{
	DisplayDescription& desc = Display::Get()->GetDescription();
	if (desc.VSync)
	{
		// Lock to monitor refresh rate
		m_pSwapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible (Swap buffers)
		// This is slow (slower than OpenGL, glfwSwapBuffers is ~2x faster), for some reason.
		m_pSwapChain->Present(0, 0);
	}
}

ID3D11RenderTargetView* Renderer::GetRenderTarget()
{
	return m_pRenderTargetView;
}

void Renderer::CreateRTV()
{
	HRESULT result;
	ID3D11Texture2D* pBackBufferPtr = 0;
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBufferPtr);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to fetch the back buffer pointer!");

	// Create the render target view with the back buffer pointer.
	result = m_pDevice->CreateRenderTargetView(pBackBufferPtr, NULL, &m_pRenderTargetView);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to create the RTV!");

	pBackBufferPtr->Release();
	pBackBufferPtr = 0;
}

void Renderer::ClearRTV()
{
	if (m_pRenderTargetView)
	{
		m_pRenderTargetView->Release();
		m_pRenderTargetView = nullptr;
	}
}

void Renderer::CreateDepthBuffer(DisplayDescription& displayDescriptor)
{
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = displayDescriptor.Width;
	depthBufferDesc.Height = displayDescriptor.Height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	HRESULT result = m_pDevice->CreateTexture2D(&depthBufferDesc, NULL, &m_pDepthStencilBuffer);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to create the depth buffer texture!");
}

void Renderer::CreateDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	HRESULT result = m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDepthStencilState);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to create the depth stencil state!");
}

void Renderer::CreateDepthStencilView()
{
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	HRESULT result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to create the depth stencil view!");
}

void Renderer::CreateRasterizer()
{
	// Setup the raster description which will determine how and what polygons will be drawn.
	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	HRESULT result = m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pRasterizerState);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to create the rasterizer state!");
}
