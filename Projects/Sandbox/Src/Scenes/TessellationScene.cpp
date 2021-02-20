#include "PreCompiled.h"
#include "TessellationScene.h"

#include "Renderer/DebugRenderer.h"
#include "Renderer/ShaderHotReloader.h"
#include "Renderer/Renderer.h"
#include "Renderer/ImGuiRenderer.h"

#include "Core/Display.h"
#include "Core/Input.h"

#include "Utils/Maths.h"

#include <glm/gtx/rotate_vector.hpp>


using namespace RS;

TessellationScene::TessellationScene() : Scene("Tessellation Scene")
{
}

void TessellationScene::Start()
{
	glm::vec3 camPos(0.0f, 0.5f, 2.0f);
	glm::vec3 camDir(0.0, 0.0, -1.0);
	constexpr float fov = glm::pi<float>() / 4.f;
	float nearPlane = 0.01f, farPlane = 100.f;
	m_Camera.Init(camPos, camDir, glm::vec3{ 0.f, 1.f, 0.f }, nearPlane, farPlane, fov);

	AttributeLayout layout;
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "POSITION", 0);
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "NORMAL", 0);
	layout.Push(DXGI_FORMAT_R32G32_FLOAT, "TEXCOORD", 0);
	{
		Shader::Descriptor shaderDesc = {};
		shaderDesc.Vertex = "TessellationScene/TessVert.hlsl";
		shaderDesc.Hull = "TessellationScene/TessTriHull.hlsl";
		shaderDesc.Domain = "TessellationScene/TessTriDomain.hlsl";
		shaderDesc.Fragment = "TessellationScene/TessFrag.hlsl";
		m_TriShader.Load(shaderDesc, layout);
		ShaderHotReloader::AddShader(&m_TriShader);
	}
	{
		Shader::Descriptor shaderDesc = {};
		shaderDesc.Vertex = "TessellationScene/TessVert.hlsl";
		shaderDesc.Hull = "TessellationScene/TessQuadHull.hlsl";
		shaderDesc.Domain = "TessellationScene/TessQuadDomain.hlsl";
		shaderDesc.Fragment = "TessellationScene/TessFrag.hlsl";
		m_QuadShader.Load(shaderDesc, layout);
		ShaderHotReloader::AddShader(&m_QuadShader);
	}

	static const float H = 1.f;
	static const float W = 1.f;
	static const float D = 1.f;
	std::vector<Vertex> m_Vertices = 
	{
		{glm::vec3(-W * .5f, 0.f, D * .5f), glm::vec3(0.f, 1.f, 0.f)},
		{glm::vec3(-W * .5f, 0.f, -D * .5f), glm::normalize(glm::vec3(0.f, 1.f, 1.f))},
		{glm::vec3(-W * .5f, H, -D * .5f), glm::vec3(0.f, 0.f, 1.f)},
		{glm::vec3(W * .5f, H, -D * .5f), glm::vec3(0.f, 0.f, 1.f)},
		{glm::vec3(W * .5f, 0.f, -D * .5f), glm::normalize(glm::vec3(0.f, 1.f, 1.f))},
		{glm::vec3(W * .5f, 0.f, D * .5f), glm::vec3(0.f, 1.f, 0.f)},
	};
	std::vector<uint32> m_TriIndices = 
	{
		0, 1, 4, 4, 5, 0,
		1, 2, 3, 3, 4, 1
	};
	m_NumTriIndices = m_TriIndices.size();

	std::vector<uint32> m_QuadIndices =
	{
		0, 1, 5, 4,
		1, 2, 4, 3
	};
	m_NumQuadIndices = m_QuadIndices.size();

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = (UINT)(sizeof(Vertex) * m_Vertices.size());
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = m_Vertices.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pVertexBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create vertex buffer!");
	}

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = (UINT)(sizeof(uint32) * m_TriIndices.size());
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = m_TriIndices.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pTriIndexBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create triangle index buffer!");

		bufferDesc.ByteWidth = (UINT)(sizeof(uint32) * m_QuadIndices.size());
		data.pSysMem = m_QuadIndices.data();
		result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pQuadIndexBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create quad index buffer!");
	}

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(glm::mat4);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		glm::mat4 identity(1.f);
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = &identity;
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pVSConstantBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to VS create constant buffer!");

		bufferDesc.ByteWidth = sizeof(CameraData);
		data.pSysMem = &m_CameraData;
		result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pDSConstantBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to DS create constant buffer!");

		glm::vec4 v(1.f);
		bufferDesc.ByteWidth = sizeof(glm::vec4);
		data.pSysMem = &v;
		result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pHSConstantBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to HS create constant buffer!");
	}

	m_Pipeline.Init();

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	m_Pipeline.SetRasterState(rasterizerDesc);
}

void TessellationScene::Selected()
{
	auto debugRenderer = DebugRenderer::Get();
	debugRenderer->PushLine(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), Color::RED);
	debugRenderer->PushLine(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), Color::GREEN);
	debugRenderer->PushLine(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), Color::BLUE);
}

