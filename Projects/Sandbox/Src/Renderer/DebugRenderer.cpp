#include "PreCompiled.h"
#include "DebugRenderer.h"

#include "Loaders/ModelLoader.h"
#include "Core/Display.h"

#include "Renderer/ShaderHotReloader.h"

using namespace RS;

std::shared_ptr<DebugRenderer> DebugRenderer::Get()
{
	static std::shared_ptr<DebugRenderer> s_DebugRenderer = std::make_shared<DebugRenderer>();
	return s_DebugRenderer;
}

void DebugRenderer::Init()
{
	m_Pipeline.Init();

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	m_Pipeline.SetRasterState(rasterizerDesc);

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(glm::mat4);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_pVPBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create constant buffer for the VP!");

		result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_pViewBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create constant buffer for the View!");

		bufferDesc.ByteWidth = sizeof(GFrameData);
		result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_pGeomBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create constant buffer for the Proj!");
	}

	{
		Shader::Descriptor shaderDesc;
		shaderDesc.Fragment		= "DebugRenderer/Frag.hlsl";
		shaderDesc.Vertex		= "DebugRenderer/Vert.hlsl";
		AttributeLayout layout;
		layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "POSITION", 0);
		layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "COLOR", 0);
		m_LineShader.Load(shaderDesc, layout);
		ShaderHotReloader::AddShader(&m_LineShader);
	}

	{
		Shader::Descriptor shaderDesc;
		shaderDesc.Fragment		= "DebugRenderer/Frag.hlsl";
		shaderDesc.Vertex		= "DebugRenderer/GVert.hlsl";
		shaderDesc.Geometry		= "DebugRenderer/Geom.hlsl";
		AttributeLayout layout;
		layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "POSITION", 0);
		layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "COLOR", 0);
		m_PointShader.Load(shaderDesc, layout);
		ShaderHotReloader::AddShader(&m_PointShader);
	}
}

void DebugRenderer::Release()
{
	if (m_pLinesVertexBuffer)
	{
		m_pLinesVertexBuffer->Release();
		m_pLinesVertexBuffer = nullptr;
	}

	if (m_pPointsVertexBuffer)
	{
		m_pPointsVertexBuffer->Release();
		m_pPointsVertexBuffer = nullptr;
	}

	if (m_pVPBuffer)
	{
		m_pVPBuffer->Release();
		m_pVPBuffer = nullptr;
	}

	if (m_pViewBuffer)
	{
		m_pViewBuffer->Release();
		m_pViewBuffer = nullptr;
	}

	if (m_pGeomBuffer)
	{
		m_pGeomBuffer->Release();
		m_pGeomBuffer = nullptr;
	}

	m_LineShader.Release();
	m_PointShader.Release();
	m_Pipeline.Release();

	m_IsCameraSet = false;
}

void DebugRenderer::UpdateCamera(const glm::mat4& view, const glm::mat4& proj)
{
	m_IsCameraSet = true;

	glm::mat4 data = proj * view;
	auto context = RenderAPI::Get()->GetDeviceContext();
	D3D11_MAPPED_SUBRESOURCE resource;

	// Proj * View
	context->Map(m_pVPBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, &data, sizeof(glm::mat4));
	context->Unmap(m_pVPBuffer, 0);

	// View
	context->Map(m_pViewBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, &view, sizeof(glm::mat4));
	context->Unmap(m_pViewBuffer, 0);

	// Geometry Frame data
	float w = (float)Display::Get()->GetWidth();
	m_GeomFrameData.PointSize = 50.f/w; // 50 pixels in width.
	m_GeomFrameData.Proj = proj;
	context->Map(m_pGeomBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, &m_GeomFrameData, sizeof(GFrameData));
	context->Unmap(m_pGeomBuffer, 0);
}

uint32 DebugRenderer::PushLine(const glm::vec3& p1, const glm::vec3& p2, const Color& color, uint32 id, bool shouldClear)
{
	uint32 newID = ProcessID(id, Type::LINES);
	DataPoints& lines = m_IDLinesMap[newID];
	if(ShouldClearLines(newID, shouldClear))
		lines.m_Vertices.clear();

	Vertex v;
	v.Position = p1;
	v.Color = color;
	lines.m_Vertices.push_back(v);
	v.Position = p2;
	lines.m_Vertices.push_back(v);
	lines.ID = newID;

	m_ShouldBakeLines = true;

	return newID;
}

