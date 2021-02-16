#include "PreCompiled.h"
#include "SandboxScene.h"

#include "Renderer/ShaderHotReloader.h"
#include "Renderer/Renderer.h"
#include "Core/Display.h"

#include "Utils/Maths.h"

using namespace RS;

SandboxScene::SandboxScene() : Scene("SandboxScene")
{
}

void SandboxScene::Start()
{
	AttributeLayout layout;
	layout.Push(DXGI_FORMAT_R32G32B32A32_FLOAT, "POSITION", 0);
	layout.Push(DXGI_FORMAT_R32G32B32A32_FLOAT, "COLOR", 0);
	m_Shader.Load("Test", RS_SHADER_VERTEX_AND_FRAGMENT, layout);
	ShaderHotReloader::AddShader(&m_Shader);

	std::vector<Vertex> vertices =
	{
		{glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f), glm::vec4(1.f, 0.f, 0.f, 1.f)},
		{glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f), glm::vec4(0.f, 1.f, 0.f, 1.f)},
		{glm::vec4(0.5f, 0.5f, 0.0f, 1.0f), glm::vec4(0.f, 0.f, 1.f, 1.f)},
		{glm::vec4(0.5f, -0.5f, 0.0f, 1.0f), glm::vec4(1.f, 1.f, 1.f, 1.f)}
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
	//renderer->BeginScene(0.2f, 0.2f, 0.2f, 1.0f);

	m_Shader.Bind();
	renderer->SetViewport(0.f, 0.f, static_cast<float>(display->GetWidth()), static_cast<float>(display->GetHeight()));

	// Update data
	{
		// Use right-handed coordinate system (+z is towards the viewer!)
		glm::vec3 camPos(0.0, 0.0, 5.0);
		glm::vec3 camDir(0.0, 0.0, -1.0);
		float fov = glm::pi<float>() / 4.f;
		float nearPlane = 0.01f, farPlane = 100.f;
		float aspectRatio = display->GetAspectRatio();

		m_FrameData.world = glm::rotate(m_FrameData.world, glm::pi<float>() * dt, glm::vec3(0.f, 1.f, 0.f));
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

	pContext->DrawIndexed(6, 0, 0);
}
