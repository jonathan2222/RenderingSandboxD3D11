#include "PreCompiled.h"
#include "PBRScene.h"

#include "Renderer/ShaderHotReloader.h"
#include "Renderer/Renderer.h"
#include "Renderer/DebugRenderer.h"
#include "Renderer/ImGuiRenderer.h"
#include "Core/Display.h"
#include "Core/Input.h"

#include "Utils/Maths.h"

#include <glm/gtx/rotate_vector.hpp>

using namespace RS;

PBRScene::PBRScene() : Scene("PBR Scene")
{
}

void PBRScene::Start()
{
	glm::vec3 camPos(0.0f, 0.5f, 2.0f);
	glm::vec3 camDir(0.0, 0.0, -1.0);
	constexpr float fov = glm::pi<float>() / 4.f;
	float nearPlane = 0.01f, farPlane = 100.f;
	m_Camera.Init(camPos, camDir, glm::vec3{ 0.f, 1.f, 0.f }, nearPlane, farPlane, fov);

	AttributeLayout layout;
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "POSITION", 0);
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "NORMAL", 0);
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "TANGENT", 0);
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "BITANGENT", 0);
	layout.Push(DXGI_FORMAT_R32G32_FLOAT, "TEXCOORD", 0);
	Shader::Descriptor shaderDesc = {};
	shaderDesc.Vertex = "PBRScene/PBRVert.hlsl";
	shaderDesc.Fragment = "PBRScene/PBRFrag.hlsl";
	m_Shader.Load(shaderDesc, layout);
	ShaderHotReloader::AddShader(&m_Shader);

	// Load a model with assimp.
	{
		ModelLoadDesc modelLoadDesc = {};
		modelLoadDesc.FilePath = "FlightHelmet/FlightHelmet.gltf";
		modelLoadDesc.Loader = ModelLoadDesc::Loader::ASSIMP;
		auto [pModel, handler] = ResourceManager::Get()->LoadModelResource(modelLoadDesc);
		m_pModel = pModel;
	}

	RS_ASSERT(m_pModel != nullptr, "Could not load model!");

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

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pConstantBufferFrame);
		RS_D311_ASSERT_CHECK(result, "Failed to create frame constant buffer!");
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
}

void PBRScene::Selected()
{
	auto debugRenderer = DebugRenderer::Get();
	debugRenderer->PushLine(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), Color::RED);
	debugRenderer->PushLine(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), Color::GREEN);
	debugRenderer->PushLine(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), Color::BLUE);
}

void PBRScene::Unselected()
{
}

void PBRScene::End()
{
	m_Pipeline.Release();

	m_Shader.Release();
	m_pConstantBufferFrame->Release();
}

void PBRScene::FixedTick()
{
}

