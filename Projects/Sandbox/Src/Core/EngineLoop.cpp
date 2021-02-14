#include "PreCompiled.h"
#include "EngineLoop.h"

#include "Utils/Logger.h"
#include "Utils/Config.h"
#include "Utils/Timer.h"

#include <glm/glm.hpp>

#include "Display.h"

#include "Renderer/RenderAPI.h"
#include "Renderer/Renderer.h"

void RS::EngineLoop::Init()
{
    Logger::Init();
    Config::Get()->Init(RS_CONFIG_FILE_PATH);

    DisplayDescription displayDesc = {};
    displayDesc.Title = Config::Get()->Fetch<std::string>("Display/Title", "Arcane Engine");
    displayDesc.Width = Config::Get()->Fetch<uint32>("Display/DefaultWidth", 1920);
    displayDesc.Height = Config::Get()->Fetch<uint32>("Display/DefaultHeight", 1080);
    displayDesc.Fullscreen = Config::Get()->Fetch<bool>("Display/Fullscreen", false);
    displayDesc.VSync = Config::Get()->Fetch<bool>("Display/VSync", true);
    Display::Get()->Init(displayDesc);
    RenderAPI::Get()->Init(displayDesc);
    Renderer::Get()->Init(displayDesc);
}

void RS::EngineLoop::Release()
{
    Renderer::Get()->Release();
    RenderAPI::Get()->Release();
    Display::Get()->Release();
}

void RS::EngineLoop::Run()
{
    std::shared_ptr<Display> pDisplay = Display::Get();

    FrameStats frameStats = {};
    const float FIXED_DT = 1.f / frameStats.fixedUpdate.fixedFPS;
    Timer timer;
    TimeStamp frameTime;
    float accumulator = 0.0f;
    float debugTimer = 0.0f;
    uint32 debugFrameCounter = 0;
    uint32 updateCalls = 0;
    uint32 accUpdateCalls = 0;
    while (!pDisplay->ShouldClose())
    {
        frameTime = timer.CalcDelta();
        frameStats.frame.currentDT = frameTime.GetDeltaTimeSec();
        accumulator += frameStats.frame.currentDT;

        pDisplay->PollEvents();

        updateCalls = 0;
        while (accumulator > FIXED_DT && updateCalls < frameStats.fixedUpdate.maxUpdateCalls)
        {
            FixedTick();
            accumulator -= FIXED_DT;
            updateCalls++;
        }
        accUpdateCalls += updateCalls;

        Tick(frameStats);

        frameStats.frame.minDT = glm::min(frameStats.frame.minDT, frameStats.frame.currentDT * 1000.f);
        frameStats.frame.maxDT = glm::max(frameStats.frame.maxDT, frameStats.frame.currentDT * 1000.f);
        debugFrameCounter++;
        debugTimer += frameStats.frame.currentDT;
        if (debugTimer >= 1.0f)
        {
            frameStats.frame.avgFPS = 1.0f / (debugTimer / (float)debugFrameCounter);
            frameStats.frame.avgDTMs = (debugTimer / (float)debugFrameCounter) * 1000.f;
            frameStats.fixedUpdate.updateCallsRatio = ((float)accUpdateCalls / (float)debugFrameCounter) * 100.f;
            LOG_INFO("dt: {}, FPS: {}", frameStats.frame.currentDT, frameStats.frame.avgFPS);
            debugTimer = 0.0f;
            debugFrameCounter = 0;
            accUpdateCalls = 0;
        }
    }
}

void RS::EngineLoop::FixedTick()
{
}

void RS::EngineLoop::Tick(const FrameStats& frameStats)
{
    RS_UNREFERENCED_VARIABLE(frameStats);

    std::shared_ptr<Renderer> renderer = Renderer::Get();
    renderer->BeginScene(0.f, 0.f, 0.f, 1.f);

    renderer->EndScene();
}

void RS::EngineLoop::DrawFrameStats(const FrameStats& frameStats)
{
    RS_UNREFERENCED_VARIABLE(frameStats);
}