uint32 DebugRenderer::PushLines(const std::vector<glm::vec3>& points, const Color& color, uint32 id, bool shouldClear)
{
	uint32 newID = ProcessID(id, Type::LINES);
	DataPoints& lines = m_IDLinesMap[newID];
	if (ShouldClearLines(newID, shouldClear))
		lines.m_Vertices.clear();

	for (uint32 i = 1; i < points.size(); i++)
	{
		glm::vec3 point1 = points[i - 1];
		Vertex v;
		v.Position = point1;
		v.Color = color;
		lines.m_Vertices.push_back(v);

		glm::vec3 point2 = points[i];
		v.Position = point2;
		lines.m_Vertices.push_back(v);
	}

	m_ShouldBakeLines = true;

	return newID;
}

uint32 DebugRenderer::PushBox(const glm::vec3& min, const glm::vec3& max, const Color& color, uint32 id, bool shouldClear)
{
	uint32 newID = ProcessID(id, Type::LINES);

	std::vector<glm::vec3> points;
	points.push_back(glm::vec3(min.x, min.y, min.z));
	points.push_back(glm::vec3(min.x, min.y, max.z));
	points.push_back(glm::vec3(min.x, max.y, max.z));
	points.push_back(glm::vec3(min.x, max.y, min.z));
	points.push_back(glm::vec3(min.x, min.y, min.z));
	PushLines(points, color, newID, shouldClear);
	points.clear();
	points.push_back(glm::vec3(max.x, min.y, min.z));
	points.push_back(glm::vec3(max.x, min.y, max.z));
	points.push_back(glm::vec3(max.x, max.y, max.z));
	points.push_back(glm::vec3(max.x, max.y, min.z));
	points.push_back(glm::vec3(max.x, min.y, min.z));
	PushLines(points, color, newID, false);

	PushLine(glm::vec3(min.x, min.y, min.z), glm::vec3(max.x, min.y, min.z), color, newID, false);
	PushLine(glm::vec3(min.x, min.y, max.z), glm::vec3(max.x, min.y, max.z), color, newID, false);
	PushLine(glm::vec3(min.x, max.y, max.z), glm::vec3(max.x, max.y, max.z), color, newID, false);
	PushLine(glm::vec3(min.x, max.y, min.z), glm::vec3(max.x, max.y, min.z), color, newID, false);
	PushLine(glm::vec3(min.x, min.y, min.z), glm::vec3(max.x, min.y, min.z), color, newID, false);

	return newID;
}

uint32 DebugRenderer::PushMesh(ModelResource* pModel, const Color& color, glm::vec3 offset, uint32 id, bool shouldClear)
{
	uint32 newID = ProcessID(id, Type::LINES);

	glm::mat4 transform = glm::translate(offset);
	PushMeshInternal(pModel, color, newID, shouldClear, transform);

	return newID;
}

uint32 DebugRenderer::PushPoint(const glm::vec3& p, const Color& color, uint32 id, bool shouldClear)
{
	uint32 newID = ProcessID(id, Type::POINTS);
	DataPoints& points = m_IDPointsMap[newID];
	if (ShouldClearPoints(newID, shouldClear))
		points.m_Vertices.clear();

	Vertex v;
	v.Position = p;
	v.Color = color;
	points.m_Vertices.push_back(v);
	points.ID = newID;

	m_ShouldBakePoints = true;

	return newID;
}

uint32 DebugRenderer::PushPoints(const std::vector<glm::vec3>& points, const Color& color, uint32 id, bool shouldClear)
{
	uint32 newID = ProcessID(id, Type::POINTS);
	DataPoints& pointsData = m_IDPointsMap[newID];
	if (ShouldClearPoints(newID, shouldClear))
		pointsData.m_Vertices.clear();

	for (uint32 i = 0; i < points.size(); i++)
	{
		Vertex v;
		v.Position = points[i];
		v.Color = color;
		pointsData.m_Vertices.push_back(v);
	}

	m_ShouldBakePoints = true;

	return newID;
}

void DebugRenderer::Clear(uint32 id)
{
	// Clear all points and colors from the list of lines.
	{
		auto it = m_IDLinesMap.find(id);
		if (it != m_IDLinesMap.end())
			it->second.m_Vertices.clear();
	}

	// Clear all points and colors from the list of points.
	{
		auto it = m_IDPointsMap.find(id);
		if (it != m_IDPointsMap.end())
			it->second.m_Vertices.clear();
	}

	m_ShouldBakeLines = true;
	m_ShouldBakePoints = true;
}

void DebugRenderer::Clear()
{
	m_IDLinesMap.clear();
	m_IDPointsMap.clear();

	m_ShouldBakeLines = true;
	m_ShouldBakePoints = true;
}

