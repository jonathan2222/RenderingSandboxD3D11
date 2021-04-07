#include "PreCompiled.h"
#include "MeshScene.h"

#include "Renderer/ShaderHotReloader.h"
#include "Renderer/Renderer.h"
#include "Renderer/DebugRenderer.h"
#include "Renderer/ImGuiRenderer.h"
#include "Core/Display.h"
#include "Core/Input.h"

#include "Utils/Maths.h"

#include "Scenes/CameraUtils.h"

using namespace RS;

MeshScene::MeshScene() : Scene("Mesh Scene")
{
}

void MeshScene::Start()
{
	glm::vec3 camPos(0.0f, 0.5f, 2.0f);
	glm::vec3 camDir(0.0, 0.0, -1.0);
	constexpr float fov = glm::pi<float>() / 4.f;
	float nearPlane = 0.01f, farPlane = 100.f;
	//LOG_INFO("w: {}, h: {}, as: {}", display->GetWidth(), display->GetHeight(), aspectRatio);
	m_Camera.Init(camPos, camDir, glm::vec3{ 0.f, 1.f, 0.f }, nearPlane, farPlane, fov);

	AttributeLayout layout;
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "POSITION", 0);
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "NORMAL", 0);
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "TANGENT", 0);
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "BITANGENT", 0);
	layout.Push(DXGI_FORMAT_R32G32_FLOAT, "TEXCOORD", 0);
	Shader::Descriptor shaderDesc = {};
	shaderDesc.Vertex = "MeshVert.hlsl";
	shaderDesc.Fragment = "MeshFrag.hlsl";
	m_Shader.Load(shaderDesc, layout);
	ShaderHotReloader::AddShader(&m_Shader);

	// Load a model with tinyobj.
	{
		ModelLoadDesc modelLoadDesc = {};
		modelLoadDesc.FilePath = "Test.obj";
		modelLoadDesc.Loader = ModelLoadDesc::Loader::TINYOBJ;
		modelLoadDesc.Flags = ModelLoadDesc::LoaderFlag::LOADER_FLAG_GENERATE_BOUNDING_BOX;
		auto [pModel, handler] = ResourceManager::Get()->LoadModelResource(modelLoadDesc);
		m_pModel = pModel;
	}

	// Load a model with assimp.
	{
		{
			ModelLoadDesc modelLoadDesc = {};
			modelLoadDesc.FilePath = "knight_d_pelegrini.fbx";
			modelLoadDesc.Loader = ModelLoadDesc::Loader::ASSIMP;
			auto [pModel, handler] = ResourceManager::Get()->LoadModelResource(modelLoadDesc);
			m_pAssimpModel = pModel;
		}

		{
			// TODO: The debug rendering of the bounding boxes seems to not work for this model! 
			//	     At least the AABBs for a model closer to the root.
			ModelLoadDesc modelLoadDesc = {};
			modelLoadDesc.FilePath = "SurvivalGuitarBackpack/Survival_BackPack_2.fbx";
			modelLoadDesc.Loader = ModelLoadDesc::Loader::ASSIMP;
			auto [pModel, handler] = ResourceManager::Get()->LoadModelResource(modelLoadDesc);
			m_pBagModel = pModel;
		}
	}

	RS_ASSERT(m_pModel != nullptr, "Could not load model!");

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = (UINT)(sizeof(MeshObject::Vertex) * m_pModel->Meshes[0].Vertices.size());
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = m_pModel->Meshes[0].Vertices.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pVertexBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create vertex buffer!");
	}

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = (UINT)(sizeof(uint32) * m_pModel->Meshes[0].Indices.size());
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = m_pModel->Meshes[0].Indices.data();
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

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pConstantBufferFrame);
		RS_D311_ASSERT_CHECK(result, "Failed to create frame constant buffer!");

		bufferDesc.ByteWidth = sizeof(MeshObject::MeshData);
		data.pSysMem = &m_MeshData;
		result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pConstantBufferMesh);
		RS_D311_ASSERT_CHECK(result, "Failed to create mesh constant buffer!");
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

void MeshScene::Selected()
{
	auto debugRenderer = DebugRenderer::Get();
	debugRenderer->PushLine(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), Color::RED);
	debugRenderer->PushLine(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), Color::GREEN);
	debugRenderer->PushLine(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), Color::BLUE);
}

void MeshScene::Unselected()
{
}

void MeshScene::End()
{
	m_Pipeline.Release();

	m_Shader.Release();
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
	m_pConstantBufferFrame->Release();
	m_pConstantBufferMesh->Release();
}

