#include "PreCompiled.h"
#include "EngineLoop.h"

#include "Utils/Logger.h"
#include "Utils/Config.h"
#include "Utils/Maths.h"
#include "FrameTimer.h"

#include "Core/Display.h"
#include "Core/Input.h"

#include "Renderer/RenderAPI.h"
#include "Renderer/Renderer.h"
#include "Renderer/DebugRenderer.h"
#include "Renderer/ImGuiRenderer.h"
#include "Renderer/ShaderHotReloader.h"

void RS::EngineLoop::Init(std::function<void(void)> fixedTickCallback, std::function<void(float)> tickCallback)
{
    m_FixedTickCallback   = fixedTickCallback;
    m_TickCallback        = tickCallback;

    Logger::Init();
    Config::Get()->Init(RS_CONFIG_FILE_PATH);

    DisplayDescription displayDesc = {};
    displayDesc.Title = Config::Get()->Fetch<std::string>("Display/Title", "Arcane Engine");
    displayDesc.Width = Config::Get()->Fetch<uint32>("Display/DefaultWidth", 1920);
    displayDesc.Height = Config::Get()->Fetch<uint32>("Display/DefaultHeight", 1080);
    displayDesc.Fullscreen = Config::Get()->Fetch<bool>("Display/Fullscreen", false);
    displayDesc.VSync = Config::Get()->Fetch<bool>("Display/VSync", true);
    Display::Get()->Init(displayDesc);
    Input::Get()->Init();
    RenderAPI::Get()->Init(displayDesc);
    Renderer::Get()->Init(displayDesc);
    DebugRenderer::Get()->Init();
    ImGuiRenderer::Init(Display::Get().get());

    ShaderHotReloader::Init();
}

void RS::EngineLoop::Release()
{
    ShaderHotReloader::Release();
    ImGuiRenderer::Release();
    DebugRenderer::Get()->Release();
    Renderer::Get()->Release();
    RenderAPI::Get()->Release();
    Display::Get()->Release();
}

void RS::EngineLoop::Run()
{
    std::shared_ptr<Display> pDisplay = Display::Get();

    FrameStats frameStats = {};
    FrameTimer frameTimer;
    frameTimer.Init(&frameStats, 0.25f);
    while (!pDisplay->ShouldClose())
    {
        frameTimer.Begin();

        pDisplay->PollEvents();
        Input::Get()->Update();

        if (Input::Get()->IsKeyPressed(Key::ESCAPE))
            pDisplay->Close();
        
        frameTimer.FixedTick([&]() { FixedTick(); });
        Tick(frameStats);

        frameTimer.End();
    }
}

void RS::EngineLoop::FixedTick()
{
    m_FixedTickCallback();
}

void RS::EngineLoop::Tick(const FrameStats& frameStats)
{
    DrawFrameStats(frameStats);

    ShaderHotReloader::Update();

    std::shared_ptr<Renderer> renderer = Renderer::Get();
    renderer->BeginScene(0.f, 0.f, 0.f, 1.f);

    m_TickCallback(frameStats.frame.currentDT);

    DebugRenderer::Get()->Render();

    ImGuiRenderer::Render();
    renderer->Present();
}

