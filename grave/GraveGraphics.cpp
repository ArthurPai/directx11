#include "GraveGraphics.h"

GraveGraphics::GraveGraphics()
{
    m_Direct3D = NULL;
    m_Camera = NULL;
    m_Model = NULL;
    m_TextureShader = NULL;
}

GraveGraphics::~GraveGraphics()
{
}

bool GraveGraphics::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
    bool result;

    // 建立 Direct3D 物件
    m_Direct3D = new GraveD3D;
    if (!m_Direct3D) {
        return false;
    }

    // 初始化 Direct3D 物件
    result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) {
        MessageBox(hwnd, L"Colud not initialize Direct3D", L"Error", MB_OK);
        return false;
    }

    // 建立攝影機物件
    m_Camera = new Camera;
    if (!m_Camera) {
        return false;
    }

    // 設定攝影機位置
    m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

    // 建立模型物件
    m_Model = new Model;
    if (!m_Model) {
        return false;
    }

    // 初始化模型
    result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), "./Data/stone01.tga");
    if (!result) {
        MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
        return false;
    }

    // 建立 shader 物件
    m_TextureShader = new GraveTextureShader;
    if (!m_TextureShader) {
        return false;
    }

    // 初始化 shader 物件
    result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
    if (!result) {
        MessageBox(hwnd, L"Could not initialize the shader object.", L"Error", MB_OK);
        return false;
    }

    return true;
}

void GraveGraphics::Shutdown()
{
    // Release the texture shader object.
    if (m_TextureShader) {
        m_TextureShader->Shutdown();
        delete m_TextureShader;
        m_TextureShader = 0;
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

    // 釋放 Direct3D 物件
    if (m_Direct3D) {
        m_Direct3D->Shutdown();
        delete m_Direct3D;
        m_Direct3D = NULL;
    }
}

bool GraveGraphics::Frame()
{
    bool result;

    // 執行繪圖
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

    // 清除背景
    m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // 計算 view matrix
    m_Camera->Render();

    // 抓取 world, view 及 projection matrices
    m_Direct3D->GetWorldMatrix(worldMatrix);
    m_Camera->GetViewMatrix(viewMatrix);
    m_Direct3D->GetProjectionMatrix(projectionMatrix);

    // 繪製模型：將模型的 vertex 及 index buffers 放入 render pipeline
    m_Model->Render(m_Direct3D->GetDeviceContext());

    // 使用 shader 繪製模型
    result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model->GetTexture());
    if (!result)
    {
        return false;
    }
    // 顯示結果到螢幕上
    m_Direct3D->EndScene();

    return true;
}
