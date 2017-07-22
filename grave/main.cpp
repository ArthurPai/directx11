#include "GraveSystem.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
    GraveSystem* System;
    bool result;

    // 建立 system 物件
    System = new GraveSystem;
    if (!System)
    {
        return 0;
    }

    // 初始化 system 物件以及執行主迴圈
    result = System->Initialize();
    if (result)
    {
        System->Run();
    }

    // 關閉以及釋放 system 物件
    System->Shutdown();
    delete System;
    System = 0;

    return 0;
}