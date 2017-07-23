#include "GraveGraphics.h"

GraveGraphics::GraveGraphics()
{
    m_Direct3D = NULL;
}

GraveGraphics::~GraveGraphics()
{
}

bool GraveGraphics::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
    bool result;

    // �إ� Direct3D ����
    m_Direct3D = new GraveD3D();
    if (!m_Direct3D) {
        return false;
    }

    // ��l�� Direct3D ����
    result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) {
        MessageBox(hwnd, L"Colud not initialize Direct3D", L"Error", MB_OK);
        return false;
    }

    return true;
}

void GraveGraphics::Shutdown()
{
    // ���� Direct3D ����
    if (m_Direct3D) {
        m_Direct3D->Shutdown();
        delete m_Direct3D;
        m_Direct3D = NULL;
    }
}

bool GraveGraphics::Frame()
{
    bool result;

    // ����ø��
    result = Render();
    if (!result) {
        return false;
    }

    return true;
}

bool GraveGraphics::Render()
{
    // �M���I��
    m_Direct3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

    // ��ܵ��G��ù��W
    m_Direct3D->EndScene();

    return true;
}
