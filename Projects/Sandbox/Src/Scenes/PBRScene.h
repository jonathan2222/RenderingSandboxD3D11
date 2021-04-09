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
	class PBRScene : public Scene
	{
	public:
		struct FrameData
		{
			glm::mat4 view = glm::mat4(1.f);
			glm::mat4 proj = glm::mat4(1.f);
		};

		struct CameraData
		{
			glm::vec4 camPos	= glm::vec4(0.f);
			glm::vec4 lightPos	= glm::vec4(0.f);
		};

		struct SkyboxFrameData
		{
			glm::mat4 world = glm::mat4(1.f);
			glm::mat4 view = glm::mat4(1.f);
			glm::mat4 proj = glm::mat4(1.f);
		};

	public:
		PBRScene();
		~PBRScene() = default;

		void Start() override;

		void Selected() override;

		void Unselected() override;

		void End() override;

		void FixedTick() override;

		void Tick(float dt) override;

	private:
		void DrawImGui();

	private:
		Shader				m_Shader;
		Shader				m_SkyboxShader;

		ID3D11Buffer*		m_pConstantBufferFrame = nullptr;
		ID3D11Buffer*		m_pConstantBufferCamera = nullptr;
		ID3D11Buffer*		m_pConstantBufferSkybox = nullptr;

		FrameData			m_FrameData;
		CameraData			m_CameraData;
		SkyboxFrameData		m_SkyboxFrameData;

		ModelResource*		m_pModel		= nullptr;
		int32				m_RenderMode	= 0;

		Camera				m_Camera;

		Pipeline			m_Pipeline;

		// IBL textures
		CubeMapResource*	m_pCubemap		= nullptr;
		ModelResource*		m_pInvCubeModel = nullptr;
	};
}