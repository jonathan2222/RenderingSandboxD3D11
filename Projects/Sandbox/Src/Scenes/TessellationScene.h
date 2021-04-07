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
			glm::mat4 world = glm::mat4(1.f);
			glm::mat4 view	= glm::mat4(1.f);
			glm::mat4 proj	= glm::mat4(1.f);
			glm::vec4 info	= glm::vec4(0.f); // First element if the alpha value for Phong Tessellation.
		};

		struct PSData
		{
			glm::vec4 cameraPos = glm::vec4(0.f);
			glm::vec4 cameraDir = glm::vec4(0.f);
			glm::vec4 lightPos	= glm::vec4(0.f);
		};

		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec3 Tangent;
			glm::vec2 UV;
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
		void ToggleWireframe(bool forceToggle);
		void CreateTexture(const std::string& fileName, ID3D11Texture2D*& pTexture, ID3D11ShaderResourceView*& pTextureView);
		void CalcTangents(std::vector<Vertex>& vertices, const std::vector<uint32>& indices);

	private:
		Shader						m_TriShader;
		Shader						m_QuadShader;

		ID3D11Buffer*				m_pVertexBuffer				= nullptr;
		ID3D11Buffer*				m_pTriIndexBuffer			= nullptr;
		ID3D11Buffer*				m_pQuadIndexBuffer			= nullptr;

		ID3D11Buffer*				m_pVSConstantBuffer			= nullptr;
		ID3D11Buffer*				m_pHSConstantBuffer			= nullptr;
		ID3D11Buffer*				m_pDSConstantBuffer			= nullptr;
		ID3D11Buffer*				m_pPSConstantBuffer			= nullptr;

		uint32						m_NumTriIndices				= 0;
		uint32						m_NumQuadIndices			= 0;

		PSData						m_PSData;
		CameraData					m_CameraData;

		Camera						m_Camera;

		bool						m_IsWireframeEnabled		= true;
		Pipeline					m_Pipeline;

		ID3D11Texture2D*			m_pAlbedoTexture			= nullptr;
		ID3D11Texture2D*			m_pNormalTexture			= nullptr;
		ID3D11Texture2D*			m_pDisplacementTexture		= nullptr;
		ID3D11Texture2D*			m_pAOTexture				= nullptr;

		ID3D11ShaderResourceView*	m_pAlbedoTextureView		= nullptr;
		ID3D11ShaderResourceView*	m_pNormalTextureView		= nullptr;
		ID3D11ShaderResourceView*	m_pDisplacementTextureView	= nullptr;
		ID3D11ShaderResourceView*	m_pAOTextureView			= nullptr;

		ID3D11SamplerState*			m_pSampler					= nullptr;
	};
}