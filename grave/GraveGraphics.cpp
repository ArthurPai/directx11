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
    
    // �إ� texture shader ����
    m_TextureShader = new GraveTextureShader;
    if (!m_TextureShader) {
        return false;
    }

    // ��l�� texture shader ����
    result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
    if (!result) {
        MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
        return false;
    }

    // �إ� light shader ����
    m_LightShader = new GraveLightShader;
    if (!m_LightShader) {
        return false;
    }

    // ��l�� light shader ����
    result = m_LightShader->Initialize(m_Direct3D->GetDevice(), hwnd);
    if (!result) {
        MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
        return false;
    }

    // �إ� Light ����
    m_Light = new GraveLight;
    if (!m_Light)
    {
        return false;
    }

    // ��l�� Light ����A����O���Τ�V�¦VZ�b
    // �ոլݳ]�w�ܫG��ambient color�A�åB��pixel shader ��45�檺 color = saturate(color); ���ѱ�
    m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
    m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light->SetDirection(0.0f, 0.0f, 1.0f);
    m_Light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light->SetSpecularPower(32.0f);

    // �إ� 2D bitmap ����
    m_Bitmap = new GraveBitmap;
    if (!m_Bitmap)
    {
        return false;
    }

    // ��l�� 2D bitmap ����
    result = m_Bitmap->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, "./Data/seafloor.tga", 256, 128);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
        return false;
    }

    // ���J�r��
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

    // ���� Direct3D ����
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

    // �C��Frame��s����q
    rotation += (float)XM_PI * 0.001f;
    if (rotation > 360.0f) {
        rotation -= 360.0f;
    }

    // �C��Frame��s 2D ���󪺦�m
    moveX += 1.0f;
    if (moveX > 544.0f) { // 800 - 256
        moveX = 0.0f;
    }

    // ø�s����
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

    // �M���I��
    m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // �p�� view matrix
    m_Camera->Render();

    // ��� world, view �� projection matrices
    m_Direct3D->GetWorldMatrix(worldMatrix);
    m_Camera->GetViewMatrix(viewMatrix);
    m_Direct3D->GetProjectionMatrix(projectionMatrix);

    // ��� orthogonal matrix
    m_Direct3D->GetOrthoMatrix(orthoMatrix);

    // �ھڶǶi�Ӫ�����q�ӧ�s world �x�}
    XMVECTOR axisY = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX rotateMatrix = XMMatrixRotationNormal(axisY, rotation);
    XMMATRIX midelWorldMatrix = XMMatrixMultiply(worldMatrix, rotateMatrix);

    // ø�s�ҫ��G�N�ҫ��� vertex �� index buffers ��J render pipeline
    m_Model->Render(m_Direct3D->GetDeviceContext());

    // �ϥ� shader ø�s�ҫ�
    result = m_LightShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), midelWorldMatrix, viewMatrix, projectionMatrix,
        m_Model->GetTexture(), m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(),
        m_Camera->GetPosition(), m_Light->GetSpecularColor(), m_Light->GetSpecularPower());
    if (!result)
    {
        return false;
    }

    // ���� Z Buffer
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

    // �}�� Z buffer
    m_Direct3D->TurnZBufferOn();

    // �}�l��X��r
    m_spriteBatch->Begin();

    // Setup the output string.
    wchar_t output[32];
    swprintf(output, L"FPS: %d, Cpu: %d, Time: %0.2f", fps, cpuPercentage, frameTime);

    // ��r���y�СA�����I
    //m_fontPos.x = m_screenWidth / 2.f;
    //m_fontPos.y = m_screenHeight / 2.f;
    //XMVECTOR origin = m_font->MeasureString(output) / 2.f;

    m_fontPos.x = 0.0f;
    m_fontPos.y = m_screenHeight;
    XMVECTOR center = m_font->MeasureString(output);
    XMFLOAT2 origin = XMFLOAT2(0, center.m128_f32[1]);

    m_font->DrawString(m_spriteBatch.get(), output, m_fontPos, Colors::White, 0.f, origin);

    m_spriteBatch->End();

    // ��ܵ��G��ù��W
    m_Direct3D->EndScene();

    return true;
}
