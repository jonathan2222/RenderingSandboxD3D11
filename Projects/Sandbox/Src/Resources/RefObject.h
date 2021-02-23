#pragma once

namespace RS
{
	class RefObject
	{
	public:
		void AddRef();
		void RemoveRef();
		uint32 GetRefCount() const;

	private:
		uint32 m_RefCount = 0;
	};
}