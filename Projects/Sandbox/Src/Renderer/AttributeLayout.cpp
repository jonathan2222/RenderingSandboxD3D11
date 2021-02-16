#include "PreCompiled.h"
#include "AttributeLayout.h"

#include "Renderer/RenderUtils.h"

using namespace RS;

AttributeLayout::Attribute::Attribute(DXGI_FORMAT format, const std::string& semanticName, uint32 offset, uint32 instanceDataStepRate) :
    m_Offset(offset), m_SemanticName(semanticName), m_Size(0), m_Format(format), m_InstanceDataStepRate(instanceDataStepRate)
{
    m_Count = RenderUtils::GetNumChannelsFromFormat(format);
    m_Size = RenderUtils::GetSizeOfFormat(format);
}

uint32 AttributeLayout::Attribute::GetInstanceDataStepRate() const
{
    return m_InstanceDataStepRate;
}

uint32 AttributeLayout::Attribute::GetCount() const
{
    return m_Count;
}

uint32 AttributeLayout::Attribute::GetSize() const
{
    return m_Size;
}

uint32 AttributeLayout::Attribute::GetOffset() const
{
    return m_Offset;
}

DXGI_FORMAT AttributeLayout::Attribute::GetFormat() const
{
    return m_Format;
}

const std::string& AttributeLayout::Attribute::GetSemanticName() const
{
    return m_SemanticName;
}

void AttributeLayout::Push(DXGI_FORMAT format, const std::string& semanticName, uint32 instanceDivisor)
{
    Attribute attribute(format, semanticName, m_Stride, instanceDivisor);
    m_Stride += attribute.GetSize();
    m_Attributes.push_back(attribute);
}

uint32 AttributeLayout::GetStride() const
{
    return m_Stride;
}

const std::vector<AttributeLayout::Attribute>& AttributeLayout::GetAttributes() const
{
    return m_Attributes;
}
