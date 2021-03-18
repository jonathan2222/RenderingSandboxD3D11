#include "PreCompiled.h"
#include "Renderer.h"

#include "Core/Display.h"
#include "Renderer/ImGuiRenderer.h"
#include "Renderer/DebugRenderer.h"

#include "Utils/Config.h"

using namespace RS;

std::shared_ptr<Renderer> Renderer::Get()
{
    static std::shared_ptr<Renderer> s_Renderer = std::make_shared<Renderer>();
    return s_Renderer;
}

void Renderer::Init(DisplayDescription& displayDescriptor)
{
	m_DefaultPipeline.Init();

	// Fetch the device, device context and the swap chain from the DirectX api.
	m_pDevice		= RenderAPI::Get()->GetDevice();
	m_pContext		= RenderAPI::Get()->GetDeviceContext();
	m_pSwapChain	= RenderAPI::Get()->GetSwapChain();

	CreateRTV();

	// Set the depth stencil state.
	CreateDepthStencilState(displayDescriptor);

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	CreateDepthStencilView();
	m_DefaultPipeline.BindDepthAndRTVs();

	// Set the rasterizer state.
	CreateRasterizer();

	m_DefaultPipeline.SetViewport(0.f, 0.f, (float)displayDescriptor.Width, (float)displayDescriptor.Height);
}

void Renderer::Release()
{
	m_DefaultPipeline.Release();
	ClearRTV();
}

void Renderer::Resize(uint32 width, uint32 height)
{
	if (width != 0 || height != 0)
	{
		ClearRTV();
		m_pSwapChain->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, 0);
		CreateRTV();

		m_DefaultPipeline.Resize(width, height);

		ImGuiRenderer::Resize();
	}
}

