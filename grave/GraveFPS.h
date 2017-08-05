#pragma once
#pragma comment(lib, "winmm.lib")

#include <windows.h>
#include <mmsystem.h>

class GraveFPS
{
public:
    GraveFPS();
    ~GraveFPS();

    void Initialize();
    void Frame();
    int GetFps();

private:
    int m_fps, m_count;
    unsigned long m_startTime;
};

