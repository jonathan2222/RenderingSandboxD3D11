#pragma once

#include "Renderer/RenderAPI.h"

namespace RS
{
	class D3D11Helper
	{
	public:
		static D3D11_DEPTH_STENCIL_DESC GetDepthStencilDesc();

		static D3D11_TEXTURE2D_DESC GetTexture2DDesc(uint32_t width, uint32_t height, DXGI_FORMAT format);

		static D3D11_RASTERIZER_DESC GetRasterizerDesc();

		static std::vector<D3D11_SUBRESOURCE_DATA> FillTexture2DSubdata(D3D11_TEXTURE2D_DESC textureDesc, const void* pixels);

	private:

	};
}