void Renderer::BeginScene(float r, float g, float b, float a)
{
	float color[4] = { r, g, b, a };
	// Clear back buffer.
	m_pContext->ClearRenderTargetView(m_pRenderTargetView, color);
	// Clear depth and stencil buffer.
	m_pContext->ClearDepthStencilView(m_DefaultPipeline.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::BindWindowRTV()
{
	m_DefaultPipeline.BindDepthAndRTVs();
}

void Renderer::Present()
{
	m_DefaultPipeline.SetViewport(0.f, 0.f, static_cast<float>(Display::Get()->GetWidth()), static_cast<float>(Display::Get()->GetHeight()));

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

void Renderer::Render(ModelResource& model, const glm::mat4& transform, DebugInfo debugInfo, RenderFlags flags)
{
	auto renderAPI = RenderAPI::Get();
	ID3D11DeviceContext* pContext = renderAPI->GetDeviceContext();
	DebugRenderer::Get()->Clear(debugInfo.ID);
	InternalRender(model, transform, pContext, debugInfo, flags);
}

void Renderer::RenderWithMaterial(ModelResource& model, const glm::mat4& transform, DebugInfo debugInfo)
{
	auto renderAPI = RenderAPI::Get();
	ID3D11DeviceContext* pContext = renderAPI->GetDeviceContext();
	DebugRenderer::Get()->Clear(debugInfo.ID);
	InternalRenderWithMaterial(model, transform, pContext, debugInfo);
}

ID3D11RenderTargetView* Renderer::GetRenderTarget()
{
	return m_pRenderTargetView;
}

Pipeline* Renderer::GetDefaultPipeline()
{
	return &m_DefaultPipeline;
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

	m_DefaultPipeline.SetRenderTargetView(m_pRenderTargetView);

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

void Renderer::CreateDepthStencilState(DisplayDescription& displayDescriptor)
{
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable					= true;
	depthStencilDesc.DepthWriteMask					= D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc						= D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable					= true;
	depthStencilDesc.StencilReadMask				= 0xFF;
	depthStencilDesc.StencilWriteMask				= 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width				= displayDescriptor.Width;
	depthBufferDesc.Height				= displayDescriptor.Height;
	depthBufferDesc.MipLevels			= 1;
	depthBufferDesc.ArraySize			= 1;
	depthBufferDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count	= 1;
	depthBufferDesc.SampleDesc.Quality	= 0;
	depthBufferDesc.Usage				= D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags		= 0;
	depthBufferDesc.MiscFlags			= 0;

	m_DefaultPipeline.SetDepthState(depthStencilDesc, depthBufferDesc);
}

void Renderer::CreateDepthStencilView()
{
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	m_DefaultPipeline.SetDepthStencilView(depthStencilViewDesc);
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
	m_DefaultPipeline.SetRasterState(rasterizerDesc);
}

void Renderer::InternalRender(ModelResource& model, const glm::mat4& transform, ID3D11DeviceContext* pContext, DebugInfo debugInfo, RenderFlags flags)
{
	auto SetPSSRV = [&](uint32& slot, ResourceID handler, RenderFlag flag)->void
	{
		if (flags & flag)
		{
			TextureResource* pTexture = ResourceManager::Get()->GetResource<TextureResource>(handler);
			pContext->PSSetShaderResources(slot++, 1, &pTexture->pTextureSRV);
		}
	};

	SamplerResource* pSampler = ResourceManager::Get()->GetResource<SamplerResource>(ResourceManager::Get()->DefaultSamplerLinear);

	MeshObject::MeshData meshData;
	meshData.world = transform * model.Transform;
	for (MeshObject& mesh : model.Meshes)
	{
		MaterialResource* pMaterial = ResourceManager::Get()->GetResource<MaterialResource>(mesh.MaterialHandler);

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = pContext->Map(mesh.pMeshBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map mesh constant buffer!");
		MeshObject::MeshData* data = (MeshObject::MeshData*)mappedResource.pData;
		memcpy(data, &meshData, sizeof(meshData));
		pContext->Unmap(mesh.pMeshBuffer, 0);

		UINT stride = sizeof(MeshObject::Vertex);
		UINT offset = 0;
		pContext->IASetVertexBuffers(0, 1, &mesh.pVertexBuffer, &stride, &offset);
		pContext->IASetIndexBuffer(mesh.pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->VSSetConstantBuffers(0, 1, &mesh.pMeshBuffer);

		uint32 textureSlot = 0;
		SetPSSRV(textureSlot, pMaterial->AlbedoTextureHandler,				RenderFlag::RENDER_FLAG_ALBEDO_TEXTURE);
		SetPSSRV(textureSlot, pMaterial->NormalTextureHandler,				RenderFlag::RENDER_FLAG_NORMAL_TEXTURE);
		SetPSSRV(textureSlot, pMaterial->AOTextureHandler,					RenderFlag::RENDER_FLAG_AO_TEXTURE);
		SetPSSRV(textureSlot, pMaterial->MetallicTextureHandler,			RenderFlag::RENDER_FLAG_METALLIC_TEXTURE);
		SetPSSRV(textureSlot, pMaterial->RoughnessTextureHandler,			RenderFlag::RENDER_FLAG_ROUGHNESS_TEXTURE);
		if(textureSlot != 0)
			pContext->PSSetSamplers(0, 1, &pSampler->pSampler);
		pContext->DrawIndexed((UINT)mesh.NumIndices, 0, 0);

		if (debugInfo.DrawAABBs)
		{
			static Color meshAABBColor = Color::HSBToRGB(1.f, 1.f, 0.2f);
			AABB transformedAABB = AABB::Transform(mesh.BoundingBox, meshData.world);
			DebugRenderer::Get()->PushBox(transformedAABB, meshAABBColor, debugInfo.ID, false);
		}
	}

	if (debugInfo.DrawAABBs)
	{
		static Color modelAABBColor = Color::HSBToRGB(1.f, 1.f, 0.8f);
		AABB transformedAABB = AABB::Transform(model.BoundingBox, meshData.world);
		DebugRenderer::Get()->PushBox(transformedAABB, modelAABBColor, debugInfo.ID, false);
	}

	for (ModelResource& child : model.Children)
		InternalRender(child, meshData.world, pContext, debugInfo, flags);
}

void Renderer::InternalRenderWithMaterial(ModelResource& model, const glm::mat4& transform, ID3D11DeviceContext* pContext, DebugInfo debugInfo)
{
	MeshObject::MeshData meshData;
	meshData.world = transform * model.Transform;
	for (MeshObject& mesh : model.Meshes)
	{
		MaterialResource* pMaterial = ResourceManager::Get()->GetResource<MaterialResource>(mesh.MaterialHandler);
		SamplerResource* pSampler = ResourceManager::Get()->GetResource<SamplerResource>(ResourceManager::Get()->DefaultSamplerLinear);
		TextureResource* pAlbedoTexture = ResourceManager::Get()->GetResource<TextureResource>(pMaterial->AlbedoTextureHandler);
		TextureResource* pNormalTexture = ResourceManager::Get()->GetResource<TextureResource>(pMaterial->NormalTextureHandler);
		TextureResource* pAOTexture = ResourceManager::Get()->GetResource<TextureResource>(pMaterial->AOTextureHandler);
		TextureResource* pMetallicTexture = ResourceManager::Get()->GetResource<TextureResource>(pMaterial->MetallicTextureHandler);
		TextureResource* pRoughnessTexture = ResourceManager::Get()->GetResource<TextureResource>(pMaterial->RoughnessTextureHandler);
		TextureResource* pMetallicRoughnessTexture = ResourceManager::Get()->GetResource<TextureResource>(pMaterial->MetallicRoughnessTextureHandler);

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		{
			HRESULT result = pContext->Map(mesh.pMeshBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			RS_D311_ASSERT_CHECK(result, "Failed to map mesh constant buffer!");
			MeshObject::MeshData* data = (MeshObject::MeshData*)mappedResource.pData;
			memcpy(data, &meshData, sizeof(meshData));
			pContext->Unmap(mesh.pMeshBuffer, 0);
		}

		{
			HRESULT result = pContext->Map(pMaterial->pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			RS_D311_ASSERT_CHECK(result, "Failed to map material constant buffer!");
			MaterialBuffer* data = (MaterialBuffer*)mappedResource.pData;
			pMaterial->InfoBuffer.Info.y = (float)debugInfo.RenderMode;
			memcpy(data, &pMaterial->InfoBuffer, sizeof(pMaterial->InfoBuffer));
			pContext->Unmap(pMaterial->pConstantBuffer, 0);
		}

		UINT stride = sizeof(MeshObject::Vertex);
		UINT offset = 0;
		pContext->IASetVertexBuffers(0, 1, &mesh.pVertexBuffer, &stride, &offset);
		pContext->IASetIndexBuffer(mesh.pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->VSSetConstantBuffers(0, 1, &mesh.pMeshBuffer);
		pContext->PSSetShaderResources(0, 1, &pAlbedoTexture->pTextureSRV);
		pContext->PSSetShaderResources(1, 1, &pNormalTexture->pTextureSRV);
		pContext->PSSetShaderResources(2, 1, &pAOTexture->pTextureSRV);
		pContext->PSSetShaderResources(3, 1, &pMetallicTexture->pTextureSRV);
		pContext->PSSetShaderResources(4, 1, &pRoughnessTexture->pTextureSRV);
		pContext->PSSetShaderResources(5, 1, &pMetallicRoughnessTexture->pTextureSRV);
		pContext->PSSetSamplers(0, 1, &pSampler->pSampler);
		pContext->PSSetConstantBuffers(0, 1, &pMaterial->pConstantBuffer);
		pContext->DrawIndexed((UINT)mesh.NumIndices, 0, 0);

		if (debugInfo.DrawAABBs)
		{
			static Color meshAABBColor = Color::HSBToRGB(1.f, 1.f, 0.2f);
			AABB transformedAABB = AABB::Transform(mesh.BoundingBox, meshData.world);
			DebugRenderer::Get()->PushBox(transformedAABB, meshAABBColor, debugInfo.ID, false);
		}
	}

	if (debugInfo.DrawAABBs)
	{
		static Color modelAABBColor = Color::HSBToRGB(1.f, 1.f, 0.8f);
		AABB transformedAABB = AABB::Transform(model.BoundingBox, meshData.world);
		DebugRenderer::Get()->PushBox(transformedAABB, modelAABBColor, debugInfo.ID, false);
	}

	for (ModelResource& child : model.Children)
		InternalRenderWithMaterial(child, meshData.world, pContext, debugInfo);
}
