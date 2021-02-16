#pragma once

#include "Core/Scene.h"
#include "Renderer/Shader.h"

#include <glm/glm.hpp>

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
	};
}