#include "GraveGraphics.h"

GraveGraphics::GraveGraphics()
{
    m_Direct3D = NULL;
    m_Camera = NULL;
    m_Model = NULL;
    m_TextureShader = NULL;
    m_LightShader = NULL;
    m_Light = NULL;
    m_Bitmap = NULL;
}

GraveGraphics::~GraveGraphics()
{
}

bool GraveGraphics::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
    bool result;

    // Store the screen size.
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

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
    result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), "./Data/cube.txt", "./Data/stone01.tga");
    if (!result) {
        MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
        return false;
    }
    
    // 建立 texture shader 物件
    m_TextureShader = new GraveTextureShader;
    if (!m_TextureShader) {
        return false;
    }

    // 初始化 texture shader 物件
    result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
    if (!result) {
        MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
        return false;
    }

    // 建立 light shader 物件
    m_LightShader = new GraveLightShader;
    if (!m_LightShader) {
        return false;
    }

    // 初始化 light shader 物件
    result = m_LightShader->Initialize(m_Direct3D->GetDevice(), hwnd);
    if (!result) {
        MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
        return false;
    }

    // 建立 Light 物件
    m_Light = new GraveLight;
    if (!m_Light)
    {
        return false;
    }

    // 初始化 Light 物件，紫色燈光及方向朝向Z軸
    // 試試看設定很亮的ambient color，並且把pixel shader 第45行的 color = saturate(color); 註解掉
    m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
    m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light->SetDirection(0.0f, 0.0f, 1.0f);
    m_Light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light->SetSpecularPower(32.0f);

    // 建立 2D bitmap 物件
    m_Bitmap = new GraveBitmap;
    if (!m_Bitmap)
    {
        return false;
    }

    // 初始化 2D bitmap 物件
    result = m_Bitmap->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, "./Data/seafloor.tga", 256, 128);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
        return false;
    }

    // 載入字型
    m_font = std::make_unique<SpriteFont>(m_Direct3D->GetDevice(), L"courier_new.spritefont");
    m_spriteBatch = std::make_unique<SpriteBatch>(m_Direct3D->GetDeviceContext());
    m_fontPos.x = m_screenWidth / 2.f;
    m_fontPos.y = m_screenHeight / 2.f;

    return true;
}

void GraveGraphics::Shutdown()
{
    if (m_Bitmap)
    {
        m_Bitmap->Shutdown();
        delete m_Bitmap;
        m_Bitmap = 0;
    }

    if (m_Light) {
        delete m_Light;
        m_Light = 0;
    }

    if (m_LightShader) {
        m_LightShader->Shutdown();
        delete m_LightShader;
        m_LightShader = 0;
    }

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

bool GraveGraphics::Frame(int fps, int cpuPercentage, float frameTime)
{
    bool result;

    static float rotation = 0.0f;
    static float moveX = 0.0f;

    // 每個Frame更新旋轉量
    rotation += (float)XM_PI * 0.001f;
    if (rotation > 360.0f) {
        rotation -= 360.0f;
    }

    // 每個Frame更新 2D 物件的位置
    moveX += 1.0f;
    if (moveX > 544.0f) { // 800 - 256
        moveX = 0.0f;
    }

    // 繪製場景
    result = Render(rotation, moveX, fps, cpuPercentage, frameTime);
    if (!result) {
        return false;
    }

    return true;
}

bool GraveGraphics::Render(float rotation, float move, int fps, int cpuPercentage, float frameTime)
{
    XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
    bool result;

    // 清除背景
    m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // 計算 view matrix
    m_Camera->Render();

    // 抓取 world, view 及 projection matrices
    m_Direct3D->GetWorldMatrix(worldMatrix);
    m_Camera->GetViewMatrix(viewMatrix);
    m_Direct3D->GetProjectionMatrix(projectionMatrix);

    // 抓取 orthogonal matrix
    m_Direct3D->GetOrthoMatrix(orthoMatrix);

    // 根據傳進來的旋轉量來更新 world 矩陣
    XMVECTOR axisY = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX rotateMatrix = XMMatrixRotationNormal(axisY, rotation);
    XMMATRIX midelWorldMatrix = XMMatrixMultiply(worldMatrix, rotateMatrix);

    // 繪製模型：將模型的 vertex 及 index buffers 放入 render pipeline
    m_Model->Render(m_Direct3D->GetDeviceContext());

    // 使用 shader 繪製模型
    result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), midelWorldMatrix, viewMatrix, projectionMatrix,
        m_Model->GetTexture(), m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(),
        m_Camera->GetPosition(), m_Light->GetSpecularColor(), m_Light->GetSpecularPower());
    if (!result)
    {
        return false;
    }

    // 關掉 Z Buffer
    m_Direct3D->TurnZBufferOff();

    // Put the bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
    result = m_Bitmap->Render(m_Direct3D->GetDeviceContext(), move, 10);
    if (!result)
    {
        return false;
    }

    // Render the bitmap with the texture shader.
    result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix,
        m_Bitmap->GetTexture());
    if (!result)
    {
        return false;
    }

    // 開啟 Z buffer
    m_Direct3D->TurnZBufferOn();

    // 開始輸出文字
    m_spriteBatch->Begin();

    // Setup the output string.
    wchar_t output[32];
    swprintf(output, L"FPS: %d, Cpu: %d, Time: %0.2f", fps, cpuPercentage, frameTime);

    // 文字的座標，中心點
    //m_fontPos.x = m_screenWidth / 2.f;
    //m_fontPos.y = m_screenHeight / 2.f;
    //XMVECTOR origin = m_font->MeasureString(output) / 2.f;

    m_fontPos.x = 0.0f;
    m_fontPos.y = m_screenHeight;
    XMVECTOR center = m_font->MeasureString(output);
    XMFLOAT2 origin = XMFLOAT2(0, center.m128_f32[1]);

    m_font->DrawString(m_spriteBatch.get(), output, m_fontPos, Colors::White, 0.f, origin);

    m_spriteBatch->End();

    // 顯示結果到螢幕上
    m_Direct3D->EndScene();

    return true;
}
