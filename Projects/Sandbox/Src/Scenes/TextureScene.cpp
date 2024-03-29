#include "PreCompiled.h"
#include "TextureScene.h"

#include "Renderer/ShaderHotReloader.h"
#include "Renderer/Renderer.h"
#include "Core/Display.h"

#include "Utils/Maths.h"
#include "Core/ResourceManager.h"

#include "Core/Input.h"
#include "Scenes/CameraUtils.h"

using namespace RS;

TextureScene::TextureScene() : Scene("Texture Scene")
{
}

void TextureScene::Start()
{
	glm::vec3 camPos(0.0f, 0.5f, 2.0f);
	glm::vec3 camDir(0.0, 0.0, -1.0);
	constexpr float fov = glm::pi<float>() / 4.f;
	float nearPlane = 0.01f, farPlane = 100.f;
	m_Camera.Init(camPos, camDir, glm::vec3{ 0.f, 1.f, 0.f }, nearPlane, farPlane, fov);

	{
		AttributeLayout layout;
		layout.Push(DXGI_FORMAT_R32G32B32A32_FLOAT, "POSITION", 0);
		layout.Push(DXGI_FORMAT_R32G32B32A32_FLOAT, "COLOR", 0);
		layout.Push(DXGI_FORMAT_R32G32_FLOAT, "TEXCOORD", 0);
		Shader::Descriptor shaderDesc = {};
		shaderDesc.Vertex = "TextureScene/SandboxVert.hlsl";
		shaderDesc.Fragment = "TextureScene/SandboxFrag.hlsl";
		m_Shader.Load(shaderDesc, layout);
		ShaderHotReloader::AddShader(&m_Shader);
	}

	{
		AttributeLayout layout;
		layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "POSITION", 0);
		layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "NORMAL", 0);
		layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "TANGENT", 0);
		layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "BITANGENT", 0);
		layout.Push(DXGI_FORMAT_R32G32_FLOAT, "TEXCOORD", 0);
		Shader::Descriptor shaderDesc = {};
		shaderDesc.Vertex = "TextureScene/SkyboxVert.hlsl";
		shaderDesc.Fragment = "TextureScene/SkyboxFrag.hlsl";
		m_SkyboxShader.Load(shaderDesc, layout);
		ShaderHotReloader::AddShader(&m_SkyboxShader);
	}
	
	std::vector<Vertex> vertices =
	{
		{glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f), glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec2(0.f, 1.f)},
		{glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f), glm::vec4(0.f, 1.f, 0.f, 1.f), glm::vec2(0.f, 0.f)},
		{glm::vec4(0.5f, 0.5f, 0.0f, 1.0f), glm::vec4(0.f, 0.f, 1.f, 1.f), glm::vec2(1.f, 0.f)},
		{glm::vec4(0.5f, -0.5f, 0.0f, 1.0f), glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec2(1.f, 1.f)}
	};

	std::vector<uint32> indices =
	{
		0, 1, 2, 2, 3, 0
	};

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = (UINT)(sizeof(Vertex)*vertices.size());
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = vertices.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pVertexBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create vertex buffer!");
	}

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth			= (UINT) (sizeof(uint32)*indices.size());
		bufferDesc.Usage				= D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags			= D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags		= 0;
		bufferDesc.MiscFlags			= 0;
		bufferDesc.StructureByteStride	= 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = indices.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pIndexBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create index buffer!");
	}

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(FrameData);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = &m_FrameData;
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pConstantBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create constant buffer!");
	}

	{
		ImageLoadDesc imageDesc = {};
		imageDesc.File.Path = "Home.jpg";
		imageDesc.Name = imageDesc.File.Path;
		imageDesc.NumChannels = ImageLoadDesc::Channels::RGBA;
		auto [pImage, handler] = ResourceManager::Get()->LoadImageResource(imageDesc);
		ImageResource* pImageResource = pImage;

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width				= pImageResource->Width;
		textureDesc.Height				= pImageResource->Height;
		textureDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.MipLevels			= 1;
		textureDesc.ArraySize			= 1;
		textureDesc.SampleDesc.Count	= 1;
		textureDesc.SampleDesc.Quality	= 0;
		textureDesc.Usage				= D3D11_USAGE_IMMUTABLE;
		textureDesc.CPUAccessFlags		= 0;
		textureDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
		textureDesc.MiscFlags			= 0;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem			= pImageResource->Data.data();
		data.SysMemPitch		= pImageResource->Width * 4;
		data.SysMemSlicePitch	= 0;

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateTexture2D(&textureDesc, &data, &m_pTexture);
		RS_D311_ASSERT_CHECK(result, "Failed to create texture!");

		ResourceManager::Get()->FreeResource(pImageResource);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(m_pTexture, &srvDesc, &m_pTextureSRV);
		RS_D311_ASSERT_CHECK(result, "Failed to create texture RSV!");

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter			= D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU		= D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV		= D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW		= D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias		= 0.f;
		samplerDesc.MaxAnisotropy	= 16;
		samplerDesc.ComparisonFunc	= D3D11_COMPARISON_GREATER_EQUAL;
		samplerDesc.MinLOD			= 0.f;
		samplerDesc.MaxLOD			= D3D11_FLOAT32_MAX;
		samplerDesc.BorderColor[0]	= 0.f;
		samplerDesc.BorderColor[1]	= 0.f;
		samplerDesc.BorderColor[2]	= 0.f;
		samplerDesc.BorderColor[3]	= 0.f;

		result = RenderAPI::Get()->GetDevice()->CreateSamplerState(&samplerDesc, &m_pSampler);
		RS_D311_ASSERT_CHECK(result, "Failed to create sampler!");
	}

	{
		ModelLoadDesc modelLoadDesc = {};
		modelLoadDesc.FilePath = "InvCube.glb";
		modelLoadDesc.Loader = ModelLoadDesc::Loader::ASSIMP;
		modelLoadDesc.Flags = ModelLoadDesc::LOADER_FLAG_UPLOAD_MESH_DATA_TO_GUP | ModelLoadDesc::LOADER_FLAG_NO_MESH_DATA_IN_RAM | ModelLoadDesc::LOADER_FLAG_USE_UV_TOP_LEFT;
		auto [pModel, handler1] = ResourceManager::Get()->LoadModelResource(modelLoadDesc);
		m_pModel = pModel;

		CubeMapLoadDesc cubeMapLoadDesc = {};
		for (uint32 i = 0; i < 6; i++)
		{
			cubeMapLoadDesc.ImageDescs[i].IsFromFile = true;
			cubeMapLoadDesc.ImageDescs[i].NumChannels = ImageLoadDesc::Channels::RGBA;
			cubeMapLoadDesc.ImageDescs[i].Name = "CubeMap_" + std::to_string(i);
			cubeMapLoadDesc.ImageDescs[i].File.UseDefaultFolder = true;
		}
		cubeMapLoadDesc.ImageDescs[0].File.Path = "Skybox/right.jpg";
		cubeMapLoadDesc.ImageDescs[1].File.Path = "Skybox/left.jpg";
		cubeMapLoadDesc.ImageDescs[2].File.Path = "Skybox/top.jpg";
		cubeMapLoadDesc.ImageDescs[3].File.Path = "Skybox/bottom.jpg";
		cubeMapLoadDesc.ImageDescs[4].File.Path = "Skybox/front.jpg";
		cubeMapLoadDesc.ImageDescs[5].File.Path = "Skybox/back.jpg";

		cubeMapLoadDesc.GenerateMipmaps = false; // Will only use the highest mip level.
		auto [pCubeMap, handler2] = ResourceManager::Get()->LoadCubeMapResource(cubeMapLoadDesc);
		m_pCubeMap = pCubeMap;
	}

	m_Pipeline.Init();

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	m_Pipeline.SetRasterState(rasterizerDesc);
}

