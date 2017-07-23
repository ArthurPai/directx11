#include "GraveGraphics.h"

GraveGraphics::GraveGraphics()
{
    m_Direct3D = NULL;
    m_Camera = NULL;
    m_Model = NULL;
    m_ColorShader = NULL;
}

GraveGraphics::~GraveGraphics()
{
}

bool GraveGraphics::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
    bool result;

    // �إ� Direct3D ����
    m_Direct3D = new GraveD3D;
    if (!m_Direct3D) {
        return false;
    }

    // ��l�� Direct3D ����
    result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) {
        MessageBox(hwnd, L"Colud not initialize Direct3D", L"Error", MB_OK);
        return false;
    }

    // �إ���v������
    m_Camera = new Camera;
    if (!m_Camera) {
        return false;
    }

    // �]�w��v����m
    m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

    // �إ߼ҫ�����
    m_Model = new Model;
    if (!m_Model) {
        return false;
    }

    // ��l�Ƽҫ�
    result = m_Model->Initialize(m_Direct3D->GetDevice());
    if (!result) {
        MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
        return false;
    }

    // �إ� color shader ����
    m_ColorShader = new ColorShader;
    if (!m_ColorShader) {
        return false;
    }

    // ��l�� color shader ����
    result = m_ColorShader->Initialize(m_Direct3D->GetDevice(), hwnd);
    if (!result) {
        MessageBox(hwnd, L"Could not initialize the color shader object.", L"Error", MB_OK);
        return false;
    }

    return true;
}

void GraveGraphics::Shutdown()
{
    if (m_ColorShader) {
        m_ColorShader->Shutdown();
        delete m_ColorShader;
        m_ColorShader = 0;
    }

    if (m_Model) {
        m_Model->Shutdown();
        delete m_Model;
        m_Model = 0;
    }

    if (m_Camera) {
        delete m_Camera;
        m_Camera = 0;
    }

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
    XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
    bool result;

    // �M���I��
    m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // �p�� view matrix
    m_Camera->Render();

    // ��� world, view �� projection matrices
    m_Direct3D->GetWorldMatrix(worldMatrix);
    m_Camera->GetViewMatrix(viewMatrix);
    m_Direct3D->GetProjectionMatrix(projectionMatrix);

    // ø�s�ҫ��G�N�ҫ��� vertex �� index buffers ��J render pipeline
    m_Model->Render(m_Direct3D->GetDeviceContext());

    // �ϥ� color shader ø�s�ҫ�
    result = m_ColorShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
    if (!result) {
        return false;
    }

    // ��ܵ��G��ù��W
    m_Direct3D->EndScene();

    return true;
}
