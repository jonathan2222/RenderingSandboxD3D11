#include "PreCompiled.h"
#include "RenderAPI.h"

#include "Core/Display.h"

using namespace RS;

std::shared_ptr<RenderAPI> RenderAPI::Get()
{
    static std::shared_ptr<RenderAPI> s_Display = std::make_shared<RenderAPI>();
    return s_Display;
}

void RenderAPI::Init(DisplayDescription& displayDescriptor)
{
	HRESULT result;

	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory2), (void**)&m_pFactory);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to create the DirectX factory object!");

	// Go through graphics adapters which are compatible with DirectX.
	std::vector <IDXGIAdapter*> adapters = EnumerateAdapters();
	IDXGIAdapter* pAdapter = ChooseAdapter(adapters);
	FillVideoCardInfo(pAdapter);

	CreateDevice(pAdapter, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN);

	// Fetch refresh rate from the default adapter.
	GetRefreshRate(adapters[0], displayDescriptor);

	// Release the adapters.
	for (auto& ad : adapters)
		ad->Release();
	pAdapter = nullptr;

	CreateSwapChain();
	// Release the factory.
	m_pFactory->Release();
	m_pFactory = nullptr;
}

void RenderAPI::Release()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_pSwapChain)
		m_pSwapChain->SetFullscreenState(false, NULL);

	if (m_pDeviceContext)
	{
		m_pDeviceContext->Release();
		m_pDeviceContext = nullptr;
	}

	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = nullptr;
	}

	if (m_pDevice)
	{
#ifdef RS_CONFIG_DEVELOPMENT
		m_pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&m_pDebug));
		m_pDebug->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_SUMMARY);
		m_pDebug->Release();
#endif
		m_pDevice->Release();
		m_pDevice = nullptr;
	}
}

RenderAPI::VideoCardInfo& RenderAPI::GetVideoCardInfo()
{
    return m_VideoCardinfo;
}

ID3D11Device* RenderAPI::GetDevice()
{
    return m_pDevice;
}

ID3D11DeviceContext* RenderAPI::GetDeviceContext()
{
    return m_pDeviceContext;
}

IDXGISwapChain1* RenderAPI::GetSwapChain()
{
    return m_pSwapChain;
}

void RenderAPI::CreateDevice(IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType)
{
	UINT flags = 0;
#ifdef RS_CONFIG_DEVELOPMENT
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// Set the feature level to DirectX 11.1.
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

	// Create a D3D11Device and a device context.
	HRESULT result = D3D11CreateDevice(adapter, driverType, NULL, flags,
		&featureLevel, 1, D3D11_SDK_VERSION, &m_pDevice, NULL, &m_pDeviceContext);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to create the device and device context!");
}

void RenderAPI::CreateSwapChain()
{
	DisplayDescription displayDiscription = Display::Get()->GetDescription();

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.Width = (UINT)displayDiscription.Width;
	swapChainDesc.Height = (UINT)displayDiscription.Height;
	// Double back buffer.
	swapChainDesc.BufferCount = 2;
	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED;
	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc;
	ZeroMemory(&fullscreenDesc, sizeof(fullscreenDesc));
	fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	fullscreenDesc.Windowed = displayDiscription.Fullscreen ? FALSE : TRUE;
	if (displayDiscription.VSync)
	{
		fullscreenDesc.RefreshRate.Numerator = m_RefreshRateNumerator;
		fullscreenDesc.RefreshRate.Denominator = m_RefreshRateDenominator;
	}
	else
	{
		fullscreenDesc.RefreshRate.Numerator = 0;
		fullscreenDesc.RefreshRate.Denominator = 1;
	}
	
	HWND wnd = (HWND)Display::Get()->GetHWND();
	HRESULT result = m_pFactory->CreateSwapChainForHwnd(m_pDevice, wnd, &swapChainDesc, &fullscreenDesc, NULL, &m_pSwapChain);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to create swap chain!");
}

