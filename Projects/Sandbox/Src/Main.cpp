#include <iostream>

#include "Core/Application.h"
#include "Scenes/SandboxScene.h"
#include "Scenes/MeshScene.h"

int main(int argc, char* argv[])
{
    RS_UNREFERENCED_VARIABLE(argc);
    RS_UNREFERENCED_VARIABLE(argv);

#ifdef RS_CONFIG_DEVELOPMENT
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    RS::Application application;
    application.Init();
    //application.AddScene(new RS::SandboxScene());
    application.AddScene(new RS::MeshScene());
    application.Run();
    application.Release();

    return 0;
}