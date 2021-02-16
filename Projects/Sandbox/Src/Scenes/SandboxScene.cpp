#include "PreCompiled.h"
#include "SandboxScene.h"

#include "Renderer/ShaderHotReloader.h"
#include "Renderer/Renderer.h"
#include "Core/Display.h"

#include "Utils/Maths.h"

#pragma warning( push )
#pragma warning( disable : 6011 )
#pragma warning( disable : 6262 )
#pragma warning( disable : 6308 )
#pragma warning( disable : 6387 )
#pragma warning( disable : 26451 )
#pragma warning( disable : 28182 )
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning( pop )

using namespace RS;

SandboxScene::SandboxScene() : Scene("SandboxScene")
{
}

void SandboxScene::Start()
{
	AttributeLayout layout;
	layout.Push(DXGI_FORMAT_R32G32B32A32_FLOAT, "POSITION", 0);
	layout.Push(DXGI_FORMAT_R32G32B32A32_FLOAT, "COLOR", 0);
	layout.Push(DXGI_FORMAT_R32G32_FLOAT, "TEXCOORD", 0);
	Shader::Descriptor shaderDesc = {};
	shaderDesc.Vertex = "SandboxVert.hlsl";
	shaderDesc.Fragment = "SandboxFrag.hlsl";
	m_Shader.Load(shaderDesc, layout);
	ShaderHotReloader::AddShader(&m_Shader);

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
		bufferDesc.ByteWidth = sizeof(Vertex)*vertices.size();
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
		bufferDesc.ByteWidth = sizeof(uint32)*indices.size();
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

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
		std::string texturePath = std::string(RS_TEXTURE_PATH) + "Home.jpg";
		int width, height, channelCount, nChannels = 4;
		uint8* pixels = (uint8*)stbi_load(texturePath.c_str(), &width, &height, &channelCount, nChannels);

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = (uint32)width;
		textureDesc.Height = (uint32)height;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = pixels;
		data.SysMemPitch = (uint32)width*4;
		data.SysMemSlicePitch = 0;

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateTexture2D(&textureDesc, &data, &m_pTexture);
		RS_D311_ASSERT_CHECK(result, "Failed to create texture!");

		stbi_image_free(pixels);
		pixels = nullptr;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(m_pTexture, &srvDesc, &m_pTextureSRV);
		RS_D311_ASSERT_CHECK(result, "Failed to create texture RSV!");

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.f;
		samplerDesc.MaxAnisotropy = 16.f;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
		samplerDesc.MinLOD = 0.f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.BorderColor[0] = 0.f;
		samplerDesc.BorderColor[1] = 0.f;
		samplerDesc.BorderColor[2] = 0.f;
		samplerDesc.BorderColor[3] = 0.f;

		result = RenderAPI::Get()->GetDevice()->CreateSamplerState(&samplerDesc, &m_pSampler);
		RS_D311_ASSERT_CHECK(result, "Failed to create sampler!");
	}
}

void SandboxScene::Selected()
{
}

void SandboxScene::End()
{
	m_Shader.Release();
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
	m_pConstantBuffer->Release();

	m_pTexture->Release();
	m_pTextureSRV->Release();
	m_pSampler->Release();
}

void SandboxScene::FixedTick()
{
}

void SandboxScene::Tick(float dt)
{
	auto display = Display::Get();
	auto renderer = Renderer::Get();
	auto renderAPI = RenderAPI::Get();
	ID3D11DeviceContext* pContext = renderAPI->GetDeviceContext();
	renderer->BeginScene(0.2f, 0.2f, 0.2f, 1.0f);
	
	m_Shader.Bind();

	// Update data
	{
		// Use right-handed coordinate system (+z is towards the viewer!)
		glm::vec3 camPos(0.0, 0.0, 2.0);
		glm::vec3 camDir(0.0, 0.0, -1.0);
		constexpr float fov = glm::pi<float>() / 4.f;
		float nearPlane = 0.01f, farPlane = 100.f;
		float aspectRatio = display->GetAspectRatio();
		//LOG_INFO("w: {}, h: {}, as: {}", display->GetWidth(), display->GetHeight(), aspectRatio);
		m_FrameData.world = glm::rotate(m_FrameData.world, glm::pi<float>() * dt*0.5f, glm::vec3(0.f, 1.f, 0.f));
		m_FrameData.view = glm::lookAtRH(camPos, camPos + camDir, glm::vec3(0.f, 1.f, 0.f));
		m_FrameData.proj = glm::perspectiveRH(fov, aspectRatio, nearPlane, farPlane);

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
}