void RS::EngineLoop::DrawFrameStats(const FrameStats& frameStats)
{
    ImGuiRenderer::Draw([&]() {
        auto Lerp = [](ImVec4 c1, ImVec4 c2, float x)-> ImVec4
        {
            ImVec4 res;
            res.x = (1.f - x) * c1.x + x * c2.x;
            res.y = (1.f - x) * c1.y + x * c2.y;
            res.z = (1.f - x) * c1.z + x * c2.z;
            res.w = (1.f - x) * c1.w + x * c2.w;
            return res;
        };

        auto CLerp = [&](float x)->ImVec4
        {
            return Lerp(ImVec4(0.f, 1.f, 0.f, 1.f), ImVec4(1.f, 0.f, 0.f, 1.f), x);
        };

        auto Norm = [](float min, float max, float x)->float
        {
            return (x - min) / (max - min);
        };

        auto OnOffText = [](bool state, const std::string& on, const std::string& off) 
        {
            ImGui::TextColored(state ? ImVec4(0.f, 1.f, 0.f, 1.f) : ImVec4(1.f, 0.f, 0.f, 1.f), "%s", state ? on.c_str() : off.c_str());
        };

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize;

        uint32_t displayWidth = Display::Get()->GetWidth();
        const uint32_t width = 260;
        const uint32_t height = 405;

        // Draw the stats in the top right corner.
        ImGui::SetNextWindowPos(ImVec2((float)displayWidth - width, 0));
        ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));
        ImGui::SetNextWindowBgAlpha(0.5f);

        ImGui::PushStyleColor(ImGuiCol_Border, 0);
        ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0);
        ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, 0);
        ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, 0);

        if (ImGui::Begin("Frame stats", (bool*)0, flags))
        {
            float v = 0.f;
            ImGui::Text("Frame");
            {
                ImGui::Indent();
                ImGui::Text("Avrage DT:", frameStats.frame.avgDTMs, (uint32_t)frameStats.frame.avgFPS);
                v = frameStats.frame.avgDTMs;
                ImGui::SameLine(); ImGui::TextColored(CLerp(Norm(0.f, 50.f, v)), "%.2f ms (FPS: %d)", frameStats.frame.avgDTMs, (uint32_t)frameStats.frame.avgFPS);
                ImGui::Text("Min DT:");
                v = frameStats.frame.minDT;
                ImGui::SameLine(); ImGui::TextColored(CLerp(Norm(0.f, 50.f, v)), "%.2f ms", v);
                ImGui::Text("Max DT:");
                v = frameStats.frame.maxDT;
                ImGui::SameLine(); ImGui::TextColored(CLerp(Norm(0.f, 50.f, v)), "%.2f ms", v);
                ImGui::Unindent();
            }

            ImGui::NewLine();
            ImGui::Text("Fixed Update");
            {
                ImGui::Indent();
                ImGui::Text("Fixed FPS: %d", (uint32_t)frameStats.fixedUpdate.fixedFPS);
                ImGui::Text("Calls:");
                v = frameStats.fixedUpdate.updateCallsRatio;
                ImGui::SameLine(); ImGui::TextColored(CLerp(glm::max(v * 0.01f, 1.f) - 1.f), "%.2f%%", v);
                ImGui::Unindent();
            }

            ImGui::NewLine();
            ImGui::Text("Display");
            {
                DisplayDescription& displayDesc = Display::Get()->GetDescription();
                ImGui::Indent();
                ImGui::Text("VSync:");
                ImGui::SameLine(); OnOffText(displayDesc.VSync, "On", "Off");
                ImGui::Text("Width: %d", displayDesc.Width);
                ImGui::Text("Height: %d", displayDesc.Height);
                ImGui::Unindent();
            }

            ImGui::NewLine();
            ImGui::Text("Renderer");
            {
#ifdef RS_CONFIG_DEBUG
                char* configStr = "DEBUG";
#elif defined(RS_CONFIG_RELEASE)
                char* configStr = "RELEASE";
#else
                char* configStr = "PRODUCTION";
#endif
                ImGui::Indent();
                ImGui::Text("Config: %s", configStr);
                RenderAPI::VideoCardInfo& videoCardInfo = RenderAPI::Get()->GetVideoCardInfo();
                ImGui::Text("%s", videoCardInfo.Name.c_str());
                ImGui::Unindent();
            }

            ImGui::NewLine();
            ImGui::Text("Debug Renderer");
            {
                const DebugRenderer::Stats& stats = DebugRenderer::Get()->GetStats();
                ImGui::Indent();
                ImGui::Text("Num line vertices: %d", stats.NumberOfLineVertices);
                ImGui::Text("Num point vertices: %d", stats.NumberOfPointVertices);
                ImGui::Text("Num IDs: %d", stats.NumberOfIDs);
                ImGui::Unindent();
            }

        }
        ImGui::End();

        ImGui::PopStyleColor(4);
    });
}
