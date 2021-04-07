#include "PreCompiled.h"
#include "TessellationScene.h"

#include "Renderer/DebugRenderer.h"
#include "Renderer/ShaderHotReloader.h"
#include "Renderer/Renderer.h"
#include "Renderer/ImGuiRenderer.h"

#include "Core/Display.h"
#include "Core/Input.h"

#include "Utils/Maths.h"

#include "Scenes/CameraUtils.h"

#include "Core/ResourceManager.h"


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
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "TANGENT", 0);
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
		{glm::vec3(-W * .5f, 0.f, D * .5f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f), glm::vec2(0.f, 1.f)},
		{glm::vec3(-W * .5f, 0.f, -D * .5f), glm::normalize(glm::vec3(0.f, 1.f, 1.f)), glm::vec3(0.f), glm::vec2(0.f, 0.f)},
		{glm::vec3(-W * .5f, H, -D * .5f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f), glm::vec2(0.f, -1.f)},
		{glm::vec3(W * .5f, H, -D * .5f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f), glm::vec2(1.f, -1.f)},
		{glm::vec3(W * .5f, 0.f, -D * .5f), glm::normalize(glm::vec3(0.f, 1.f, 1.f)), glm::vec3(0.f), glm::vec2(1.f, 0.f)},
		{glm::vec3(W * .5f, 0.f, D * .5f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f), glm::vec2(1.f, 1.f)},
	};
	std::vector<uint32> m_TriIndices = 
	{
		0, 1, 4, 4, 5, 0,
		1, 2, 3, 3, 4, 1
	};
	CalcTangents(m_Vertices, m_TriIndices);
	m_NumTriIndices = (uint32)m_TriIndices.size();

	std::vector<uint32> m_QuadIndices =
	{
		0, 1, 5, 4,
		1, 2, 4, 3
	};
	m_NumQuadIndices = (uint32)m_QuadIndices.size();

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
		RS_D311_ASSERT_CHECK(result, "Failed to create VS constant buffer!");

		bufferDesc.ByteWidth = sizeof(CameraData);
		data.pSysMem = &m_CameraData;
		result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pDSConstantBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create DS constant buffer!");

		glm::vec4 v(1.f);
		bufferDesc.ByteWidth = sizeof(glm::vec4);
		data.pSysMem = &v;
		result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pHSConstantBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create HS constant buffer!");

		bufferDesc.ByteWidth = sizeof(PSData);
		data.pSysMem = &m_PSData;
		result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pPSConstantBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create PS constant buffer!");
	}

	{
		CreateTexture("HexagonRocks/Rocks_Hexagons_001_basecolor.jpg", m_pAlbedoTexture, m_pAlbedoTextureView);
		CreateTexture("HexagonRocks/Rocks_Hexagons_001_height.png", m_pDisplacementTexture, m_pDisplacementTextureView);
		CreateTexture("HexagonRocks/Rocks_Hexagons_001_normal.jpg", m_pNormalTexture, m_pNormalTextureView);

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

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateSamplerState(&samplerDesc, &m_pSampler);
		RS_D311_ASSERT_CHECK(result, "Failed to create sampler!");
	}

	m_Pipeline.Init();

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.CullMode = m_IsWireframeEnabled ? D3D11_CULL_NONE : D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.FillMode = m_IsWireframeEnabled ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
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
	m_pPSConstantBuffer->Release();

	m_pAlbedoTexture->Release();
	m_pAlbedoTextureView->Release();
	m_pNormalTexture->Release();
	m_pNormalTextureView->Release();
	m_pDisplacementTexture->Release();
	m_pDisplacementTextureView->Release();
	m_pSampler->Release();

	// TODO: Release the other textures and their views!
}

void TessellationScene::FixedTick()
{
}

