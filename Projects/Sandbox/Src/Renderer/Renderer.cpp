#include "PreCompiled.h"
#include "Renderer.h"

#include "Core/Display.h"
#include "Renderer/ImGuiRenderer.h"
#include "Renderer/DebugRenderer.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderHotReloader.h"
#include "Renderer/D3D11/D3D11Helper.h"

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
	m_DefaultPipeline.BindDepthAndRTVs(BindType::BOTH);

	// Set the rasterizer state.
	CreateRasterizer();

	m_DefaultPipeline.SetViewport(0.f, 0.f, (float)displayDescriptor.Width, (float)displayDescriptor.Height);

	// Texture format conversion resources.
	{
		m_TextureFormatConvertionPipeline.Init();
		D3D11_RASTERIZER_DESC rasterState = {};
		rasterState.FillMode = D3D11_FILL_SOLID;
		rasterState.CullMode = D3D11_CULL_NONE;
		rasterState.ScissorEnable = false;
		rasterState.DepthClipEnable = false;
		m_TextureFormatConvertionPipeline.SetRasterState(rasterState);

		AttributeLayout layout;
		layout.Push(DXGI_FORMAT_R32G32_FLOAT, "POSITION", 0);
		Shader::Descriptor shaderDesc	= {};
		shaderDesc.Vertex				= "RenderTools/FormatConverterVert.hlsl";
		shaderDesc.Fragment				= "RenderTools/FormatConverterFrag.hlsl";
		m_TextureFormatConvertionShader.Load(shaderDesc, layout);
		ShaderHotReloader::AddShader(&m_TextureFormatConvertionShader);
	}

	// Equirectangular To Cubemap
	{
		m_EquirectangularToCubemapPipeline.Init();
		D3D11_RASTERIZER_DESC rasterState = {};
		rasterState.FillMode = D3D11_FILL_SOLID;
		rasterState.CullMode = D3D11_CULL_NONE;
		rasterState.ScissorEnable = false;
		rasterState.DepthClipEnable = false;
		m_EquirectangularToCubemapPipeline.SetRasterState(rasterState);

		AttributeLayout layout;
		layout.Push(DXGI_FORMAT_R32G32_FLOAT, "POSITION", 0);
		Shader::Descriptor shaderDesc = {};
		shaderDesc.Vertex = "RenderTools/EquirectangularToCubemapVert.hlsl";
		shaderDesc.Fragment = "RenderTools/EquirectangularToCubemapFrag.hlsl";
		m_EquirectangularToCubemapShader.Load(shaderDesc, layout);
		ShaderHotReloader::AddShader(&m_EquirectangularToCubemapShader);

		{
			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.ByteWidth = sizeof(m_EquirectangularToCubemapFrameData);
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;

			m_EquirectangularToCubemapFrameData.View = glm::mat4(1.f);
			m_EquirectangularToCubemapFrameData.Proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = &m_EquirectangularToCubemapFrameData;
			data.SysMemPitch = 0;
			data.SysMemSlicePitch = 0;

			HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pEquirectangularToCubemapConstantBuffer);
			RS_D311_ASSERT_CHECK(result, "Failed to create EquirectangularToCubemap constant buffer!");

			m_EquirectangularToCubemapCaptureViews[0] = glm::lookAt(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
			m_EquirectangularToCubemapCaptureViews[1] = glm::lookAt(glm::vec3(0.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
			m_EquirectangularToCubemapCaptureViews[2] = glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
			m_EquirectangularToCubemapCaptureViews[3] = glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
			m_EquirectangularToCubemapCaptureViews[4] = glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f));
			m_EquirectangularToCubemapCaptureViews[5] = glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f));
		}
	}
}

