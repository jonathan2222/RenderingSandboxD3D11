#include <iostream>

#include "Core/Application.h"
#include "Scenes/TextureScene.h"
#include "Scenes/MeshScene.h"
#include "Scenes/TessellationScene.h"
#include "Scenes/PBRScene.h"

int main(int argc, char* argv[])
{
    RS_UNREFERENCED_VARIABLE(argc);
    RS_UNREFERENCED_VARIABLE(argv);

#ifdef RS_CONFIG_DEVELOPMENT
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    RS::Application application;
    application.Init();
    application.AddScene(new RS::TextureScene());
    application.AddScene(new RS::MeshScene());
    application.AddScene(new RS::TessellationScene());
    application.AddScene(new RS::PBRScene());
    application.SelectScene(3);
    application.Run();
    application.Release();

    return 0;
}