void TessellationScene::Tick(float dt)
{
	ToggleWireframe(false);

	CameraUtils::UpdateOrbitCamera(m_Camera);
	DebugRenderer::Get()->UpdateCamera(m_Camera.GetView(), m_Camera.GetProj());

	m_Pipeline.Bind(BindType::BOTH);

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

		static const float s_MaxTessFactor = 64.f;
		static float s_Outer = s_MaxTessFactor*.5f, s_Inner = s_MaxTessFactor * .5f;
		static float s_PhongAlpha = 0.75f, s_Height = 0.05f;
		static bool s_ToggleWireframe = false;
		static bool s_IsWireframeEnabled = false;
		s_IsWireframeEnabled = m_IsWireframeEnabled;
		ImGuiRenderer::Draw([&]()
		{
			static bool s_ApplySame = true;
			static bool s_TessPanelActive = true;
			if (ImGui::Begin("Tesselation Params", &s_TessPanelActive))
			{
				ImGui::SliderFloat("Displacement Factor", &s_Height, 0.f, 1.f, "Height = %.3f");
				ImGui::SliderFloat("Phong Tessellation", &s_PhongAlpha, 0.0f, 1.0f, "Alpha = %.3f");
				s_ToggleWireframe = ImGui::Checkbox("Wireframe", &s_IsWireframeEnabled);
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

		if (s_ToggleWireframe)
		{
			s_ToggleWireframe = false;
			ToggleWireframe(true);
		}

		m_CameraData.world = world;
		m_CameraData.view = m_Camera.GetView();
		m_CameraData.proj = m_Camera.GetProj();
		m_CameraData.info = glm::vec4(s_PhongAlpha, s_Height, 0.f, 0.f);
		result = pContext->Map(m_pDSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map DS constant buffer!");
		memcpy(mappedResource.pData, &m_CameraData, sizeof(m_CameraData));
		pContext->Unmap(m_pDSConstantBuffer, 0);

		glm::vec4 v(s_Outer, s_Inner, 0.f, 0.f);
		result = pContext->Map(m_pHSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map HS constant buffer!");
		memcpy(mappedResource.pData, &v, sizeof(v));
		pContext->Unmap(m_pHSConstantBuffer, 0);

		static glm::vec3 s_LightPos = glm::vec3(0.2f, 0.1f, 0.f);
		glm::vec3 target(0.f, 0.1f, 0.f);
		s_LightPos = target + glm::rotate(s_LightPos - target, dt, glm::vec3(0.f, 1.f, 0.f));
		DebugRenderer::Get()->PushPoint(s_LightPos, Color::WHITE, id);

		m_PSData.cameraPos = glm::vec4(m_Camera.GetPos(), 1.f);
		m_PSData.cameraDir = glm::vec4(m_Camera.GetDir(), 0.f);
		m_PSData.lightPos = glm::vec4(s_LightPos, 1.f);
		result = pContext->Map(m_pPSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map PS constant buffer!");
		memcpy(mappedResource.pData, &m_PSData, sizeof(m_PSData));
		pContext->Unmap(m_pPSConstantBuffer, 0);
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
		pContext->DSSetSamplers(0, 1, &m_pSampler);
		pContext->DSSetShaderResources(0, 1, &m_pDisplacementTextureView);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		pContext->PSSetSamplers(0, 1, &m_pSampler);
		pContext->PSSetShaderResources(0, 1, &m_pNormalTextureView);
		pContext->PSSetShaderResources(1, 1, &m_pAlbedoTextureView);
		pContext->PSSetConstantBuffers(0, 1, &m_pPSConstantBuffer);
		pContext->DrawIndexed((UINT)m_NumTriIndices, 0, 0);
	}
	
	{ // Shift the model to the Left
		glm::mat4 world = glm::translate(glm::vec3(-0.5f, 0.f, 0.f));

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = pContext->Map(m_pVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map VS constant buffer!");
		memcpy(mappedResource.pData, &world, sizeof(world));
		pContext->Unmap(m_pVSConstantBuffer, 0);

		m_CameraData.world = world;
		result = pContext->Map(m_pDSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map DS constant buffer!");
		memcpy(mappedResource.pData, &m_CameraData, sizeof(m_CameraData));
		pContext->Unmap(m_pDSConstantBuffer, 0);
	}
	
	m_QuadShader.Bind();
	{
		pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		pContext->IASetIndexBuffer(m_pQuadIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		pContext->VSSetConstantBuffers(0, 1, &m_pVSConstantBuffer);
		pContext->HSSetConstantBuffers(0, 1, &m_pHSConstantBuffer);
		pContext->DSSetConstantBuffers(0, 1, &m_pDSConstantBuffer);
		pContext->DSSetSamplers(0, 1, &m_pSampler);
		pContext->DSSetShaderResources(0, 1, &m_pDisplacementTextureView);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		pContext->PSSetSamplers(0, 1, &m_pSampler);
		pContext->PSSetShaderResources(0, 1, &m_pNormalTextureView);
		pContext->PSSetShaderResources(1, 1, &m_pAlbedoTextureView);
		pContext->PSSetConstantBuffers(0, 1, &m_pPSConstantBuffer);
		pContext->DrawIndexed((UINT)m_NumQuadIndices, 0, 0);
	}
}

void TessellationScene::ToggleWireframe(bool forceToggle)
{
	static bool first = true;
	KeyState state = Input::Get()->GetKeyState(Key::W);
	if (state == KeyState::PRESSED && first || forceToggle)
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
		if (m_IsWireframeEnabled)
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

		m_IsWireframeEnabled = !m_IsWireframeEnabled;
	}
	else if (state == KeyState::RELEASED)
	{
		first = true;
	}
}

void TessellationScene::CreateTexture(const std::string& fileName, ID3D11Texture2D*& pTexture, ID3D11ShaderResourceView*& pTextureView)
{
	ImageLoadDesc imageDesc = {};
	imageDesc.File.Path		= fileName;
	imageDesc.Name			= imageDesc.File.Path;
	imageDesc.NumChannels	= ImageLoadDesc::Channels::RGBA;
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

	HRESULT result = RenderAPI::Get()->GetDevice()->CreateTexture2D(&textureDesc, &data, &pTexture);
	RS_D311_ASSERT_CHECK(result, "Failed to create the albedo texture!");

	ResourceManager::Get()->FreeResource(pImageResource);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	result = RenderAPI::Get()->GetDevice()->CreateShaderResourceView(pTexture, &srvDesc, &pTextureView);
	RS_D311_ASSERT_CHECK(result, "Failed to create texture RSV!");
}

void TessellationScene::CalcTangents(std::vector<Vertex>& vertices, const std::vector<uint32>& indices)
{
	const uint32 size = (uint32)indices.size();
	for (uint32 i = 0; i < size; i += 3)
	{
		Vertex& v0 = vertices[(size_t)indices[(size_t)i+0]];
		Vertex& v1 = vertices[(size_t)indices[(size_t)i+1]];
		Vertex& v2 = vertices[(size_t)indices[(size_t)i+2]];

		glm::vec3 edge1 = v1.Position - v0.Position;
		glm::vec3 edge2 = v2.Position - v0.Position;
		glm::vec2 uv1 = v1.UV - v0.UV;
		glm::vec2 uv2 = v2.UV - v0.UV;

		float invDet = 1.f / (uv1.x * uv2.y - uv2.x * uv1.y);
		glm::mat2x3 edgeMat2x3(edge1, edge2);
		glm::mat3x2 edgeMat = glm::transpose(edgeMat2x3);
		glm::vec3 tangent = invDet * glm::vec2(uv2.y, -uv1.y) * edgeMat;

		v0.Tangent = tangent;
		v1.Tangent = tangent;
		v2.Tangent = tangent;
	}
}
