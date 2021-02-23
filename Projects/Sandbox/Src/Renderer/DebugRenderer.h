#pragma once

#include "Utils/Maths.h"
#include "Renderer/Color.h"

#include "Renderer/Pipeline.h"
#include "Renderer/Shader.h"

namespace RS
{
	struct Model;
	class DebugRenderer
	{
	public:
		struct Stats
		{
			uint32 NumberOfLineVertices		= 0;
			uint32 NumberOfPointVertices	= 0;
			uint32 NumberOfIDs				= 0;
		};

	public:
		static std::shared_ptr<DebugRenderer> Get();

		void Init();
		void Release();

		void UpdateCamera(const glm::mat4& view, const glm::mat4& proj);

		uint32 PushLine(const glm::vec3& p1, const glm::vec3& p2, const Color& color = Color::RED, uint32 id = 0, bool shouldClear = true);
		uint32 PushLines(const std::vector<glm::vec3>& points, const Color& color = Color::RED, uint32 id = 0, bool shouldClear = true);
		uint32 PushBox(const glm::vec3& min, const glm::vec3& max, const Color& color = Color::RED, uint32 id = 0, bool shouldClear = true);
		uint32 PushMesh(Model* model, const Color& color = Color::RED, glm::vec3 offset = glm::vec3(0.f), uint32 id = 0, bool shouldClear = true);
		uint32 PushPoint(const glm::vec3& p, const Color& color = Color::RED, uint32 id = 0, bool shouldClear = true);
		uint32 PushPoints(const std::vector<glm::vec3>& points, const Color& color = Color::RED, uint32 id = 0, bool shouldClear = true);

		/*
		* Clear the data for a specific id.
		*/
		void Clear(uint32 id);

		/*
		* Clear all data.
		*/
		void Clear();

		void Render();

		uint32 GenID();

		const Stats& GetStats() const;

	private:
		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Color;
		};

		struct DataPoints
		{
			std::vector<Vertex> m_Vertices;
			uint32 ID = 0u;
		};

		struct GFrameData
		{
			glm::mat4	Proj		= glm::mat4(1.f);
			float		PointSize	= 0.1f;
			glm::vec3	_Padding	= glm::vec3(0.f);
		};

		enum Type
		{
			LINES,
			POINTS
		};

		uint32 ProcessID(uint32 id, Type type);

		void DrawLines(ID3D11DeviceContext* pContext);
		void DrawPoints(ID3D11DeviceContext* pContext);

		void BakeLines();
		void BakePoints();

		void UpdateLinesDrawBuffer();
		void UpdatePointsDrawBuffer();

		bool ShouldClearPoints(uint32 id, bool shouldClear);
		bool ShouldClearLines(uint32 id, bool shouldClear);

	private:
		// Holds data of the different types.
		std::unordered_map<uint32, DataPoints>	m_IDLinesMap;
		std::unordered_map<uint32, DataPoints>	m_IDPointsMap;
		DataPoints				m_LinesToRender;
		DataPoints				m_PointsToRender;
		bool					m_ShouldBakeLines				= false;
		bool					m_ShouldBakePoints				= false;

		// Rendering objects.
		Pipeline				m_Pipeline;
		ID3D11Buffer*			m_pLinesVertexBuffer		= nullptr;
		uint32					m_PreviousLinesBufferSize	= 0u;
		ID3D11Buffer*			m_pPointsVertexBuffer		= nullptr;
		uint32					m_PreviousPointsBufferSize	= 0u;
		// Holds data of view and projection matrices.
		ID3D11Buffer*			m_pVPBuffer					= nullptr;
		ID3D11Buffer*			m_pViewBuffer				= nullptr;
		ID3D11Buffer*			m_pGeomBuffer				= nullptr;
		Shader					m_LineShader;
		Shader					m_PointShader;

		GFrameData				m_GeomFrameData;

		// Statistics for debugging.
		Stats					m_Stats;

		// Safe guard.
		bool					m_IsCameraSet				= false;

		// Variables to prevent the default push to clear each time its called.
		bool					m_LinesFirstCall			= true;
		bool					m_PointsFirstCall			= true;

		// ID related variables.
		static const uint32		s_DefaultLinesID			= 1u;
		static const uint32		s_DefaultPointsID			= s_DefaultLinesID+1;
		inline static uint32	s_IDGenerator				= s_DefaultPointsID;
	};
}