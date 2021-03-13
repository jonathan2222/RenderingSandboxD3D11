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
		void UpdateCamera(float dt);

	private:
		Shader m_Shader;

		ID3D11Buffer* m_pConstantBufferFrame = nullptr;

		FrameData				m_FrameData;

		ModelResource* m_pModel = nullptr;

		Camera			m_Camera;

		Pipeline		m_Pipeline;
	};
}