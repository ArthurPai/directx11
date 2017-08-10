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

    // ��l�Ƶ���
    InitializeWindows(screenWidth, screenHeight);

    // �إ� input ����, �o�Ӫ���|�t�d�B�zŪ���ϥΪ̱q��L��J������
    m_Input = new GraveInput;
    if (!m_Input) {
        return false;
    }

    // ��l�� input ���� 
    result = m_Input->Initialize(m_hInstance, m_hwnd, screenWidth, screenHeight);
    if (!result)
    {
        MessageBox(m_hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
        return false;
    }

    // �إ� graphics ����, �o�Ӫ���|�t�dø�Ϫ��\��
    m_Graphics = new GraveGraphics;
    if (!m_Graphics) {
        return false;
    }

    // ��l�� graphics ����
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

    // ���� graphics ����
    if (m_Graphics) {
        m_Graphics->Shutdown();
        delete m_Graphics;
        m_Graphics = NULL;
    }

    // ���� input ����
    if (m_Input)
    {
        m_Input->Shutdown();
        delete m_Input;
        m_Input = 0;
    }

    // ��������
    ShutdownWindows();
}

void GraveSystem::Run()
{
    MSG msg;
    bool done, result;

    // ��l�� msg ��Ƶ��c
    ZeroMemory(&msg, sizeof(MSG));

    // �T���L�a�j��A����ϥΪ̿�J���}���T��
    done = false;
    while(!done) {
        // �B�z�����T��
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // �p�Gwindows�o�X���}���T���A�h�����j��
        if (msg.message == WM_QUIT) {
            done = true;
        }
        else {
            // �_�h�I�sFrame()����ø�Ϥu�@
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

    // �I�s Direct Input �B�z�y�{
    result = m_Input->Frame();
    if (!result)
    {
        return false;
    }

    // ���o�ƹ���m
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

    // ����ø�Ϥu�@
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

    // ������ε{����instance
    m_hInstance = GetModuleHandle(NULL);

    // �]�w���ε{�����W��
    m_applicationName = L"Grave Engine";

    // �]�wwindows class
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

    // ���Uwindows class
    RegisterClassEx(&wc);

    // ����ù��ѪR��
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // �]�w�����j�p�A�ھ�FULL_SCREEN�ӨM�w�O�_�n���ù�
    if (FULL_SCREEN) {
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
        dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // �]�w���ù�
        ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

        // �]�w������m��ù����W��
        posX = posY = 0;
    }
    else {
        // �]�w�ѪR�׬�800x600
        screenWidth = 800;
        screenHeight = 600;

        // �N������b�ù�������
        posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
    }

    // �إߵ����H�����������Handle
    m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
        posX, posY, screenWidth, screenHeight, NULL, NULL, m_hInstance, NULL);

    // ��ܵ����A�ó]���J�I
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);

    // ���÷ƹ����
    ShowCursor(false);
}

void GraveSystem::ShutdownWindows()
{
    // ��ܷƹ����
    ShowCursor(true);

    // ���}���ù�
    if (FULL_SCREEN) {
        ChangeDisplaySettings(NULL, 0);
    }

    // ��������
    DestroyWindow(m_hwnd);
    m_hwnd = NULL;

    // �������ε{��instance
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
