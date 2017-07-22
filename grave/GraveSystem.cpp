#include "GraveSystem.h"

GraveSystem::GraveSystem()
{
    m_Input = NULL;
    m_Graphics = NULL;
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
    m_Input->Initialize();

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

    return true;
}

void GraveSystem::Shutdown()
{
    // ���� graphics ����
    if (m_Graphics) {
        m_Graphics->Shutdown();
        delete m_Graphics;
        m_Graphics = NULL;
    }

    // ���� input ����
    if (m_Input) {
        delete m_Input;
        m_Input = NULL;
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
    }
}

bool GraveSystem::Frame()
{
    bool result;

    // �ˬd�ϥΪ̬O�_���UESC������
    if (m_Input->IsKeyDown(VK_ESCAPE)) {
        return false;
    }

    // ����ø�Ϥu�@
    result = m_Graphics->Frame();
    if (!result) {
        return false;
    }

    return true;
}

LRESULT CALLBACK GraveSystem::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    switch (umsg)
    {
    // �ˬd�O�_�����U����
    case WM_KEYDOWN:
        // �N������U���T���ǰe�� input ����Ӭ������䪬�A
        m_Input->KeyDown((unsigned int)wparam);
        return 0;
    // �ˬd�O�_���������
    case WM_KEYUP:
        // �N�������񪺰T���ǰe�� input ����Ӭ������䪬�A
        m_Input->KeyUp((unsigned int)wparam);
        return 0;
    // ��L���B�z���T���A���ٵ��t�γB�z
    default:
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
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