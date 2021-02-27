#pragma once

#include "Core/Scene.h"

#include "Loaders/ModelLoader.h"
#include "Renderer/Shader.h"
#include "Utils/Maths.h"

#include "Scenes/Camera.h"

#include "Renderer/Pipeline.h"
#include "Resources/Resources.h"

namespace RS
{
	class MeshScene : public Scene
	{
	public:
		struct FrameData
		{
			glm::mat4 view = glm::mat4(1.f);
			glm::mat4 proj = glm::mat4(1.f);
		};

	public:
		MeshScene();
		~MeshScene() = default;

		void Start() override;

		void Selected() override;

		void Unselected() override;

		void End() override;

		void FixedTick() override;

		void Tick(float dt) override;

	private:
		void UpdateCamera(float dt);
		void DrawRecursiveImGui(int index, ModelResource& model);
		void DrawImGuiAABB(int index, const AABB& aabb);

	private:
		Shader m_Shader;

		ID3D11Buffer* m_pVertexBuffer			= nullptr;
		ID3D11Buffer* m_pIndexBuffer			= nullptr;
		ID3D11Buffer* m_pConstantBufferFrame	= nullptr;
		ID3D11Buffer* m_pConstantBufferMesh		= nullptr;

		FrameData				m_FrameData;
		MeshResource::MeshData	m_MeshData;

		ModelResource*	m_pModel				= nullptr;

		ModelResource*	m_pAssimpModel			= nullptr;

		Camera			m_Camera;

		Pipeline		m_Pipeline;
	};
}