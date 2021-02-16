#include "PreCompiled.h"
#include "SandboxScene.h"

#include "Renderer/ShaderHotReloader.h"
#include "Renderer/Renderer.h"
#include "Core/Display.h"

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

	Vertex vertices[3] = 
	{
		{glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f), glm::vec4(1.f, 0.f, 0.f, 1.f)},
		{glm::vec4(0.0f, 0.5f, 0.0f, 1.0f), glm::vec4(0.f, 1.f, 0.f, 1.f)},
		{glm::vec4(0.5f, -0.5f, 0.0f, 1.0f), glm::vec4(0.f, 0.f, 1.f, 1.f)},
	};

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertices;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pVertexBuffer);
	RS_D311_ASSERT_CHECK(result, "Failed to create vertex buffer!");
}

void SandboxScene::Selected()
{
}

void SandboxScene::End()
{
	m_Shader.Release();
	m_pVertexBuffer->Release();
}

void SandboxScene::FixedTick()
{
}

void SandboxScene::Tick(float dt)
{
	auto display = Display::Get();
	auto renderer = Renderer::Get();
	auto renderAPI = RenderAPI::Get();
	//renderer->BeginScene(0.2f, 0.2f, 0.2f, 1.0f);

	m_Shader.Bind();
	renderer->SetViewport(0.f, 0.f, static_cast<float>(display->GetWidth()), static_cast<float>(display->GetHeight()));

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	renderAPI->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	renderAPI->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	renderAPI->GetDeviceContext()->Draw(3, 0);
}
