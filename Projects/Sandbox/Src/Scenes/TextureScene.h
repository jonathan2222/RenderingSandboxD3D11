#pragma once

#include "Core/Scene.h"
#include "Renderer/Shader.h"

#include "Utils/Maths.h"

#include "Renderer/Pipeline.h"
#include "Resources/Resources.h"

#include "Scenes/Camera.h"

namespace RS
{
	class TextureScene : public Scene
	{
	public:
		struct Vertex
		{
			glm::vec4 position;
			glm::vec4 color;
			glm::vec2 uv;
		};

		struct FrameData
		{
			glm::mat4 world = glm::mat4(1.f);
			glm::mat4 view	= glm::mat4(1.f);
			glm::mat4 proj	= glm::mat4(1.f);
		};

	public:
		TextureScene();
		~TextureScene() = default;

		void Start() override;

		void Selected() override;

		void Unselected() override;

		void End() override;

		void FixedTick() override;

		void Tick(float dt) override;

	private:
		void UpdateCamera(float dt);

	private:
		Shader m_Shader;
		Shader m_SkyboxShader;

		ID3D11Buffer* m_pVertexBuffer = nullptr;
		ID3D11Buffer* m_pIndexBuffer = nullptr;
		ID3D11Buffer* m_pConstantBuffer = nullptr;

		ID3D11Texture2D* m_pTexture = nullptr;
		ID3D11ShaderResourceView* m_pTextureSRV = nullptr;
		ID3D11SamplerState* m_pSampler = nullptr;

		FrameData m_FrameData;

		CubeMapResource*	m_pCubeMap	= nullptr;
		ModelResource*		m_pModel	= nullptr;

		Pipeline m_Pipeline;

		Camera m_Camera;
	};
}