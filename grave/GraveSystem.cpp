#include "GraveSystem.h"

GraveSystem::GraveSystem()
{
    m_Input = NULL;
    m_Graphics = NULL;
    m_Fps = NULL;
    m_Cpu = NULL;
    m_Timer = NULL;
}

GraveSystem::~GraveSystem()
{
}

bool GraveSystem::Initialize()
{
    int screenWidth = 0, screenHeight = 0;
    bool result;

    // 初始化視窗
    InitializeWindows(screenWidth, screenHeight);

    // 建立 input 物件, 這個物件會負責處理讀取使用者從鍵盤輸入的按鍵
    m_Input = new GraveInput;
    if (!m_Input) {
        return false;
    }

    // 初始化 input 物件 
    result = m_Input->Initialize(m_hInstance, m_hwnd, screenWidth, screenHeight);
    if (!result)
    {
        MessageBox(m_hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
        return false;
    }

    // 建立 graphics 物件, 這個物件會負責繪圖的功能
    m_Graphics = new GraveGraphics;
    if (!m_Graphics) {
        return false;
    }

    // 初始化 graphics 物件
    result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
    if (!result) {
        return false;
    }

    // Create the fps object.
    m_Fps = new GraveFPS;
    if (!m_Fps)
    {
        return false;
    }

    // Initialize the fps object.
    m_Fps->Initialize();
    
    // Create the cpu object.
    m_Cpu = new GraveCpuProfile;
    if (!m_Cpu)
    {
        return false;
    }

    // Initialize the cpu object.
    m_Cpu->Initialize();

    // Create the timer object.
    m_Timer = new GraveTimer;
    if (!m_Timer)
    {
        return false;
    }

    // Initialize the timer object.
    result = m_Timer->Initialize();
    if (!result)
    {
        MessageBox(m_hwnd, L"Could not initialize the Timer object.", L"Error", MB_OK);
        return false;
    }

    return true;
}

void GraveSystem::Shutdown()
{
    if (m_Timer)
    {
        delete m_Timer;
        m_Timer = 0;
    }

    if (m_Cpu) {
        m_Cpu->Shutdown();
        delete m_Cpu;
        m_Cpu = 0;
    }

    if (m_Fps) {
        delete m_Fps;
        m_Fps = 0;
    }

    // 釋放 graphics 物件
    if (m_Graphics) {
        m_Graphics->Shutdown();
        delete m_Graphics;
        m_Graphics = NULL;
    }

    // 釋放 input 物件
    if (m_Input)
    {
        m_Input->Shutdown();
        delete m_Input;
        m_Input = 0;
    }

    // 關閉視窗
    ShutdownWindows();
}

void GraveSystem::Run()
{
    MSG msg;
    bool done, result;

    // 初始化 msg 資料結構
    ZeroMemory(&msg, sizeof(MSG));

    // 訊息無窮迴圈，直到使用者輸入離開的訊息
    done = false;
    while(!done) {
        // 處理視窗訊息
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 如果windows發出離開的訊息，則結束迴圈
        if (msg.message == WM_QUIT) {
            done = true;
        }
        else {
            // 否則呼叫Frame()執行繪圖工作
            result = Frame();
            if (!result) {
                done = true;
            }
        }

        // Check if the user pressed escape and wants to quit.
        if (m_Input->IsEscapePressed() == true)
        {
            done = true;
        }
    }
}

bool GraveSystem::Frame()
{
    bool result;
    int mouseX, mouseY;

    // 呼叫 Direct Input 處理流程
    result = m_Input->Frame();
    if (!result)
    {
        return false;
    }

    // 取得滑鼠位置
    m_Input->GetMouseLocation(mouseX, mouseY);

    // Update the system stats.
	m_Timer->Frame();
    m_Fps->Frame();
    m_Cpu->Frame();

    //POINT point;
    //GetCursorPos(&point);
    //ScreenToClient(m_hwnd, &point);

	if (m_Input->IsUpPressed()) {
		m_Graphics->MoveCamera(0, 0, 1);
	}
	else if (m_Input->IsDownPressed()) {
		m_Graphics->MoveCamera(0, 0, -1);
	}

    // 執行繪圖工作
    result = m_Graphics->Frame(mouseX, mouseY, m_Fps->GetFps(), m_Cpu->GetCpuPercentage(), m_Timer->GetTime());
    if (!result) {
        return false;
    }

    return true;
}

LRESULT CALLBACK GraveSystem::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    return DefWindowProc(hwnd, umsg, wparam, lparam);
}

void GraveSystem::InitializeWindows(int &screenWidth, int &screenHeight)
{
    WNDCLASSEX wc;
    DEVMODE dmScreenSettings;
    int posX, posY;

    // Save this grave system to global point
    ApplicationHandle = this;

    // 抓取應用程式的instance
    m_hInstance = GetModuleHandle(NULL);

    // 設定應用程式的名稱
    m_applicationName = L"Grave Engine";

    // 設定windows class
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = m_applicationName;
    wc.cbSize = sizeof(WNDCLASSEX);

    // 註冊windows class
    RegisterClassEx(&wc);

    // 抓取螢幕解析度
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 設定視窗大小，根據FULL_SCREEN來決定是否要全螢幕
    if (FULL_SCREEN) {
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
        dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // 設定全螢幕
        ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

        // 設定視窗位置到螢幕左上角
        posX = posY = 0;
    }
    else {
        // 設定解析度為800x600
        screenWidth = 800;
        screenHeight = 600;

        // 將視窗放在螢幕正中央
        posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
    }

    // 建立視窗以及獲取視窗的Handle
    m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
        posX, posY, screenWidth, screenHeight, NULL, NULL, m_hInstance, NULL);

    // 顯示視窗，並設為焦點
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);

    // 隱藏滑鼠游標
    ShowCursor(false);
}

void GraveSystem::ShutdownWindows()
{
    // 顯示滑鼠游標
    ShowCursor(true);

    // 離開全螢幕
    if (FULL_SCREEN) {
        ChangeDisplaySettings(NULL, 0);
    }

    // 關閉視窗
    DestroyWindow(m_hwnd);
    m_hwnd = NULL;

    // 移除應用程式instance
    UnregisterClass(m_applicationName, m_hInstance);
    m_hInstance = NULL;
    
    ApplicationHandle = NULL;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    switch (umessage)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    default:
        return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
    }
}
