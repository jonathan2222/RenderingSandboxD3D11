#include "PreCompiled.h"
#include "D3D11Helper.h"

#include "Renderer/RenderUtils.h"

using namespace RS;

D3D11_DEPTH_STENCIL_DESC D3D11Helper::GetDepthStencilDesc()
{
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable					= true;
	depthStencilDesc.DepthWriteMask					= D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc						= D3D11_COMPARISON_LESS_EQUAL;

	depthStencilDesc.StencilEnable					= true;
	depthStencilDesc.StencilReadMask				= 0xFF;
	depthStencilDesc.StencilWriteMask				= 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

	return depthStencilDesc;
}

D3D11_TEXTURE2D_DESC D3D11Helper::GetTexture2DDesc(uint32_t width, uint32_t height, DXGI_FORMAT format)
{
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width				= width;
	depthBufferDesc.Height				= height;
	depthBufferDesc.MipLevels			= 1;
	depthBufferDesc.ArraySize			= 1;
	depthBufferDesc.Format				= format;
	depthBufferDesc.SampleDesc.Count	= 1;
	depthBufferDesc.SampleDesc.Quality	= 0;
	depthBufferDesc.Usage				= D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags			= 0;
	depthBufferDesc.CPUAccessFlags		= 0;
	depthBufferDesc.MiscFlags			= 0;

	return depthBufferDesc;
}

D3D11_RASTERIZER_DESC D3D11Helper::GetRasterizerDesc()
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode					= D3D11_FILL_SOLID;
	rasterizerDesc.CullMode					= D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise	= false;
	rasterizerDesc.DepthBias				= 0;
	rasterizerDesc.DepthBiasClamp			= 0.0f;
	rasterizerDesc.SlopeScaledDepthBias		= 0.0f;
	rasterizerDesc.DepthClipEnable			= true;
	rasterizerDesc.ScissorEnable			= false;
	rasterizerDesc.MultisampleEnable		= false;
	rasterizerDesc.AntialiasedLineEnable	= false;
	return rasterizerDesc;
}

std::vector<D3D11_SUBRESOURCE_DATA> D3D11Helper::FillTexture2DSubdata(D3D11_TEXTURE2D_DESC textureDesc, const void* pixels)
{
	uint32 pixelSize = RenderUtils::GetSizeOfFormat(textureDesc.Format);
	std::vector<D3D11_SUBRESOURCE_DATA> subData;
	if (textureDesc.MipLevels > 1)
	{
		uint32 width = textureDesc.Width;
		for (uint32 mip = 0; mip < textureDesc.MipLevels; mip++)
		{
			D3D11_SUBRESOURCE_DATA data = {};
			data.pSysMem = pixels;
			data.SysMemPitch = width * pixelSize;
			data.SysMemSlicePitch = 0;
			subData.push_back(data);

			width /= 2;
		}
	}
	else
	{
		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = pixels;
		data.SysMemPitch = textureDesc.Width * pixelSize;
		data.SysMemSlicePitch = textureDesc.Width * textureDesc.Height * pixelSize; // This is not used, only used for 3D textures!
		subData.push_back(data);
	}

	return subData;
}