void DebugRenderer::Render()
{
	if (m_ShouldBakeLines || m_ShouldBakePoints)
	{
		BakeLines();
		BakePoints();

		// Update stats
		m_Stats.NumberOfLineVertices	= m_PreviousLinesBufferSize;
		m_Stats.NumberOfPointVertices	= m_PreviousPointsBufferSize;
		m_Stats.NumberOfIDs				= s_IDGenerator;
	}
	
	if (m_IsCameraSet)
	{
		// Only bind the raster state, use the default depth state and view. Same for the RTV
		m_Pipeline.BindRasterState();

		auto renderAPI = RenderAPI::Get();
		ID3D11DeviceContext* pContext = renderAPI->GetDeviceContext();
		DrawLines(pContext);
		DrawPoints(pContext);
	}
	else
	{
		LOG_WARNING("Camera data was not updated for the Debug Renderer!");
	}
}

uint32 DebugRenderer::GenID()
{
	return ++s_IDGenerator;
}

const DebugRenderer::Stats& DebugRenderer::GetStats() const
{
	return m_Stats;
}

uint32 DebugRenderer::ProcessID(uint32 id, Type type)
{
	uint32 newID = id;
	if (newID == 0)
	{
		switch (type)
		{
		case RS::DebugRenderer::LINES:
			newID = s_DefaultLinesID;
			break;
		case RS::DebugRenderer::POINTS:
			newID = s_DefaultPointsID;
			break;
		default:
			LOG_WARNING("Debug rendering type is not supported!");
			break;
		}
	}
	return newID;
}

void DebugRenderer::DrawLines(ID3D11DeviceContext* pContext)
{
	if (m_pLinesVertexBuffer)
	{
		if (m_PreviousLinesBufferSize > 0)
		{
			m_LineShader.Bind();
			UINT stride = sizeof(Vertex);
			UINT offset = 0;
			pContext->IASetVertexBuffers(0, 1, &m_pLinesVertexBuffer, &stride, &offset);
			pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			pContext->VSSetConstantBuffers(0, 1, &m_pVPBuffer);
			pContext->Draw((UINT)m_PreviousLinesBufferSize, 0);
		}

		m_LinesFirstCall = true;
	}
}

void DebugRenderer::DrawPoints(ID3D11DeviceContext* pContext)
{
	if (m_pPointsVertexBuffer)
	{
		if (m_PreviousPointsBufferSize > 0)
		{
			m_PointShader.Bind();
			UINT stride = sizeof(Vertex);
			UINT offset = 0;
			pContext->IASetVertexBuffers(0, 1, &m_pPointsVertexBuffer, &stride, &offset);
			pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			pContext->VSSetConstantBuffers(0, 1, &m_pViewBuffer);
			pContext->GSSetConstantBuffers(0, 1, &m_pGeomBuffer);
			pContext->Draw((UINT)m_PreviousPointsBufferSize, 0);
		}

		m_PointsFirstCall = true;
	}
}

void DebugRenderer::BakeLines()
{
	if (m_ShouldBakeLines)
	{
		m_LinesToRender.m_Vertices.clear();

		for (auto it = m_IDLinesMap.begin(); it != m_IDLinesMap.end(); it++)
		{
			DataPoints& lines = it->second;
			uint32 nPoints = (uint32)lines.m_Vertices.size();
			for (uint32 i = 0; i < nPoints; i++)
				m_LinesToRender.m_Vertices.push_back(lines.m_Vertices[i]);
		}

		UpdateLinesDrawBuffer();

		m_ShouldBakeLines = false;
	}
}

void DebugRenderer::BakePoints()
{
	if (m_ShouldBakePoints)
	{
		m_PointsToRender.m_Vertices.clear();

		for (auto it = m_IDPointsMap.begin(); it != m_IDPointsMap.end(); it++)
		{
			DataPoints& points = it->second;
			uint32 nPoints = (uint32)points.m_Vertices.size();
			for (uint32 i = 0; i < nPoints; i++)
				m_PointsToRender.m_Vertices.push_back(points.m_Vertices[i]);
		}

		UpdatePointsDrawBuffer();

		m_ShouldBakePoints = false;
	}
}

