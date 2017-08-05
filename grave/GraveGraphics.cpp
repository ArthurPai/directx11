#include "GraveGraphics.h"

GraveGraphics::GraveGraphics()
{
    m_Direct3D = NULL;
    m_Camera = NULL;
    m_Model = NULL;
    m_LightShader = NULL;
    m_Light = NULL;
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
    result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), "./Data/cube.txt", "./Data/stone01.tga");
    if (!result) {
        MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
        return false;
    }
    
    // �إ� shader ����
    m_LightShader = new GraveLightShader;
    if (!m_LightShader) {
        return false;
    }

    // ��l�� shader ����
    result = m_LightShader->Initialize(m_Direct3D->GetDevice(), hwnd);
    if (!result) {
        MessageBox(hwnd, L"Could not initialize the shader object.", L"Error", MB_OK);
        return false;
    }

    // �إ� Light ����
    m_Light = new GraveLight;
    if (!m_Light)
    {
        return false;
    }

    // ��l�� Light ����A����O���Τ�V�¦VZ�b
    m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light->SetDirection(0.0f, 0.0f, 1.0f);

    return true;
}

void GraveGraphics::Shutdown()
{
    if (m_Light) {
        delete m_Light;
        m_Light = 0;
    }

    if (m_LightShader) {
        m_LightShader->Shutdown();
        delete m_LightShader;
        m_LightShader = 0;
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

    static float rotation = 0.0f;

    // �C��Frame��s����q
    rotation += (float)XM_PI * 0.001f;
    if (rotation > 360.0f) {
        rotation -= 360.0f;
    }

    // ø�s����
    result = Render(rotation);
    if (!result) {
        return false;
    }

    return true;
}

bool GraveGraphics::Render(float rotation)
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

    // �ھڶǶi�Ӫ�����q�ӧ�s world �x�}
    XMVECTOR axisY = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX rotateMatrix = XMMatrixRotationNormal(axisY, rotation);
    worldMatrix = XMMatrixMultiply(worldMatrix, rotateMatrix);

    // ø�s�ҫ��G�N�ҫ��� vertex �� index buffers ��J render pipeline
    m_Model->Render(m_Direct3D->GetDeviceContext());

    // �ϥ� shader ø�s�ҫ�
    result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
        m_Model->GetTexture(), m_Light->GetDirection(), m_Light->GetDiffuseColor());
    if (!result)
    {
        return false;
    }

    // ��ܵ��G��ù��W
    m_Direct3D->EndScene();

    return true;
}
