#include "PreCompiled.h"
#include "Shader.h"

using namespace RS;

void Shader::Release()
{
    if (m_pLayout)
    {
        m_pLayout->Release();
        m_pLayout = nullptr;
    }

    if (m_pVShader)
    {
        m_pVShader->Release();
        m_pVShader = nullptr;
    }

    if (m_pPShader)
    {
        m_pPShader->Release();
        m_pPShader = nullptr;
    }

    if (m_pCShader)
    {
        m_pCShader->Release();
        m_pCShader = nullptr;
    }

    if (m_pGShader)
    {
        m_pGShader->Release();
        m_pGShader = nullptr;
    }

    if (m_pHShader)
    {
        m_pHShader->Release();
        m_pHShader = nullptr;
    }

    if (m_pDShader)
    {
        m_pDShader->Release();
        m_pDShader = nullptr;
    }

    if (m_pPReflector)
    {
        m_pPReflector->Release();
        m_pPReflector = nullptr;
    }

    if (m_pVReflector)
    {
        m_pVReflector->Release();
        m_pVReflector = nullptr;
    }

    if (m_pCReflector)
    {
        m_pCReflector->Release();
        m_pCReflector = nullptr;
    }

    if (m_pGReflector)
    {
        m_pGReflector->Release();
        m_pGReflector = nullptr;
    }

    if (m_pHReflector)
    {
        m_pHReflector->Release();
        m_pHReflector = nullptr;
    }

    if (m_pDReflector)
    {
        m_pDReflector->Release();
        m_pDReflector = nullptr;
    }
}

bool Shader::Load(const std::string& fileName, ShaderTypeFlags types, const AttributeLayout& layout)
{
    Descriptor descriptor = {};
    std::string finalPath = std::string(RS_SHADER_PATH) + fileName;
    if (types & ShaderTypeFlag::VERTEX)
        descriptor.Vertex = finalPath + ".vs";
    if (types & ShaderTypeFlag::FRAGMENT)
        descriptor.Fragment = finalPath + ".ps";
    if (types & ShaderTypeFlag::GEOMETRY)
        descriptor.Geometry = finalPath + ".gs";
    if (types & ShaderTypeFlag::COMPUTE)
        descriptor.Compute = finalPath + ".cs";
    if (types & ShaderTypeFlag::TESS_HULL)
        descriptor.Hull = finalPath + ".hs";
    if (types & ShaderTypeFlag::TESS_DOMAIN)
        descriptor.Domain = finalPath + ".ds";
    m_ShaderTypes = types;
    return InitAndReload(descriptor, layout);
}

bool Shader::Load(const Descriptor& shaderDescriptor, const AttributeLayout& layout)
{
    Descriptor descriptor = {};
    std::string finalPath = std::string(RS_SHADER_PATH);
    if (!shaderDescriptor.Vertex.empty())
    {
        descriptor.Vertex = finalPath + shaderDescriptor.Vertex;
        m_ShaderTypes = ShaderTypeFlag::VERTEX;
    }
    if (!shaderDescriptor.Fragment.empty())
    {
        descriptor.Fragment = finalPath + shaderDescriptor.Fragment;
        m_ShaderTypes |= ShaderTypeFlag::FRAGMENT;
    }
    if (!shaderDescriptor.Geometry.empty())
    {
        descriptor.Geometry = finalPath + shaderDescriptor.Geometry;
        m_ShaderTypes |= ShaderTypeFlag::GEOMETRY;
    }
    if (!shaderDescriptor.Compute.empty())
    {
        descriptor.Compute = finalPath + shaderDescriptor.Compute;
        m_ShaderTypes |= ShaderTypeFlag::COMPUTE;
    }
    if (!shaderDescriptor.Hull.empty())
    {
        descriptor.Hull = finalPath + shaderDescriptor.Hull;
        m_ShaderTypes |= ShaderTypeFlag::TESS_HULL;
    }
    if (!shaderDescriptor.Domain.empty())
    {
        descriptor.Domain = finalPath + shaderDescriptor.Domain;
        m_ShaderTypes |= ShaderTypeFlag::TESS_DOMAIN;
    }
    return InitAndReload(descriptor, layout);
}

