#include <iostream>

#include "Utils/Logger.h"
#include "Utils/Config.h"

int main()
{
    RS::Logger::Init();
    RS::Config::Get()->Init(RS_CONFIG_FILE_PATH);

    
    
    return 0;
}