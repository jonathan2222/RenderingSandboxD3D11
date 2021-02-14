#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>

#include "Renderer/D3D11Defines.h"

namespace RS
{
	struct DisplayDescription;
	class RenderAPI
	{
	public:
		struct VideoCardInfo
		{
			std::string Name;
			uint32 VideoMemory;
			uint32 SystemMemory;
			uint32 SharedSystemMemory;
		};

	public:
		RS_DEFAULT_ABSTRACT_CLASS(RenderAPI);

		static std::shared_ptr<RenderAPI> Get();

		void PreDisplayInit(DisplayDescription& displayDescriptor);
		void PostDisplayInit();
		void Release();

		VideoCardInfo& GetVideoCardInfo();

		ID3D11Device* GetDevice();
		ID3D11DeviceContext* GetDeviceContext();
		IDXGISwapChain1* GetSwapChain();

	private:
		void CreateDevice(IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType);
		void CreateSwapChain();
		void GetRefreshRate(IDXGIAdapter* adapter, DisplayDescription& displayDescriptor);
		std::vector<IDXGIAdapter*> EnumerateAdapters();

		void FillVideoCardInfo(IDXGIAdapter* adapter);

	private:
		uint32					m_RefreshRateNumerator		= 0;
		uint32					m_RefreshRateDenominator	= 1;
		VideoCardInfo			m_VideoCardinfo;

		IDXGIFactory2*			m_pFactory;
		ID3D11Device*			m_pDevice;
		ID3D11DeviceContext*	m_pDeviceContext;
		IDXGISwapChain1*		m_pSwapChain;

		#ifdef RS_CONFIG_DEVELOPMENT
		ID3D11Debug*			m_pDebug;
		#endif
	};
}