bool Shader::Reload()
{
    Descriptor descriptor = {};
    for (uint32 i = 0; i < m_Files.size(); i++)
    {
        std::string filePath    = m_Files[i];
        ShaderTypeFlag type     = m_FileTypes[i];
        if (type == ShaderTypeFlag::VERTEX)
            descriptor.Vertex = filePath;
        if (type == ShaderTypeFlag::FRAGMENT)
            descriptor.Fragment = filePath;
        if (type == ShaderTypeFlag::GEOMETRY)
            descriptor.Geometry = filePath;
        if (type == ShaderTypeFlag::COMPUTE)
            descriptor.Compute = filePath;
        if (type == ShaderTypeFlag::TESS_HULL)
            descriptor.Hull = filePath;
        if (type == ShaderTypeFlag::TESS_DOMAIN)
            descriptor.Domain = filePath;
    }
    
    LOG_INFO("Reload shaders");
    if (!descriptor.Vertex.empty())
        LOG_INFO("\tVertex Shader: {}", descriptor.Vertex);
    if (!descriptor.Fragment.empty())
        LOG_INFO("\tPixel Shader: {}", descriptor.Fragment);
    if (!descriptor.Geometry.empty())
        LOG_INFO("\tGeometry Shader: {}", descriptor.Geometry);
    if (!descriptor.Compute.empty())
        LOG_INFO("\tCompute Shader: {}", descriptor.Compute);
    if (!descriptor.Hull.empty())
        LOG_INFO("\tHull Shader: {}", descriptor.Hull);
    if (!descriptor.Domain.empty())
        LOG_INFO("\tDomain Shader: {}", descriptor.Domain);

    return InitAndReload(descriptor, m_Layout);
}

void Shader::Bind()
{
    ID3D11DeviceContext* pContext = RenderAPI::Get()->GetDeviceContext();
    pContext->IASetInputLayout(m_pLayout);

    if (m_ShaderTypes & ShaderTypeFlag::VERTEX)
        pContext->VSSetShader(m_pVShader, nullptr, 0);

    if (m_ShaderTypes & ShaderTypeFlag::FRAGMENT)
        pContext->PSSetShader(m_pPShader, nullptr, 0);

    if (m_ShaderTypes & ShaderTypeFlag::COMPUTE)
        pContext->CSSetShader(m_pCShader, nullptr, 0);

    if (m_ShaderTypes & ShaderTypeFlag::GEOMETRY)
        pContext->GSSetShader(m_pGShader, nullptr, 0);

    if (m_ShaderTypes & ShaderTypeFlag::TESS_HULL)
        pContext->HSSetShader(m_pHShader, nullptr, 0);

    if (m_ShaderTypes & ShaderTypeFlag::TESS_DOMAIN)
        pContext->DSSetShader(m_pDShader, nullptr, 0);
}

const std::vector<std::string>& Shader::GetFiles()
{
    return m_Files;
}

