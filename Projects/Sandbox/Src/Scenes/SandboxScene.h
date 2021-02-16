#pragma once

#include "Core/Scene.h"
#include "Renderer/Shader.h"

#include "Utils/Maths.h"

namespace RS
{
	class SandboxScene : public Scene
	{
	public:
		struct Vertex
		{
			glm::vec4 position;
			glm::vec4 color;
		};

		struct FrameData
		{
			glm::mat4 world = glm::mat4(1.f);
			glm::mat4 view	= glm::mat4(1.f);
			glm::mat4 proj	= glm::mat4(1.f);
		};

	public:
		SandboxScene();
		~SandboxScene() = default;

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
	};
}