void Renderer::Release()
{
	if (m_TextureFormatConvertionRTV)
	{
		m_TextureFormatConvertionRTV->Release();
		m_TextureFormatConvertionRTV = nullptr;
	}
	m_TextureFormatConvertionShader.Release();
	m_TextureFormatConvertionPipeline.Release();

	for (auto& rtv : m_EquirectangularToCubemapRTVs)
	{
		if (rtv)
		{
			rtv->Release();
			rtv = nullptr;
		}
	}
	m_pEquirectangularToCubemapConstantBuffer->Release();
	m_EquirectangularToCubemapShader.Release();
	m_EquirectangularToCubemapPipeline.Release();

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
	m_DefaultPipeline.BindDepthAndRTVs(BindType::BOTH);
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

void Renderer::ConvertTextureFormat(TextureResource* pTexture, DXGI_FORMAT newFormat)
{
	D3D11_TEXTURE2D_DESC oldTextureDesc = {};
	pTexture->pTexture->GetDesc(&oldTextureDesc);
	std::string preFormatStr = RenderUtils::FormatToString(oldTextureDesc.Format);
	std::string newFormatStr = RenderUtils::FormatToString(newFormat);

	if (!pTexture->UseAsRTV)
	{
		LOG_WARNING("Failed to convert texture format, from {} to {}, texture must allow RTV use!", preFormatStr.c_str(), newFormatStr.c_str());
		return;
	}

	if (m_TextureFormatConvertionRTV != nullptr)
	{
		m_TextureFormatConvertionRTV->Release();
		m_TextureFormatConvertionRTV = nullptr;
	}
	
	ImageResource* pImage = ResourceManager::Get()->GetResource<ImageResource>(pTexture->ImageHandler);

	ID3D11Texture2D* pNewTexture = nullptr;
	ID3D11ShaderResourceView* pNewTextureSRV = nullptr;

	{
		D3D11_TEXTURE2D_DESC textureDesc = D3D11Helper::GetTexture2DDesc(pImage->Width, pImage->Height, newFormat);
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		if (pTexture->NumMipLevels > 1)
		{
			textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
			textureDesc.MipLevels = (uint32)glm::ceil(glm::max(glm::log2(glm::min((float)textureDesc.Width, (float)textureDesc.Height)), 1.f));
		}

		std::vector<D3D11_SUBRESOURCE_DATA> subData = D3D11Helper::FillTexture2DSubdata(textureDesc, pImage->Data.data());

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateTexture2D(&textureDesc, subData.data(), &pNewTexture);
		RS_D311_ASSERT_CHECK(result, "Failed to create texture!");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = pTexture->NumMipLevels > 1 ? -1 : 1;
		result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pNewTexture, &srvDesc, &pNewTextureSRV);
		RS_D311_ASSERT_CHECK(result, "Failed to create texture RSV!");
	}
	
	D3D11_RENDER_TARGET_VIEW_DESC desc = {};
	desc.Format = newFormat;
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;
	HRESULT hr = RenderAPI::Get()->GetDevice()->CreateRenderTargetView(pNewTexture, &desc, &m_TextureFormatConvertionRTV);
	if (FAILED(hr))
	{
		LOG_WARNING("Failed to convert texture format, from {} to {}!", preFormatStr.c_str(), newFormatStr.c_str());
		return;
	}

	// Use a pipeline and draw to the texture as a rendertarget.
	m_TextureFormatConvertionPipeline.SetRenderTargetView(m_TextureFormatConvertionRTV);
	m_TextureFormatConvertionPipeline.Bind(BindType::RTV_ONLY);

	SamplerResource* pSamplerResource = ResourceManager::Get()->GetResource<SamplerResource>(ResourceManager::Get()->DefaultSamplerLinear);

	m_TextureFormatConvertionShader.Bind();
	m_TextureFormatConvertionPipeline.SetViewport(0.f, 0.f, (float)pImage->Width, (float)pImage->Height);
	ID3D11DeviceContext* pContext = RenderAPI::Get()->GetDeviceContext();
	pContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->PSSetSamplers(0, 1, &pSamplerResource->pSampler);
	pContext->PSSetShaderResources(0, 1, &pTexture->pTextureSRV);
	pContext->Draw(3, 0);

	// Reset render target
	ID3D11RenderTargetView* nullRTVs = nullptr;
	pContext->OMSetRenderTargets(1, &nullRTVs, nullptr);
	
	// Generate mips from the newly created texture with a new format and make that the texture instead. Also create debug SRVs for it.
	{
		if (pTexture->NumMipLevels > 1)
			RenderAPI::Get()->GetDeviceContext()->GenerateMips(pNewTextureSRV);

		// Release the previous textures.
		pTexture->pTexture->Release();
		pTexture->pTextureSRV->Release();

		pTexture->Format		= newFormat;
		pTexture->pTexture		= pNewTexture;
		pTexture->pTextureSRV	= pNewTextureSRV;

		// Remove the previous debug SRVs
		for (auto srv : pTexture->DebugMipmapSRVs)
			srv->Release();

		// Create the new debug SRVs
		if (pTexture->NumMipLevels > 0)
			pTexture->DebugMipmapSRVs.resize(pTexture->NumMipLevels - 1);
		for (uint32 mip = 1; mip < pTexture->NumMipLevels; mip++)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = newFormat;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = mip;
			HRESULT result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pTexture->pTexture, &srvDesc, &pTexture->DebugMipmapSRVs[mip - 1]);
			RS_D311_ASSERT_CHECK(result, "Failed to create debug texture SRV for one of the mip levels in the texture!");
		}
	}
}