void TextureScene::Selected()
{
}

void TextureScene::Unselected()
{
}

void TextureScene::End()
{
	m_Pipeline.Release();

	m_Shader.Release();
	m_SkyboxShader.Release();
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
	m_pConstantBuffer->Release();

	m_pTexture->Release();
	m_pTextureSRV->Release();
	m_pSampler->Release();
}

void TextureScene::FixedTick()
{
}

void TextureScene::Tick(float dt)
{
	CameraUtils::UpdateOrbitCamera(m_Camera);

	m_Pipeline.Bind(BindType::BOTH);

	auto display = Display::Get();
	auto renderer = Renderer::Get();
	auto renderAPI = RenderAPI::Get();
	ID3D11DeviceContext* pContext = renderAPI->GetDeviceContext();
	renderer->BeginScene(0.0f, 0.2f, 0.2f, 1.0f);

	m_Shader.Bind();

	// Update data
	{
		// Use right-handed coordinate system (+z is towards the viewer!)
		m_FrameData.world = glm::rotate(m_FrameData.world, glm::pi<float>() * dt*0.5f, glm::vec3(0.f, 1.f, 0.f));
		m_FrameData.view = m_Camera.GetView();
		m_FrameData.proj = m_Camera.GetProj();

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = pContext->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map constant buffer!");

		FrameData* data = (FrameData*)mappedResource.pData;
		memcpy(data, &m_FrameData, sizeof(FrameData));

		pContext->Unmap(m_pConstantBuffer, 0);
	}

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->PSSetShaderResources(0, 1, &m_pTextureSRV);
	pContext->PSSetSamplers(0, 1, &m_pSampler);
	pContext->DrawIndexed(6, 0, 0);
	
	{
		m_SkyboxShader.Bind();

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = pContext->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map constant buffer!");
		FrameData* data = (FrameData*)mappedResource.pData;
		data->world = glm::scale(glm::vec3(10.f));
		data->view = m_Camera.GetView();
		data->proj = m_Camera.GetProj();
		pContext->Unmap(m_pConstantBuffer, 0);

		SamplerResource* pSampler = ResourceManager::Get()->GetResource<SamplerResource>(ResourceManager::Get()->DefaultSamplerLinear);
		pContext->VSSetConstantBuffers(1, 1, &m_pConstantBuffer);
		pContext->PSSetShaderResources(0, 1, &m_pCubeMap->pTextureSRV);
		pContext->PSSetSamplers(0, 1, &pSampler->pSampler);

		glm::mat4 transform(1.f);
		Renderer::DebugInfo debugInfo = {};
		debugInfo.DrawAABBs = false;
		debugInfo.ID = 0;
		debugInfo.RenderMode = 0;
		renderer->Render(*m_pModel, transform, debugInfo, RenderFlag::RENDER_FLAG_NO_TEXTURES);
	}
}