void RenderAPI::GetRefreshRate(IDXGIAdapter* adapter, DisplayDescription& displayDescriptor)
{
	HRESULT result;

	// Enumerate the primary adapter output (monitor).
	IDXGIOutput* adapterOutput;
	result = adapter->EnumOutputs(0, &adapterOutput);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to create an DirectX adapter for the monitor!");

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	unsigned int numModes = 0;
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to fetch the monitor's display mode list length!");

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	RS_ASSERT(displayModeList, "Could not initiate DirectX11: Failed to allocate memory for the monitor's display mode list!");

	// Fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to fetch the display mode list!");

	bool gotRefreshRate = false;
	for (unsigned int i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)displayDescriptor.Width)
		{
			if (displayModeList[i].Height == (unsigned int)displayDescriptor.Height)
			{
				m_RefreshRateNumerator = displayModeList[i].RefreshRate.Numerator;
				m_RefreshRateDenominator = displayModeList[i].RefreshRate.Denominator;
				gotRefreshRate = true;
				break;
			}
		}
	}
	RS_ASSERT(gotRefreshRate, "Could not initiate DirectX11: Could not find a matching display mode for a with and height of ({0}, {1})!", displayDescriptor.Width, displayDescriptor.Height);

	// Release the display mode list.
	delete[] displayModeList;
	displayModeList = 0;

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = 0;
}

std::vector<IDXGIAdapter*> RenderAPI::EnumerateAdapters()
{
	IDXGIAdapter* pAdapter;
	std::vector <IDXGIAdapter*> vAdapters;

	for (unsigned int i = 0; m_pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
		vAdapters.push_back(pAdapter);

	return vAdapters;
}

IDXGIAdapter* RenderAPI::ChooseAdapter(std::vector<IDXGIAdapter*>& adapters)
{
	IDXGIAdapter* pAdapter = nullptr;

	DXGI_ADAPTER_DESC descI;
	DXGI_ADAPTER_DESC descCurrent;
	for (IDXGIAdapter*& ad : adapters)
	{
		ZeroMemory(&descI, sizeof(descI));
		ad->GetDesc(&descI);

		if (pAdapter == nullptr) pAdapter = ad;
		else
		{
			ZeroMemory(&descCurrent, sizeof(descCurrent));
			pAdapter->GetDesc(&descCurrent);
			// TODO: Change this, this is a bad way of fetching the gpu.
			if (descI.DedicatedVideoMemory > descCurrent.DedicatedVideoMemory)
			{
				pAdapter = ad;
			}
		}
	}

	return pAdapter;
}

void RenderAPI::FillVideoCardInfo(IDXGIAdapter* adapter)
{
	HRESULT result;
	// Get the adapter (video card) description.
	DXGI_ADAPTER_DESC adapterDesc;
	result = adapter->GetDesc(&adapterDesc);
	RS_D311_ASSERT_CHECK(result, "Could not initiate DirectX11: Failed to fetch adapter description from the video card!");

	VideoCardInfo& videoCardInfo = GetVideoCardInfo();
	// Store the dedicated video card memory in megabytes.
	videoCardInfo.VideoMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	char name[128];
	unsigned long long len;
	int error = wcstombs_s(&len, name, 128, adapterDesc.Description, 128);
	if (error != 0)
		LOG_WARNING("Failed to convert video card name from w-string to c-string!");
	videoCardInfo.Name = std::string(name);

#ifdef RS_CONFIG_DEVELOPMENT
	LOG_INFO("----- Adapter -----");
	LOG_INFO("Description: {0}", name);
	LOG_INFO("VendorId: {0}", adapterDesc.VendorId);
	LOG_INFO("DeviceId: {0}", adapterDesc.DeviceId);
	LOG_INFO("SubSysId: {0}", adapterDesc.SubSysId);
	LOG_INFO("Revision: {0}", adapterDesc.Revision);
	LOG_INFO("DedicatedVideoMemory: {0}", adapterDesc.DedicatedVideoMemory);
	LOG_INFO("DedicatedSystemMemory: {0}", adapterDesc.DedicatedSystemMemory);
	LOG_INFO("SharedSystemMemory: {0}", adapterDesc.SharedSystemMemory);
#endif
}