void MeshScene::FixedTick()
{
}

void MeshScene::Tick(float dt)
{
	CameraUtils::UpdateOrbitCamera(m_Camera);
	DebugRenderer::Get()->UpdateCamera(m_Camera.GetView(), m_Camera.GetProj());

	m_Pipeline.Bind(BindType::BOTH);

	auto display = Display::Get();
	auto renderer = Renderer::Get();
	auto renderAPI = RenderAPI::Get();
	ID3D11DeviceContext* pContext = renderAPI->GetDeviceContext();
	renderer->BeginScene(0.2f, 0.2f, 0.2f, 1.0f);

	DebugRenderer::Get()->PushPoint(glm::vec3(0.f, 0.6f, 0.f), Color(1.0f, 0.2f, 0.2f));
	DebugRenderer::Get()->PushPoint(glm::vec3(0.1f, 0.6f, 0.f), Color(1.0f, 1.0f, 0.2f));
	DebugRenderer::Get()->PushPoint(glm::vec3(0.2f, 0.6f, 0.f), Color(1.0f, 0.2f, 1.0f));

	static uint32 id = DebugRenderer::Get()->GenID();


	m_Shader.Bind();

	// Update data
	{
		static float t = 0.f;
		t += glm::pi<float>() * dt * 0.5f;
		glm::mat4 m = glm::scale(glm::vec3(0.2f));
		m_MeshData.world = glm::rotate(m, t, glm::vec3(0.f, 1.f, 0.f));
		m_FrameData.view = m_Camera.GetView();
		m_FrameData.proj = m_Camera.GetProj();

		glm::vec3 v = glm::rotate(glm::vec3(0.3f, 0.6f, 0.f), -t, glm::vec3(0.f, 1.f, 0.f));
		DebugRenderer::Get()->PushPoint(v, Color::BLUE, id);
		DebugRenderer::Get()->PushPoint(v - glm::vec3(0.f, 0.1f, 0.f), Color::BLUE, id);
		DebugRenderer::Get()->PushPoint(v - glm::vec3(0.f, 0.2f, 0.f), Color::BLUE, id, false);
		DebugRenderer::Get()->PushPoint(v - glm::vec3(0.f, 0.3f, 0.f), Color::BLUE, id, false);

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = pContext->Map(m_pConstantBufferFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map frame constant buffer!");
		FrameData* data = (FrameData*)mappedResource.pData;
		memcpy(data, &m_FrameData, sizeof(FrameData));
		pContext->Unmap(m_pConstantBufferFrame, 0);

		result = pContext->Map(m_pConstantBufferMesh, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		RS_D311_ASSERT_CHECK(result, "Failed to map mesh constant buffer!");
		MeshObject::MeshData* dataMesh = (MeshObject::MeshData*)mappedResource.pData;
		memcpy(dataMesh, &m_MeshData, sizeof(m_MeshData));
		pContext->Unmap(m_pConstantBufferMesh, 0);
	}

	MaterialResource* pMaterial = ResourceManager::Get()->GetResource<MaterialResource>(m_pModel->Meshes[0].MaterialHandler);
	SamplerResource* pSampler = ResourceManager::Get()->GetResource<SamplerResource>(ResourceManager::Get()->DefaultSamplerAnisotropic);
	TextureResource* pAlbedoTexture = ResourceManager::Get()->GetResource<TextureResource>(pMaterial->AlbedoTextureHandler);
	TextureResource* pNormalTexture = ResourceManager::Get()->GetResource<TextureResource>(pMaterial->NormalTextureHandler);

	UINT stride = sizeof(MeshObject::Vertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pContext->VSSetConstantBuffers(0, 1, &m_pConstantBufferMesh);
	pContext->VSSetConstantBuffers(1, 1, &m_pConstantBufferFrame);
	pContext->PSSetShaderResources(0, 1, &pAlbedoTexture->pTextureSRV);
	pContext->PSSetShaderResources(1, 1, &pNormalTexture->pTextureSRV);
	pContext->PSSetSamplers(0, 1, &pSampler->pSampler);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->DrawIndexed((UINT)m_pModel->Meshes[0].Indices.size(), 0, 0);

	// Draw assimp model
	{
		glm::mat4 transform = glm::translate(glm::vec3(1.5f, 0.f, 0.f)) * glm::scale(glm::vec3(0.01f));
		pContext->VSSetConstantBuffers(1, 1, &m_pConstantBufferFrame);
		Renderer::DebugInfo debugInfo = {};
		debugInfo.DrawAABBs = true;
		static uint32 debugInfoID = DebugRenderer::Get()->GenID();
		debugInfo.ID = debugInfoID;
		renderer->Render(*m_pAssimpModel, transform, debugInfo, RenderFlag::RENDER_FLAG_ALBEDO_TEXTURE | RenderFlag::RENDER_FLAG_NORMAL_TEXTURE);
	}

	// Draw assimp model
	{
		glm::mat4 transform = glm::translate(glm::vec3(-1.5f, 0.f, 0.f)) * glm::scale(glm::vec3(0.005f));
		pContext->VSSetConstantBuffers(1, 1, &m_pConstantBufferFrame);
		Renderer::DebugInfo debugInfo = {};
		debugInfo.DrawAABBs = true;
		static uint32 debugInfoID = DebugRenderer::Get()->GenID();
		debugInfo.ID = debugInfoID;
		renderer->Render(*m_pBagModel, transform, debugInfo, RenderFlag::RENDER_FLAG_ALBEDO_TEXTURE | RenderFlag::RENDER_FLAG_NORMAL_TEXTURE);
	}

	// Test Assimp
	{
		ImGuiRenderer::Draw([&]()->void
		{
			static bool s_IsAssimpInspectorActive = true;
			if (ImGui::Begin("Model Inspector", &s_IsAssimpInspectorActive))
			{
				if (ImGui::TreeNode("Loaded Models"))
				{
					DrawRecursiveImGui(0, *m_pModel);
					DrawRecursiveImGui(1, *m_pAssimpModel);
					ImGui::TreePop();
				}
			}
			ImGui::End();
		});
	}
}

void MeshScene::DrawRecursiveImGui(int index, ModelResource& model)
{
	if (ImGui::TreeNode((void*)(intptr_t)index, model.Name.c_str()))
	{
		ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Name: %s", model.Name.c_str());
		if (model.pParent)
			ImGui::Text("Parent: %s", model.pParent->Name.c_str());
		else
			ImGui::Text("Parent: No Parent");
		DrawImGuiAABB(0, model.BoundingBox);
		
		if (model.Meshes.empty() == false)
		{
			if (ImGui::TreeNode((void*)(intptr_t)1, "Meshes"))
			{
				for (uint32 i = 0; i < (uint32)model.Meshes.size(); i++)
				{
					if (ImGui::TreeNode((void*)(intptr_t)i, "Mesh #%d", i))
					{
						MeshObject& mesh = model.Meshes[i];
						ImGui::Text("Is in RAM: ");
						float on = mesh.Vertices.empty() ? 0.f : 1.f;
						ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f-on, on, 0.f, 1.f), "%s", on > 0.5f ? "True" : "False");

						ImGui::Text("Is in GPU: ");
						on = mesh.pVertexBuffer ? 1.f : 0.f;
						ImGui::SameLine(); ImGui::TextColored(ImVec4(1.f - on, on, 0.f, 1.f), "%s", on > 0.5f ? "True" : "False");

						ImGui::Text("Num Vertices: %d", mesh.NumVertices);
						ImGui::Text("Num Indices: %d", mesh.NumIndices);
						DrawImGuiAABB(0, mesh.BoundingBox);
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
		}

		if (model.Children.empty() == false)
		{
			index = 1;
			if (model.Meshes.empty()) index = 2;
			if (ImGui::TreeNode((void*)(intptr_t)index, "Children"))
			{
				for (int i = 0; i < (int)model.Children.size(); i++)
					DrawRecursiveImGui(i, model.Children[i]);
				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}
}

void MeshScene::DrawImGuiAABB(int index, const AABB& aabb)
{
	if (ImGui::TreeNode((void*)(intptr_t)index, "Bounding Box"))
	{
		bool isSame = glm::all(glm::lessThan(glm::abs(aabb.min - aabb.max), glm::vec3(FLT_EPSILON)));
		ImVec4 color(0.f, 1.f, 0.f, 1.f);
		if (isSame) color = ImVec4(1.f, 0.f, 0.f, 1.f);
		ImGui::TextColored(color, "Size: (%f, %f, %f)", aabb.max.x-aabb.min.x, aabb.max.y-aabb.min.y, aabb.max.z-aabb.min.z);
		ImGui::TextColored(color, "Min: (%f, %f, %f)", aabb.min.x, aabb.min.y, aabb.min.z);
		ImGui::TextColored(color, "Max: (%f, %f, %f)", aabb.max.x, aabb.max.y, aabb.max.z);
		ImGui::TreePop();
	}
}
