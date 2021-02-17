#pragma once

#include "Core/Scene.h"

#include "Loaders/ModelLoader.h"
#include "Renderer/Shader.h"
#include "Utils/Maths.h"

namespace RS
{
	class MeshScene : public Scene
	{
	public:
		struct FrameData
		{
			glm::mat4 world = glm::mat4(1.f);
			glm::mat4 view = glm::mat4(1.f);
			glm::mat4 proj = glm::mat4(1.f);
		};

	public:
		MeshScene();
		~MeshScene() = default;

		void Start() override;

		void Selected() override;

		void End() override;

		void FixedTick() override;

		void Tick(float dt) override;

	private:
		Shader m_Shader;

		ID3D11Buffer* m_pVertexBuffer = nullptr;
		ID3D11Buffer* m_pIndexBuffer = nullptr;
		ID3D11Buffer* m_pConstantBuffer = nullptr;

		FrameData m_FrameData;

		Model* m_pModel = nullptr;
	};
}