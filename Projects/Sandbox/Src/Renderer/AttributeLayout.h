#pragma once

#include "Renderer/RenderAPI.h"

namespace RS
{
	class AttributeLayout
	{
	public:
		class Attribute
		{
		public:
			Attribute(DXGI_FORMAT format, const std::string& semanticName, uint32 offset, uint32 instanceDataStepRate);
			~Attribute() = default;

			uint32 GetInstanceDataStepRate() const;
			uint32 GetCount() const;
			uint32 GetSize() const;
			uint32 GetOffset() const;
			DXGI_FORMAT GetFormat() const;
			const std::string& GetSemanticName() const;

		private:
			uint32 m_InstanceDataStepRate = 0;
			uint32 m_Offset = 0;
			uint32 m_Count	= 0;
			uint32 m_Size	= 0;
			DXGI_FORMAT m_Format;
			std::string m_SemanticName;
		};

		RS_DEFAULT_CLASS(AttributeLayout);
		void Push(DXGI_FORMAT format, const std::string& semanticName, uint32 instanceDivisor);

		uint32 GetStride() const;
		const std::vector<Attribute>& GetAttributes() const;

	private:
		uint32					m_Stride = 0;
		std::vector<Attribute>	m_Attributes;
	};
}