void PBRScene::Tick(float dt)
{
	DrawImGui();

	UpdateCamera(dt);
	DebugRenderer::Get()->UpdateCamera(m_Camera.GetView(), m_Camera.GetProj());

	m_Pipeline.Bind();

	auto display = Display::Get();
	auto renderer = Renderer::Get();
	auto renderAPI = RenderAPI::Get();
	ID3D11DeviceContext* pContext = renderAPI->GetDeviceContext();
	renderer->BeginScene(0.2f, 0.2f, 0.2f, 1.0f);

	m_Shader.Bind();

	// Update data
	{
		m_FrameData.view = m_Camera.GetView();
		m_FrameData.proj = m_Camera.GetProj();

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = pContext->Map(m_pConstantBufferFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map frame constant buffer!");
		FrameData* data = (FrameData*)mappedResource.pData;
		memcpy(data, &m_FrameData, sizeof(FrameData));
		pContext->Unmap(m_pConstantBufferFrame, 0);
	}

	// Draw assimp model
	{
		glm::mat4 transform = glm::translate(glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::vec3(2.f)) * glm::rotate(glm::pi<float>(), glm::vec3(0.f, 1.f, 0.f));
		pContext->VSSetConstantBuffers(1, 1, &m_pConstantBufferFrame);
		Renderer::DebugInfo debugInfo = {};
		debugInfo.DrawAABBs = false;
		static uint32 debugInfoID = DebugRenderer::Get()->GenID();
		debugInfo.ID = debugInfoID;
		debugInfo.RenderMode = (uint32)m_RenderMode;
		renderer->RenderWithMaterial(*m_pModel, transform, debugInfo);
	}
}

void PBRScene::UpdateCamera(float dt)
{
	/*
	*	Camera
	*	Type: Orbit
	*	Controlls:
	*		Pan:			Hold L_SHIFT and drag with the LMB.
	*		Zoom:			Hold L_CTL and drag with the LBM up and down.
	*		Orbit			Drag with the LMB.
	*		Reset target:	Press the 'C' key.
	*/

	auto input = Input::Get();
	glm::vec2 delta = input->GetCursorDelta();

	static const glm::vec3 s_StartingTarget = glm::vec3(0.f, 0.5f, 0.f);
	static glm::vec3 s_Target = s_StartingTarget;
	uint32 s_CameraState = 0;
	if (input->IsKeyPressed(Key::LEFT_CONTROL))		s_CameraState = 1;
	else if (input->IsKeyPressed(Key::LEFT_SHIFT))	s_CameraState = 2;
	else											s_CameraState = 0;

	if (input->IsKeyPressed(Key::C))
	{
		s_Target = s_StartingTarget;
		glm::vec3 pos = s_StartingTarget + glm::vec3(0.f, 0.f, 2.0f);
		m_Camera.LookAt(pos, s_Target);
	}

	if (input->IsMBPressed(MB::LEFT))
	{

		float mouseSensitivity = 0.005f;
		float zoomFactor = 2.f;

		glm::vec3 pos = m_Camera.GetPos();
		if (s_CameraState == 0) // Orbit
		{
			glm::vec3 v = pos - s_Target;
			v = glm::rotate(v, -delta.x * mouseSensitivity, glm::vec3(0.f, 1.f, 0.f));
			v = glm::rotate(v, -delta.y * mouseSensitivity, m_Camera.GetRight());
			pos = s_Target + v;
		}
		else if (s_CameraState == 1) // Zoom
		{
			float zoom = delta.y * mouseSensitivity * zoomFactor;
			glm::vec3 dir = glm::normalize(s_Target - pos);
			dir *= zoom;
			pos += dir;
		}
		else if (s_CameraState == 2) // Pan
		{
			glm::vec3 right = m_Camera.GetRight();
			glm::vec3 up = m_Camera.GetUp();
			glm::vec3 offset = right * -delta.x + up * delta.y;
			offset *= mouseSensitivity;
			pos += offset;
			s_Target += offset;
		}
		m_Camera.LookAt(pos, s_Target);

		// Update the projection for the possibility that the screen size has been changed.
		m_Camera.UpdateProj();
	}
}

void PBRScene::DrawImGui()
{
	static bool s_MaterialDebugWindow = true;
	ImGuiRenderer::Draw([&]()
	{
		if (ImGui::Begin("Material Debugger", &s_MaterialDebugWindow))
		{
			ImGui::RadioButton("Normal Rendering", &m_RenderMode, 0);
			ImGui::RadioButton("Only Albedo", &m_RenderMode, 1);
			ImGui::RadioButton("Only Normal", &m_RenderMode, 2);
			ImGui::RadioButton("Only AO", &m_RenderMode, 3);
			ImGui::RadioButton("Only Mettalic", &m_RenderMode, 4);
			ImGui::RadioButton("Only Roughness", &m_RenderMode, 5);
			ImGui::RadioButton("Only Combined Metallic-Roughness", &m_RenderMode, 6);
		}
		ImGui::End();
	});
}
