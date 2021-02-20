#include "PreCompiled.h"
#include "DebugRenderer.h"

#include "Loaders/ModelLoader.h"

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

		glm::mat4 identity(1.f);
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = &identity;
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;

		HRESULT result = RenderAPI::Get()->GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_pConstantBuffer);
		RS_D311_ASSERT_CHECK(result, "Failed to create constant buffer!");
	}

	Shader::Descriptor shaderDesc;
	shaderDesc.Fragment = "DebugRenderer/Frag.hlsl";
	shaderDesc.Vertex = "DebugRenderer/Vert.hlsl";
	AttributeLayout layout;
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "POSITION", 0);
	layout.Push(DXGI_FORMAT_R32G32B32_FLOAT, "COLOR", 0);
	m_Shader.Load(shaderDesc, layout);
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

	if (m_pConstantBuffer)
	{
		m_pConstantBuffer->Release();
		m_pConstantBuffer = nullptr;
	}

	m_Shader.Release();
	m_Pipeline.Release();
}

void DebugRenderer::UpdateCamera(const glm::mat4& view, const glm::mat4& proj)
{
	glm::mat4 data = proj * view;
	auto context = RenderAPI::Get()->GetDeviceContext();
	D3D11_MAPPED_SUBRESOURCE resource;
	context->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, &data, sizeof(glm::mat4));
	context->Unmap(m_pConstantBuffer, 0);
}

uint32_t DebugRenderer::PushLine(const glm::vec3& p1, const glm::vec3& p2, const Color& color, uint32_t id, bool shouldClear)
{
	uint32_t newID = id == 0 ? GenID() : id;
	DataPoints& lines = m_IDLinesMap[newID];
	if (shouldClear)
		lines.m_Vertices.clear();

	Vertex v;
	v.Position = p1;
	v.Color = color;
	lines.m_Vertices.push_back(v);
	v.Position = p2;
	lines.m_Vertices.push_back(v);
	lines.ID = newID;

	m_ShouldBake = true;

	return newID;
}

uint32_t DebugRenderer::PushLines(const std::vector<glm::vec3>& points, const Color& color, uint32_t id, bool shouldClear)
{
	uint32_t newID = id == 0 ? GenID() : id;
	DataPoints& lines = m_IDLinesMap[newID];
	if (shouldClear)
		lines.m_Vertices.clear();

	for (uint32_t i = 1; i < points.size(); i++)
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

	m_ShouldBake = true;

	return newID;
}

uint32_t DebugRenderer::PushBox(const glm::vec3& min, const glm::vec3& max, const Color& color, uint32_t id, bool shouldClear)
{
	uint32_t newID = id == 0 ? GenID() : id;

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

uint32_t DebugRenderer::PushMesh(Model* model, const Color& color, glm::vec3 offset, uint32_t id, bool shouldClear)
{
	uint32_t newID = id == 0 ? GenID() : id;

	std::vector<glm::vec3> points;
	points.resize(4);
	for (uint32_t i = 0; i < model->Indices.size(); i += 3)
	{
		Model::Vertex v = model->Vertices[model->Indices[i]];
		points[0] = v.Position + offset;
		v = model->Vertices[model->Indices[(size_t)i + 1]];
		points[1] = v.Position + offset;
		v = model->Vertices[model->Indices[(size_t)i + 2]];
		points[2] = v.Position + offset;
		points[3] = points[0];
		PushLines(points, color, newID, i == 0 ? shouldClear : false);
	}

	return newID;
}

uint32_t DebugRenderer::PushPoint(const glm::vec3& p, const Color& color, uint32_t id, bool shouldClear)
{
	uint32_t newID = id == 0 ? GenID() : id;
	DataPoints& points = m_IDPointsMap[newID];
	if (shouldClear)
		points.m_Vertices.clear();

	Vertex v;
	v.Position = p;
	v.Color = color;
	points.m_Vertices.push_back(v);
	points.ID = newID;

	m_ShouldBake = true;

	return newID;
}

uint32_t DebugRenderer::PushPoints(const std::vector<glm::vec3>& points, const Color& color, uint32_t id, bool shouldClear)
{
	uint32_t newID = id == 0 ? GenID() : id;
	DataPoints& pointsData = m_IDPointsMap[newID];
	if (shouldClear)
		pointsData.m_Vertices.clear();

	for (uint32_t i = 0; i < points.size(); i++)
	{
		Vertex v;
		v.Position = points[i];
		v.Color = color;
		pointsData.m_Vertices.push_back(v);
	}

	m_ShouldBake = true;

	return newID;
}

void DebugRenderer::Clear(uint32_t id)
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

	m_ShouldBake = true;
}

