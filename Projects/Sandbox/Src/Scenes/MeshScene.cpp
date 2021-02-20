#include "PreCompiled.h"
#include "MeshScene.h"

#include "Renderer/ShaderHotReloader.h"
#include "Renderer/Renderer.h"
#include "Renderer/DebugRenderer.h"
#include "Core/Display.h"
#include "Core/Input.h"

#include "Utils/Maths.h"

#include <glm/gtx/rotate_vector.hpp>

using namespace RS;

MeshScene::MeshScene() : Scene("Mesh Scene")
{
}

void MeshScene::Start()
{
	glm::vec3 camPos(0.0, 0.0, 2.0);
	glm::vec3 camDir(0.0, 0.0, -1.0);
	constexpr float fov = glm::pi<float>() / 4.f;
	float nearPlane = 0.01f, farPlane = 100.f;
	//LOG_INFO("w: {}, h: {}, as: {}", display->GetWidth(), display->GetHeight(), aspectRatio);
	m_Camera.Init(camPos, camDir, glm::vec3{ 0.f, 1.f, 0.f }, nearPlane, farPlane, fov);

	AttributeLayout layout;
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "POSITION", 0);
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "NORMAL", 0);
	layout.Push(DXGI_FORMAT_R32G32_FLOAT, "TEXCOORD", 0);
	Shader::Descriptor shaderDesc = {};
	shaderDesc.Vertex = "MeshVert.hlsl";
	shaderDesc.Fragment = "MeshFrag.hlsl";
	m_Shader.Load(shaderDesc, layout);
	ShaderHotReloader::AddShader(&m_Shader);

	m_pModel = ModelLoader::Load("Test.obj");

	RS_ASSERT(m_pModel != nullptr, "Could not load model!");

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = (UINT)(sizeof(Model::Vertex) * m_pModel->Vertices.size());
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = m_pModel->Vertices.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pVertexBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create vertex buffer!");
	}

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = (UINT)(sizeof(uint32) * m_pModel->Indices.size());
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = m_pModel->Indices.data();
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

	m_Pipeline.Init();

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	m_Pipeline.SetRasterState(rasterizerDesc);

	auto debugRenderer = DebugRenderer::Get();
	uint32 id = debugRenderer->GenID();
	debugRenderer->PushLine(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), Color::RED, id, true);
	debugRenderer->PushLine(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), Color::GREEN, id, false);
	debugRenderer->PushLine(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), Color::BLUE, id, false);
}

void MeshScene::Selected()
{
}

void MeshScene::End()
{
	m_Pipeline.Release();

	m_Shader.Release();
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
	m_pConstantBuffer->Release();

	delete m_pModel;
}

void MeshScene::FixedTick()
{
}

void MeshScene::Tick(float dt)
{
	UpdateCamera(dt);
	DebugRenderer::Get()->UpdateCamera(m_Camera.GetView(), m_Camera.GetProj());

	m_Pipeline.Bind();

	auto display = Display::Get();
	auto renderer = Renderer::Get();
	auto renderAPI = RenderAPI::Get();
	ID3D11DeviceContext* pContext = renderAPI->GetDeviceContext();
	renderer->BeginScene(0.2f, 0.2f, 0.2f, 1.0f);

	DebugRenderer::Get()->PushPoint(glm::vec3(0.f, 0.6f, 0.f), Color(1.0f, 0.2f, 0.2f));

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
		m_FrameData.world = glm::rotate(glm::scale(glm::vec3(0.2f)), glm::pi<float>() * dt * 0.5f, glm::vec3(0.f, 1.f, 0.f));
		m_FrameData.view = m_Camera.GetView();// glm::lookAtRH(camPos, camPos + camDir, glm::vec3(0.f, 1.f, 0.f));
		m_FrameData.proj = m_Camera.GetProj();// glm::perspectiveRH(fov, aspectRatio, nearPlane, farPlane);

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = pContext->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map constant buffer!");

		FrameData* data = (FrameData*)mappedResource.pData;
		memcpy(data, &m_FrameData, sizeof(FrameData));

		pContext->Unmap(m_pConstantBuffer, 0);
	}

	UINT stride = sizeof(Model::Vertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->DrawIndexed((UINT)m_pModel->Indices.size(), 0, 0);
}

void MeshScene::UpdateCamera(float dt)
{
	auto input = Input::Get();
	glm::vec2 delta = input->GetCursorDelta();

	if (input->IsMBPressed(MB::LEFT))
	{
		float mouseSensitivity = 5.f*dt;
		//m_Camera.SetOrientaion(delta.x* mouseSensitivity, delta.y * mouseSensitivity);

		glm::vec3 target(0.f, 0.f, 0.f);
		glm::vec3 pos = m_Camera.GetPos();
		pos = glm::rotate(pos, -delta.x * mouseSensitivity, glm::vec3(0.f, 1.f, 0.f));
		pos = glm::rotate(pos, -delta.y * mouseSensitivity, m_Camera.GetRight());
		m_Camera.LookAt(pos, target);

		// If the screen changes, update the projection.
		m_Camera.UpdateProj();
	}
}