bool Shader::InitAndReload(const Descriptor& shaderDescriptor, const AttributeLayout& layout)
{
    bool result = true;

    Shader newShader;
    ID3DBlob* pBlob = nullptr;
    ID3DBlob* pVertexShaderBuffer = nullptr;
    if (!shaderDescriptor.Vertex.empty())
        result &= newShader.CreateAndCompileShaderPart(shaderDescriptor.Vertex, ShaderTypeFlag::VERTEX, (void**)&newShader.m_pVShader, newShader.m_pVReflector, pVertexShaderBuffer, false);
    else
    {
        result = false;
        LOG_ERROR("Failed to create shaders! Missing vertex shader!");
    }

    if (!shaderDescriptor.Fragment.empty())
        result &= newShader.CreateAndCompileShaderPart(shaderDescriptor.Fragment, ShaderTypeFlag::FRAGMENT, (void**)&newShader.m_pPShader, newShader.m_pPReflector, pBlob, true);
    if (!shaderDescriptor.Geometry.empty())
        result &= newShader.CreateAndCompileShaderPart(shaderDescriptor.Geometry, ShaderTypeFlag::GEOMETRY, (void**)&newShader.m_pGShader, newShader.m_pGReflector, pBlob, true);
    if (!shaderDescriptor.Compute.empty())
        result &= newShader.CreateAndCompileShaderPart(shaderDescriptor.Compute, ShaderTypeFlag::COMPUTE, (void**)&newShader.m_pCShader, newShader.m_pCReflector, pBlob, true);
    if (!shaderDescriptor.Hull.empty())
        result &= newShader.CreateAndCompileShaderPart(shaderDescriptor.Hull, ShaderTypeFlag::TESS_HULL, (void**)&newShader.m_pHShader, newShader.m_pHReflector, pBlob, true);
    if (!shaderDescriptor.Domain.empty())
        result &= newShader.CreateAndCompileShaderPart(shaderDescriptor.Domain, ShaderTypeFlag::TESS_DOMAIN, (void**)&newShader.m_pDShader, newShader.m_pDReflector, pBlob, true);

    result &= CreateLayout(pVertexShaderBuffer, layout, newShader.m_pLayout);

    if (!result)
    {
        newShader.Release();
        return false;
    }

    pVertexShaderBuffer->Release();
    pVertexShaderBuffer = nullptr;

    // Copy data over to this instance.
    m_Files     = newShader.m_Files;
    m_FileTypes = newShader.m_FileTypes;
    m_pLayout   = newShader.m_pLayout;

    m_pVShader = newShader.m_pVShader;
    m_pPShader = newShader.m_pPShader;
    m_pGShader = newShader.m_pGShader;
    m_pCShader = newShader.m_pCShader;
    m_pHShader = newShader.m_pHShader;
    m_pDShader = newShader.m_pDShader;

    m_pVReflector = newShader.m_pVReflector;
    m_pPReflector = newShader.m_pPReflector;
    m_pGReflector = newShader.m_pGReflector;
    m_pCReflector = newShader.m_pCReflector;
    m_pHReflector = newShader.m_pHReflector;
    m_pDReflector = newShader.m_pDReflector;

    m_Layout = layout;
}

bool Shader::CreateAndCompileShaderPart(const std::string& filePath, ShaderTypeFlag type, void** pShader, ID3D11ShaderReflection*& pReflection, ID3DBlob*& pByteCode, bool releaseBlob)
{
    HRESULT result;
    ID3DBlob* pErrorMessageBlob = nullptr;
    std::wstring p = Utils::Str2WStr(filePath);
    LPCWSTR pathW = p.c_str();

    std::string target = ShaderTypeToTarget(type);
    result = D3DCompileFromFile(pathW, NULL, NULL, "main", target.c_str(), D3DCOMPILE_ENABLE_STRICTNESS, 0, &pByteCode, &pErrorMessageBlob);
    if (FAILED(result))
    {
        if (pErrorMessageBlob)
        {
            // Get a pointer to the error message text buffer.
            char* compileErrors = (char*)(pErrorMessageBlob->GetBufferPointer());

            // Write out the error message.
            LOG_ERROR("Failed to compile shader!");
            LOG_ERROR(compileErrors);

            // Release the error message.
            pErrorMessageBlob->Release();
            pErrorMessageBlob = nullptr;

            return false;
        }
        else
        {
            LOG_ERROR("Failed to compile pixel shader: Missing file: {0}", filePath.c_str());
            return false;
        }
    }

    if (!CreateShader(type, pShader, pByteCode, pReflection))
        return false;

    if (releaseBlob)
    {
        pByteCode->Release();
        pByteCode = nullptr;
    }

    m_Files.push_back(filePath);
    m_FileTypes.push_back(type);

    return true;
}