void DebugRenderer::Render()
{
	if (m_ShouldBake)
	{
		BakeLines();
		BakePoints();
		m_ShouldBake = false;
	}
	
	// Only bind the raster state, use the default depth state and view. Same for the RTV
	m_Pipeline.BindRasterState();

	auto renderAPI = RenderAPI::Get();
	ID3D11DeviceContext* pContext = renderAPI->GetDeviceContext();
	m_Shader.Bind();
	pContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DrawLines(pContext);
	DrawPoints(pContext);
}

uint32_t DebugRenderer::GenID()
{
	static uint32_t s_ID = 0;
	return ++s_ID;
}

void DebugRenderer::DrawLines(ID3D11DeviceContext* pContext)
{
	if (m_pLinesVertexBuffer)
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		pContext->IASetVertexBuffers(0, 1, &m_pLinesVertexBuffer, &stride, &offset);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		pContext->Draw((UINT)m_PreviousLinesBufferSize, 0);
	}
}

void DebugRenderer::DrawPoints(ID3D11DeviceContext* pContext)
{
	if (m_pPointsVertexBuffer)
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		pContext->IASetVertexBuffers(0, 1, &m_pPointsVertexBuffer, &stride, &offset);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		pContext->Draw((UINT)m_PreviousPointsBufferSize, 0);
	}
}

void DebugRenderer::BakeLines()
{
	m_LinesToRender.m_Vertices.clear();

	for (auto it = m_IDLinesMap.begin(); it != m_IDLinesMap.end(); it++)
	{
		DataPoints& lines = it->second;
		uint32_t nPoints = (uint32_t)lines.m_Vertices.size();
		for (uint32_t i = 0; i < nPoints; i++)
			m_LinesToRender.m_Vertices.push_back(lines.m_Vertices[i]);
	}

	UpdateLinesDrawBuffer();
}

void DebugRenderer::BakePoints()
{
	m_PointsToRender.m_Vertices.clear();

	for (auto it = m_IDPointsMap.begin(); it != m_IDPointsMap.end(); it++)
	{
		DataPoints& points = it->second;
		uint32_t nPoints = (uint32_t)points.m_Vertices.size();
		for (uint32_t i = 0; i < nPoints; i++)
			m_PointsToRender.m_Vertices.push_back(points.m_Vertices[i]);
	}

	UpdatePointsDrawBuffer();
}

void DebugRenderer::UpdateLinesDrawBuffer()
{
	if(m_PreviousLinesBufferSize != m_LinesToRender.m_Vertices.size())
	{
		if (m_pLinesVertexBuffer == nullptr)
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
		else
		{
			auto context = RenderAPI::Get()->GetDeviceContext();
			D3D11_MAPPED_SUBRESOURCE resource;
			context->Map(m_pLinesVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
			size_t size = sizeof(Vertex) * m_LinesToRender.m_Vertices.size();
			memcpy(resource.pData, m_LinesToRender.m_Vertices.data(), size);
			context->Unmap(m_pLinesVertexBuffer, 0);
		}

		m_PreviousLinesBufferSize = m_LinesToRender.m_Vertices.size();
	}
}

void RS::DebugRenderer::UpdatePointsDrawBuffer()
{
	if (m_PreviousPointsBufferSize != m_PointsToRender.m_Vertices.size())
	{
		if (m_pPointsVertexBuffer == nullptr)
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
			RS_D311_CHECK(result, "Failed to create points vertex buffer!");
		}
		else
		{
			auto context = RenderAPI::Get()->GetDeviceContext();
			D3D11_MAPPED_SUBRESOURCE resource;
			context->Map(m_pPointsVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
			size_t size = sizeof(Vertex) * m_PointsToRender.m_Vertices.size();
			memcpy(resource.pData, m_PointsToRender.m_Vertices.data(), size);
			context->Unmap(m_pPointsVertexBuffer, 0);
		}

		m_PreviousPointsBufferSize = m_PointsToRender.m_Vertices.size();
	}
}
