#include "GraveSystem.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
    GraveSystem* System;
    bool result;

    // �إ� system ����
    System = new GraveSystem;
    if (!System)
    {
        return 0;
    }

    // ��l�� system ����H�ΰ���D�j��
    result = System->Initialize();
    if (result)
    {
        System->Run();
    }

    // �����H������ system ����
    System->Shutdown();
    delete System;
    System = 0;

    return 0;
}