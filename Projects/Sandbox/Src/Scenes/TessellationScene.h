#pragma once

#include "Core/Scene.h"

#include "Loaders/ModelLoader.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Shader.h"

#include "Scenes/Camera.h"

namespace RS
{
	class TessellationScene : public Scene
	{
	public:
		struct CameraData
		{
			glm::mat4 view	= glm::mat4(1.f);
			glm::mat4 proj	= glm::mat4(1.f);
		};

		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
		};

	public:
		TessellationScene();
		~TessellationScene() = default;

		void Start() override;

		void Selected() override;

		void Unselected() override;

		void End() override;

		void FixedTick() override;

		void Tick(float dt) override;

	private:
		void UpdateCamera(float dt);
		void ToggleWireframe();

	private:
		Shader m_Shader;

		ID3D11Buffer* m_pVertexBuffer = nullptr;
		ID3D11Buffer* m_pIndexBuffer = nullptr;
		ID3D11Buffer* m_pVSConstantBuffer = nullptr;
		ID3D11Buffer* m_pHSConstantBuffer = nullptr;
		ID3D11Buffer* m_pDSConstantBuffer = nullptr;

		uint32		m_NumIndices = 0;

		CameraData	m_CameraData;

		Camera		m_Camera;

		Pipeline	m_Pipeline;
	};
}