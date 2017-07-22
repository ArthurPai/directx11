#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "GraveInput.h"
#include "GraveGraphics.h"

class GraveSystem
{
public:
    GraveSystem();
    ~GraveSystem();

    bool Initialize();
    void Shutdown();
    void Run();

    LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

private:
    void InitializeWindows(int &screenWidth, int &screenHeight);
    void ShutdownWindows();
    bool Frame();

private:
    LPCWSTR m_applicationName;
    HINSTANCE m_hInstance;
    HWND m_hwnd;

    GraveInput* m_Input;
    GraveGraphics* m_Graphics;
};

static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
static GraveSystem* ApplicationHandle = 0;
