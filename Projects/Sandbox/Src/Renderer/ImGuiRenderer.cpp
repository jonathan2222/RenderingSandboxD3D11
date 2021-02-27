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

	/*ImGuiStyle& style = ImGui::GetStyle();
	uint32 width	= pDisplay->GetWidth();
	uint32 height	= pDisplay->GetHeight();
	float scale = width / 1920.f;
	scale = scale > height / 1080.f ? scale : height / 1080.f;
	style.ScaleAllSizes(scale);
	*/
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
	BeginFrame();

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