CubeMapResource* Renderer::ConvertEquirectangularToCubemap(TextureResource* pTexture, uint32_t width, uint32_t height)
{
	CubeMapLoadDesc cubemapLoadDesc = {};
	// Will not generate mipmaps because of EmptyInitialization but it will enable us to do it later.
	cubemapLoadDesc.GenerateMipmaps = pTexture->NumMipLevels > 1;
	cubemapLoadDesc.EmptyInitialization = true;
	cubemapLoadDesc.Width = width;
	cubemapLoadDesc.Height = height;
	cubemapLoadDesc.Format = pTexture->Format;
	// As to not get a warning and to see it in the resource inspector we give it a name.
	cubemapLoadDesc.ImageDescs[0].Name = ResourceManager::Get()->GetResourceName(pTexture->key);
	auto [pCubemap, id] = ResourceManager::Get()->LoadCubeMapResource(cubemapLoadDesc);

	D3D11_RENDER_TARGET_VIEW_DESC desc	= {};
	desc.Format							= cubemapLoadDesc.Format;
	desc.ViewDimension					= D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	desc.Texture2DArray.ArraySize		= 1;
	desc.Texture2DArray.MipSlice		= 0;
	for (uint32_t i = 0; i < 6; i++)
	{
		desc.Texture2DArray.FirstArraySlice = i;
		HRESULT hr = RenderAPI::Get()->GetDevice()->CreateRenderTargetView(pCubemap->pTexture, &desc, &m_EquirectangularToCubemapRTVs[i]);
		if (FAILED(hr))
		{
			LOG_WARNING("Failed to convert a equirectangular texture to a cubemap!");
			return pCubemap;
		}
	}

	// Use a pipeline and draw to the texture as a rendertarget.
	SamplerResource* pSamplerResource = ResourceManager::Get()->GetResource<SamplerResource>(ResourceManager::Get()->DefaultSamplerLinear);

	m_EquirectangularToCubemapShader.Bind();
	m_EquirectangularToCubemapPipeline.SetViewport(0.f, 0.f, (float)width, (float)height);
	ID3D11DeviceContext* pContext = RenderAPI::Get()->GetDeviceContext();
	pContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->PSSetSamplers(0, 1, &pSamplerResource->pSampler);
	pContext->PSSetShaderResources(0, 1, &pTexture->pTextureSRV);

	for (uint32_t i = 0; i < 6; i++)
	{
		m_EquirectangularToCubemapPipeline.SetRenderTargetView(m_EquirectangularToCubemapRTVs[i]);
		m_EquirectangularToCubemapPipeline.Bind(BindType::RTV_ONLY);

		m_EquirectangularToCubemapFrameData.View = m_EquirectangularToCubemapCaptureViews[i];
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		{
			HRESULT result = pContext->Map(m_pEquirectangularToCubemapConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			RS_D311_ASSERT_CHECK(result, "Failed to map EquirectangularToCubemap constant buffer!");
			memcpy(mappedResource.pData, &m_EquirectangularToCubemapFrameData, sizeof(EquirectangularToCubemapFrameData));
			pContext->Unmap(m_pEquirectangularToCubemapConstantBuffer, 0);
		}
		pContext->VSSetConstantBuffers(0, 1, &m_pEquirectangularToCubemapConstantBuffer);
		pContext->Draw(3, 0);
	}

	ID3D11RenderTargetView* nullRTVs = nullptr;
	pContext->OMSetRenderTargets(1, &nullRTVs, nullptr);

	if(cubemapLoadDesc.GenerateMipmaps)
		ResourceManager::Get()->GenerateMipmaps(pCubemap);
	else
	{
		for (auto& srvs : pCubemap->DebugMipmapSRVs)
		{
			for (auto& srv : srvs)
			{
				srv->Release();
				srv = nullptr;
			}
		}

		pCubemap->DebugMipmapSRVs.resize(6);
		for (uint32 side = 0; side < 6; side++)
		{
			pCubemap->DebugMipmapSRVs[side].resize(1);
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = pTexture->Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.MipLevels = 1;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			srvDesc.Texture2DArray.ArraySize = 1;
			srvDesc.Texture2DArray.FirstArraySlice = side;
			HRESULT result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pCubemap->pTexture, &srvDesc, &pCubemap->DebugMipmapSRVs[side][0]);
			if (FAILED(result))
				LOG_WARNING("Failed to create debug texture SRV for one of the sides on the EquirectangularToCubemap cubemap!");
		}
	}

	return pCubemap;
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
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = D3D11Helper::GetDepthStencilDesc();

	D3D11_TEXTURE2D_DESC depthBufferDesc = D3D11Helper::GetTexture2DDesc(displayDescriptor.Width, displayDescriptor.Height, DXGI_FORMAT_D24_UNORM_S8_UINT);
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

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
	D3D11_RASTERIZER_DESC rasterizerDesc = D3D11Helper::GetRasterizerDesc();
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
