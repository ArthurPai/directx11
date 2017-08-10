#pragma once
#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

class GraveInput
{
public:
    GraveInput();
    ~GraveInput();

    bool Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
    void Shutdown();
    bool Frame();

	bool IsEscapePressed();
	bool IsUpPressed();
	bool IsDownPressed();
    void GetMouseLocation(int&, int&);

private:
    bool ReadKeyboard();
    bool ReadMouse();
    void ProcessInput();

private:
    IDirectInput8* m_directInput;
    IDirectInputDevice8* m_keyboard;
    IDirectInputDevice8* m_mouse;

    unsigned char m_keyboardState[256];
    DIMOUSESTATE m_mouseState;

    int m_screenWidth, m_screenHeight;
    int m_mouseX, m_mouseY;
};

