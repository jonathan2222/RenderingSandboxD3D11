#pragma once

#include "Core/Scene.h"

#include "Renderer/Shader.h"
#include "Utils/Maths.h"

namespace RS
{
	class TerrainScene : public Scene
	{
	public:
		struct Vertex
		{
			glm::vec4 position;
			glm::vec4 normal;
			glm::vec2 uv;
		};

		struct FrameData
		{
			glm::mat4 world = glm::mat4(1.f);
			glm::mat4 view = glm::mat4(1.f);
			glm::mat4 proj = glm::mat4(1.f);
		};

	public:
		TerrainScene();
		~TerrainScene() = default;

		void Start() override;

		void Selected() override;

		void Unselected() override;

		void End() override;

		void FixedTick() override;

		void Tick(float dt) override;

	private:
		Shader m_Shader;

		ID3D11Buffer* m_pVertexBuffer = nullptr;
		ID3D11Buffer* m_pIndexBuffer = nullptr;
		ID3D11Buffer* m_pConstantBuffer = nullptr;

		ID3D11Texture2D* m_pTexture = nullptr;
		ID3D11ShaderResourceView* m_pTextureSRV = nullptr;
		ID3D11SamplerState* m_pSampler = nullptr;

		FrameData m_FrameData;
	};
}