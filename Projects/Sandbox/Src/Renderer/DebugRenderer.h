#pragma once

#include "Utils/Maths.h"
#include "Renderer/Color.h"

#include "Renderer/Pipeline.h"
#include "Renderer/Shader.h"

namespace RS
{
	class Model;
	class DebugRenderer
	{
	public:
		static std::shared_ptr<DebugRenderer> Get();

		void Init();
		void Release();

		void UpdateCamera(const glm::mat4& view, const glm::mat4& proj);

		uint32_t PushLine(const glm::vec3& p1, const glm::vec3& p2, const Color& color = Color::RED, uint32_t id = 0, bool shouldClear = true);
		uint32_t PushLines(const std::vector<glm::vec3>& points, const Color& color = Color::RED, uint32_t id = 0, bool shouldClear = true);
		uint32_t PushBox(const glm::vec3& min, const glm::vec3& max, const Color& color = Color::RED, uint32_t id = 0, bool shouldClear = true);
		uint32_t PushMesh(Model* model, const Color& color = Color::RED, glm::vec3 offset = glm::vec3(0.f), uint32_t id = 0, bool shouldClear = true);
		uint32_t PushPoint(const glm::vec3& p, const Color& color = Color::RED, uint32_t id = 0, bool shouldClear = true);
		uint32_t PushPoints(const std::vector<glm::vec3>& points, const Color& color = Color::RED, uint32_t id = 0, bool shouldClear = true);

		void Clear(uint32_t id);

		void Render();

		uint32_t GenID();

	private:
		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Color;
		};

		struct DataPoints
		{
			std::vector<Vertex> m_Vertices;
			uint32_t ID;
		};

		void DrawLines(ID3D11DeviceContext* pContext);
		void DrawPoints(ID3D11DeviceContext* pContext);

		void BakeLines();
		void BakePoints();

		void UpdateLinesDrawBuffer();
		void UpdatePointsDrawBuffer();

	private:
		std::unordered_map<uint32_t, DataPoints>	m_IDLinesMap;
		std::unordered_map<uint32_t, DataPoints>	m_IDPointsMap;
		DataPoints	m_LinesToRender;
		DataPoints	m_PointsToRender;
		bool		m_ShouldBake = false;
		Pipeline	m_Pipeline;

		ID3D11Buffer*	m_pLinesVertexBuffer		= nullptr;
		uint32_t		m_PreviousLinesBufferSize	= 0;

		ID3D11Buffer*	m_pPointsVertexBuffer		= nullptr;
		uint32_t		m_PreviousPointsBufferSize	= 0;

		// Holds data of view and projection matrices.
		ID3D11Buffer*	m_pConstantBuffer			= nullptr;

		Shader			m_Shader;
	};
}