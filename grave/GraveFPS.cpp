#include "GraveFPS.h"

GraveFPS::GraveFPS()
{
    m_fps = 0;
    m_count = 0;
    m_startTime = timeGetTime();
}

GraveFPS::~GraveFPS()
{
}

void GraveFPS::Initialize()
{
    m_fps = 0;
    m_count = 0;
    m_startTime = timeGetTime();
}

void GraveFPS::Frame()
{
    m_count++;

    if (timeGetTime() >= (m_startTime + 1000))
    {
        m_fps = m_count;
        m_count = 0;

        m_startTime = timeGetTime();
    }
}

int GraveFPS::GetFps()
{
    return m_fps;
}