void TessellationScene::Unselected()
{
}

void TessellationScene::End()
{
	m_Pipeline.Release();

	m_TriShader.Release();
	m_QuadShader.Release();
	m_pVertexBuffer->Release();
	m_pTriIndexBuffer->Release();
	m_pQuadIndexBuffer->Release();
	m_pVSConstantBuffer->Release();
	m_pHSConstantBuffer->Release();
	m_pDSConstantBuffer->Release();
}

void TessellationScene::FixedTick()
{
}

void TessellationScene::Tick(float dt)
{
	ToggleWireframe();

	UpdateCamera(dt);
	DebugRenderer::Get()->UpdateCamera(m_Camera.GetView(), m_Camera.GetProj());

	m_Pipeline.Bind();

	auto display = Display::Get();
	auto renderer = Renderer::Get();
	auto renderAPI = RenderAPI::Get();
	ID3D11DeviceContext* pContext = renderAPI->GetDeviceContext();
	renderer->BeginScene(0.2f, 0.2f, 0.2f, 1.0f);

	static uint32 id = DebugRenderer::Get()->GenID();

	// Update data
	{
		// Shift the first model to the right.
		glm::mat4 world = glm::translate(glm::vec3(0.5f, 0.f, 0.f));

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = pContext->Map(m_pVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map VS constant buffer!");
		memcpy(mappedResource.pData, &world, sizeof(world));
		pContext->Unmap(m_pVSConstantBuffer, 0);

		m_CameraData.view = m_Camera.GetView();
		m_CameraData.proj = m_Camera.GetProj();
		result = pContext->Map(m_pDSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map DS constant buffer!");
		memcpy(mappedResource.pData, &m_CameraData, sizeof(m_CameraData));
		pContext->Unmap(m_pDSConstantBuffer, 0);

		static const float s_MaxTessFactor = 50.f;
		static float s_Outer = s_MaxTessFactor*.5f, s_Inner = s_MaxTessFactor * .5f;
		ImGuiRenderer::Draw([&]()
		{
			static bool s_ApplySame = true;
			static bool s_TessPanelActive = true;
			if (ImGui::Begin("Tesselation Params", &s_TessPanelActive))
			{
				ImGui::Checkbox("Both", &s_ApplySame);

				if (s_ApplySame)
				{
					ImGui::SliderFloat("Both", &s_Inner, 1.0f, s_MaxTessFactor, "Factor = %.3f");
					s_Outer = s_Inner;
				}
				else
				{
					ImGui::SliderFloat("OuterTessFactor", &s_Outer, 1.0f, s_MaxTessFactor, "Factor = %.3f");
					ImGui::SliderFloat("InnerTessFactor", &s_Inner, 1.0f, s_MaxTessFactor, "Factor = %.3f");
				}
			}
			ImGui::End();
		});

		glm::vec4 v(s_Outer, s_Inner, 0.f, 0.f);
		result = pContext->Map(m_pHSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map HS constant buffer!");
		memcpy(mappedResource.pData, &v, sizeof(v));
		pContext->Unmap(m_pHSConstantBuffer, 0);
	}

	m_TriShader.Bind();
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	{
		pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		pContext->IASetIndexBuffer(m_pTriIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		pContext->VSSetConstantBuffers(0, 1, &m_pVSConstantBuffer);
		pContext->HSSetConstantBuffers(0, 1, &m_pHSConstantBuffer);
		pContext->DSSetConstantBuffers(0, 1, &m_pDSConstantBuffer);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		pContext->DrawIndexed((UINT)m_NumTriIndices, 0, 0);
	}
	
	{ // Shift the model to the Left
		glm::mat4 world = glm::translate(glm::vec3(-0.5f, 0.f, 0.f));

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = pContext->Map(m_pVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map VS constant buffer!");
		memcpy(mappedResource.pData, &world, sizeof(world));
		pContext->Unmap(m_pVSConstantBuffer, 0);
	}
	
	m_QuadShader.Bind();
	{
		pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		pContext->IASetIndexBuffer(m_pQuadIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		pContext->VSSetConstantBuffers(0, 1, &m_pVSConstantBuffer);
		pContext->HSSetConstantBuffers(0, 1, &m_pHSConstantBuffer);
		pContext->DSSetConstantBuffers(0, 1, &m_pDSConstantBuffer);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		pContext->DrawIndexed((UINT)m_NumQuadIndices, 0, 0);
	}
}

void TessellationScene::UpdateCamera(float dt)
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

		float mouseSensitivity = 5.f * dt;
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

void TessellationScene::ToggleWireframe()
{
	static bool stateW = true;
	static bool first = true;
	KeyState state = Input::Get()->GetKeyState(Key::W);
	if (state == KeyState::PRESSED && first)
	{
		first = false;
		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.AntialiasedLineEnable = false;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		if (stateW)
		{
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		}
		else
		{
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
		}
		m_Pipeline.SetRasterState(rasterizerDesc);

		stateW = !stateW;
	}
	else if (state == KeyState::RELEASED)
	{
		first = true;
	}
}
