#include "ImGuiRenderer.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_dx11.h>

#include "Renderer/RenderAPI.h"

using namespace RS;

std::vector<std::function<void(void)>> ImGuiRenderer::s_DrawCalls;
std::mutex ImGuiRenderer::s_Mutex;

void ImGuiRenderer::Init(Display* pDisplay)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	//ImGui::SetWindowFontScale();
	ImGui::StyleColorsDark();

	ReScale(pDisplay->GetWidth(), pDisplay->GetHeight());

	GLFWwindow* pWindow = pDisplay->GetGLFWWindow();
	ImGui_ImplGlfw_InitForOpenGL(pWindow, true);

	ID3D11Device* pDevice = RenderAPI::Get()->GetDevice();
	ID3D11DeviceContext* pDeviceContext = RenderAPI::Get()->GetDeviceContext();
	ImGui_ImplDX11_Init(pDevice, pDeviceContext);
}

void ImGuiRenderer::Release()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiRenderer::Draw(std::function<void(void)> callback)
{
	std::lock_guard<std::mutex> lock(s_Mutex);
	s_DrawCalls.emplace_back(callback);
}

void ImGuiRenderer::Render()
{
	if (s_ShouldRescale)
	{
		// Resize only, after BeginFrame set s_ShouldRescale to false.
		s_ShouldRescale = false;
		InternalResize();
	}

	BeginFrame();

	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = s_Scale;

	{
		std::lock_guard<std::mutex> lock(s_Mutex);
		for (auto& drawCall : s_DrawCalls)
			drawCall();
	}

	EndFrame();

	// Clear the call stack
	s_DrawCalls.clear();
}

bool ImGuiRenderer::WantKeyInput()
{
	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureKeyboard || io.WantCaptureMouse;
}

void ImGuiRenderer::Resize()
{
	s_ShouldRescale = true;
}

float ImGuiRenderer::GetGuiScale()
{
	return s_Scale;
}

void ImGuiRenderer::BeginFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiRenderer::EndFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiRenderer::InternalResize()
{
	auto pDisplay	= Display::Get();
	uint32 width	= pDisplay->GetWidth();
	uint32 height	= pDisplay->GetHeight();
	if (width != 0 && height != 0)
	{
		Release();
		Init(pDisplay.get());
	}
}

void ImGuiRenderer::ReScale(uint32 width, uint32 height)
{
	ImGuiStyle& style = ImGui::GetStyle();
	float widthScale = width / 1920.f;
	float heightScale = height / 1080.f;
	s_Scale = widthScale > heightScale ? widthScale : heightScale;
	style.ScaleAllSizes(s_Scale);

	LOG_INFO("ImGui Scale: {}", s_Scale);
}
