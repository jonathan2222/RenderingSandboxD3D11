#pragma once

#include "Renderer/RenderAPI.h"
#include "Renderer/ShaderDefines.h"
#include "Renderer/AttributeLayout.h"

#include "Utils/Utils.h"

namespace RS
{
	class Shader
	{
	public:
		struct Descriptor
		{
			std::string Vertex;
			std::string Geometry;
			std::string Fragment;
			std::string Compute;
			std::string Hull;
			std::string Domain;
		};

	public:
		RS_DEFAULT_CLASS(Shader);

		void Release();

		bool Load(const std::string& fileName, ShaderTypeFlags types, const AttributeLayout& layout);
		bool Load(const Descriptor& shaderDescriptor, const AttributeLayout& layout);
		bool Reload();

		void Bind();

		const std::vector<std::string>& GetFiles();

	private:
		bool InitAndReload(const Descriptor& shaderDescriptor, const AttributeLayout& layout);

		bool CreateAndCompileShaderPart(const std::string& filePath, ShaderTypeFlag type, void** pShader, ID3D11ShaderReflection*& pReflection, ID3DBlob*& pByteCode, bool releaseBlob);
		bool CreateShader(ShaderTypeFlag type, void** pShader, ID3DBlob*& pByteCode, ID3D11ShaderReflection*& pReflection);
		bool CreateLayout(ID3DBlob*& pVertexShaderByteCode, const AttributeLayout& layout, ID3D11InputLayout*& pInputLayout);

	private:
		std::vector<std::string>		m_Files;
		std::vector<ShaderTypeFlag>		m_FileTypes; // This is in the same order as m_Files.

		ShaderTypeFlags		m_ShaderTypes;

		// Shaders
		ID3D11VertexShader*		m_pVShader	= nullptr;
		ID3D11PixelShader*		m_pPShader	= nullptr;
		ID3D11GeometryShader*	m_pGShader	= nullptr;
		ID3D11ComputeShader*	m_pCShader	= nullptr;
		ID3D11HullShader*		m_pHShader	= nullptr;
		ID3D11DomainShader*		m_pDShader	= nullptr;

		AttributeLayout			m_Layout;
		ID3D11InputLayout*		m_pLayout	= nullptr;

		// Shader reflections
		ID3D11ShaderReflection* m_pPReflector = nullptr;
		ID3D11ShaderReflection* m_pVReflector = nullptr;
		ID3D11ShaderReflection* m_pCReflector = nullptr;
		ID3D11ShaderReflection* m_pGReflector = nullptr;
		ID3D11ShaderReflection* m_pHReflector = nullptr;
		ID3D11ShaderReflection* m_pDReflector = nullptr;
	};
}