bool Shader::CreateShader(ShaderTypeFlag type, void** pShader, ID3DBlob*& pByteCode, ID3D11ShaderReflection*& pReflection)
{
    HRESULT result;
    ID3D11Device* pDevice = RenderAPI::Get()->GetDevice();
    switch (type)
    {
    case ShaderTypeFlag::VERTEX:
        {
            ID3D11VertexShader** pVShader = (ID3D11VertexShader**)pShader;
            result = pDevice->CreateVertexShader(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), 0, pVShader);
            RS_D311_CHECK(result, "Failed to create Vertex Shader!");

            if (FAILED(result))
                return false;
        }
        break;
    case ShaderTypeFlag::FRAGMENT:
        {
            ID3D11PixelShader** pPShader = (ID3D11PixelShader**)pShader;
            result = pDevice->CreatePixelShader(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), 0, pPShader);
            RS_D311_CHECK(result, "Failed to create Pixel Shader!");

            if (FAILED(result))
                return false;
        }
        break;
    case ShaderTypeFlag::GEOMETRY:
        {
            ID3D11GeometryShader** pGShader = (ID3D11GeometryShader**)pShader;
            result = pDevice->CreateGeometryShader(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), 0, pGShader);
            RS_D311_CHECK(result, "Failed to create Geometry Shader!");

            if (FAILED(result))
                return false;
        }
        break;
    case ShaderTypeFlag::COMPUTE:
        {
            ID3D11ComputeShader** pCShader = (ID3D11ComputeShader**)pShader;
            result = pDevice->CreateComputeShader(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), 0, pCShader);
            RS_D311_CHECK(result, "Failed to create Compute Shader!");

            if (FAILED(result))
                return false;
        }
        break;
    case ShaderTypeFlag::TESS_HULL:
        {
            ID3D11HullShader** pHShader = (ID3D11HullShader**)pShader;
            result = pDevice->CreateHullShader(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), 0, pHShader);
            RS_D311_CHECK(result, "Failed to create Hull Shader!");

            if (FAILED(result))
                return false;
        }
        break;
    case ShaderTypeFlag::TESS_DOMAIN:
        {
            ID3D11DomainShader** pDShader = (ID3D11DomainShader**)pShader;
            result = pDevice->CreateDomainShader(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(), 0, pDShader);
            RS_D311_CHECK(result, "Failed to create Domain Shader!");

            if (FAILED(result))
                return false;
        }
        break;
    default:
        {
            LOG_ERROR("Failed to create shader! Shader type not supported!");
            return false;
        }
        break;
    }

    result = D3DReflect(pByteCode->GetBufferPointer(), pByteCode->GetBufferSize(),
        IID_ID3D11ShaderReflection, (void**)&pReflection);
    RS_D311_CHECK(result, "Failed to create {} shader reflection!", ShaderTypeToStringArr[type].c_str());

    if (FAILED(result))
        return false;

    return true;
}

bool Shader::CreateLayout(ID3DBlob*& pVertexShaderByteCode, const AttributeLayout& layout, ID3D11InputLayout*& pInputLayout)
{
    // Create the vertex input layout description.
    // This setup needs to match the Vertex stucture in used in the model and in the shader.
    size_t numElements = layout.GetAttributes().size();
    const std::vector<AttributeLayout::Attribute>& attributes = layout.GetAttributes();
    D3D11_INPUT_ELEMENT_DESC* polygonLayout = new D3D11_INPUT_ELEMENT_DESC[numElements];
    for (unsigned int i = 0; i < numElements; i++)
    {
        const AttributeLayout::Attribute& attribute = attributes[i];
        polygonLayout[i].SemanticName = attribute.GetSemanticName().c_str();
        polygonLayout[i].SemanticIndex = 0;
        polygonLayout[i].Format = attribute.GetFormat();
        polygonLayout[i].InputSlot = 0;
        polygonLayout[i].AlignedByteOffset = (i == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT);

        uint32 instanceDataStep = attribute.GetInstanceDataStepRate();
        polygonLayout[i].InputSlotClass = instanceDataStep != 0 ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
        polygonLayout[i].InstanceDataStepRate = instanceDataStep;
    }

    // Create the vertex input layout.
    ID3D11Device* device = RenderAPI::Get()->GetDevice();
    HRESULT result = device->CreateInputLayout(polygonLayout, (UINT)numElements, pVertexShaderByteCode->GetBufferPointer(),
        pVertexShaderByteCode->GetBufferSize(), &pInputLayout);
    RS_D311_CHECK(result, "Failed to create input layout!");

    delete[] polygonLayout;

    if (FAILED(result))
        return false;

    return true;
}