void DebugRenderer::UpdateLinesDrawBuffer()
{
	if (m_pLinesVertexBuffer == nullptr)
	{
		if (m_LinesToRender.m_Vertices.empty() == false)
		{
			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.ByteWidth = (UINT)(sizeof(Vertex) * m_LinesToRender.m_Vertices.size());
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = m_LinesToRender.m_Vertices.data();
			data.SysMemPitch = 0;
			data.SysMemSlicePitch = 0;

			HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pLinesVertexBuffer);
			RS_D311_CHECK(result, "Failed to create lines vertex buffer!");
		}
	}
	else
	{
		// Recreate the buffer if the size if too small.
		if (m_PreviousLinesBufferSize < (uint32)m_LinesToRender.m_Vertices.size())
		{
			m_pLinesVertexBuffer->Release();

			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.ByteWidth = (UINT)(sizeof(Vertex) * m_LinesToRender.m_Vertices.size());
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = m_LinesToRender.m_Vertices.data();
			data.SysMemPitch = 0;
			data.SysMemSlicePitch = 0;

			HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pLinesVertexBuffer);
			RS_D311_CHECK(result, "Failed to recreate lines vertex buffer!");
		}

		auto context = RenderAPI::Get()->GetDeviceContext();
		D3D11_MAPPED_SUBRESOURCE resource;
		context->Map(m_pLinesVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		size_t size = sizeof(Vertex) * m_LinesToRender.m_Vertices.size();
		memcpy(resource.pData, m_LinesToRender.m_Vertices.data(), size);
		context->Unmap(m_pLinesVertexBuffer, 0);
	}

	m_PreviousLinesBufferSize = (uint32)m_LinesToRender.m_Vertices.size();
}

void RS::DebugRenderer::UpdatePointsDrawBuffer()
{
	if (m_pPointsVertexBuffer == nullptr)
	{
		if (m_PointsToRender.m_Vertices.empty() == false)
		{
			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.ByteWidth = (UINT)(sizeof(Vertex) * m_PointsToRender.m_Vertices.size());
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = m_PointsToRender.m_Vertices.data();
			data.SysMemPitch = 0;
			data.SysMemSlicePitch = 0;

			HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pPointsVertexBuffer);
			RS_D311_CHECK(result, "Failed to recreate points vertex buffer!");
		}
	}
	else
	{
		// Recreate the buffer if the size if too small.
		if (m_PreviousPointsBufferSize < (uint32)m_PointsToRender.m_Vertices.size())
		{
			m_pPointsVertexBuffer->Release();

			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.ByteWidth = (UINT)(sizeof(Vertex) * m_PointsToRender.m_Vertices.size());
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = m_PointsToRender.m_Vertices.data();
			data.SysMemPitch = 0;
			data.SysMemSlicePitch = 0;

			HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, &data, &m_pPointsVertexBuffer);
			RS_D311_CHECK(result, "Failed to create points vertex buffer!");
		}

		auto context = RenderAPI::Get()->GetDeviceContext();
		D3D11_MAPPED_SUBRESOURCE resource;
		context->Map(m_pPointsVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		size_t size = sizeof(Vertex) * m_PointsToRender.m_Vertices.size();
		memcpy(resource.pData, m_PointsToRender.m_Vertices.data(), size);
		context->Unmap(m_pPointsVertexBuffer, 0);
	}

	m_PreviousPointsBufferSize = (uint32)m_PointsToRender.m_Vertices.size();
}

bool DebugRenderer::ShouldClearPoints(uint32 id, bool shouldClear)
{
	bool res = shouldClear;
	if (id == s_DefaultPointsID)
	{
		if (m_PointsFirstCall)
		{
			res = true;
			m_PointsFirstCall = false;
		}
		else
		{
			res = false;
		}
	}
	
	return res;
}

bool RS::DebugRenderer::ShouldClearLines(uint32 id, bool shouldClear)
{
	bool res = shouldClear;
	if (id == s_DefaultLinesID)
	{
		if (m_LinesFirstCall)
		{
			res = true;
			m_LinesFirstCall = false;
		}
		else
		{
			res = false;
		}
	}

	return res;
}

void DebugRenderer::PushMeshInternal(ModelResource* model, const Color& color, uint32 id, bool shouldClear, const glm::mat4& accTransform)
{
	glm::mat4 transform(1.f);
	std::vector<glm::vec3> points;
	points.resize(4);
	transform = model->Transform * accTransform;
	for (MeshResource& mesh : model->Meshes)
	{
		for (uint32 i = 0; i < mesh.Indices.size(); i += 3)
		{
			MeshResource::Vertex v = mesh.Vertices[mesh.Indices[i]];
			glm::vec4 transformedPos = glm::vec4(v.Position, 1.f);
			transformedPos = transformedPos * transform;
			points[0] = (glm::vec3)transformedPos;

			v = mesh.Vertices[mesh.Indices[(size_t)i + 1]];
			transformedPos = glm::vec4(v.Position, 1.f);
			transformedPos = transformedPos * transform;
			points[1] = (glm::vec3)transformedPos;

			v = mesh.Vertices[mesh.Indices[(size_t)i + 2]];
			transformedPos = glm::vec4(v.Position, 1.f);
			transformedPos = transformedPos * transform;
			points[2] = (glm::vec3)transformedPos;

			points[3] = points[0];
			PushLines(points, color, id, i == 0 ? shouldClear : false);
		}
	}

	for (ModelResource& child : model->Children)
		PushMeshInternal(&child, color, id, shouldClear, transform);
}
