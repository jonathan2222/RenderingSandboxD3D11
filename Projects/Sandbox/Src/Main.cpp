#include <iostream>

#include "Core/EngineLoop.h"

int main(int argc, char* argv[])
{
    RS_UNREFERENCED_VARIABLE(argc);
    RS_UNREFERENCED_VARIABLE(argv);

#ifdef RS_CONFIG_DEVELOPMENT
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    RS::EngineLoop engineLoop;
    engineLoop.Init();
    engineLoop.Run();
    engineLoop.Release();

    return 0;
}