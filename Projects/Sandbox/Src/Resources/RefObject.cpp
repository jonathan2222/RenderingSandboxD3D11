#include "PreCompiled.h"
#include "RefObject.h"

using namespace RS;

void RefObject::AddRef()
{
	m_RefCount++;
}

void RefObject::RemoveRef()
{
	m_RefCount--;
}

uint32 RefObject::GetRefCount() const
{
	return m_RefCount;
}
