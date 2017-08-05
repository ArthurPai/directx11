#include "GraveD3D.h"

GraveD3D::GraveD3D()
{
    m_swapChain = NULL;
    m_device = NULL;
    m_deviceContext = NULL;
    m_renderTargetView = NULL;
    m_depthStencilBuffer = NULL;
    m_depthDisabledStencilState = NULL;
    m_depthStencilState = NULL;
    m_depthStencilView = NULL;
    m_rasterState = NULL;
}


GraveD3D::~GraveD3D()
{
}

bool GraveD3D::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen,
    float screenDepth, float screenNear)
{
    HRESULT result;
    IDXGIFactory* factory;
    IDXGIAdapter* adapter;
    IDXGIOutput* adapterOutput;
    unsigned int numModes, i, numerator, denominator;
    size_t stringLength;
    DXGI_MODE_DESC* displayModeList;
    DXGI_ADAPTER_DESC adapterDesc;
    int error;
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ID3D11Texture2D* backBufferPtr;
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    D3D11_RASTERIZER_DESC rasterDesc;
    D3D11_VIEWPORT viewport;
    float fieldOfView, screenAspect;

    m_vsync_enabled = vsync;

    // �إ� DirectX graphics interface factory
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    if (FAILED(result)) {
        return false;
    }

    // �ϥ� factory ���o�D�n��ܥd(adapter) 
    result = factory->EnumAdapters(0, &adapter);
    if (FAILED(result)) {
        return false;
    }

    // ���o�D��ܥd���D�n��X�]��(�ù�)
    result = adapter->EnumOutputs(0, &adapterOutput);
    if (FAILED(result)) {
        return false;
    }

    // ���o�D�n��X�]�Ư���䴩 DXGI_FORMAT_R8G8B8A8_UNORM �Ҧ����ƶq
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
    if (FAILED(result)) {
        return false;
    }

    // �إߥΨ��x�s��ܥd/�ù���T��䴩DXGI_FORMAT_R8G8B8A8_UNORM�Ҧ����}�C
    displayModeList = new DXGI_MODE_DESC[numModes];
    if (!displayModeList) {
        return false;
    }

    // ���o��ܥd/�ù���䴩DXGI_FORMAT_R8G8B8A8_UNORM�Ҧ�����T
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
    if (FAILED(result)) {
        return false;
    }

    // �q���o���C���A��X�䤤��䴩���w�ѪR�ת���ܼҦ��A�M�������s�W�v���ƭ�
    for (i = 0; i < numModes; i++) {
        if (displayModeList[i].Width == (unsigned int)screenWidth) {
            if (displayModeList[i].Height == (unsigned int)screenHeight) {
                numerator = displayModeList[i].RefreshRate.Numerator;
                denominator = displayModeList[i].RefreshRate.Numerator;
            }
        }
    }

    // ���o��ܥd����T
    result = adapter->GetDesc(&adapterDesc);
    if (FAILED(result)) {
        return false;
    }

    // ���o��ܥd���O����(MB)
    m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

    // �x�s��ܥd�W��
    error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
    if (error != 0)
    {
        return false;
    }

    // ����O����
    delete[] displayModeList;
    displayModeList = NULL;

    adapterOutput->Release();
    adapterOutput = NULL;

    adapter->Release();
    adapter = NULL;

    factory->Release();
    factory = NULL;

    // ��l�� swap chain description
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    // �]�w�@�� Back Buffer 
    swapChainDesc.BufferCount = 1;

    // �]�w Back Buffer �e���j�p
    swapChainDesc.BufferDesc.Width = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;

    // �]�w Back Buffer ���e���榡
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // �]�w Back Buffer ��s�W�v(refresh rate)
    if (m_vsync_enabled) {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
    }
    else {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }

    // �]�w Back Buffure ���γ~��ø�ϥؼ�
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // �]�w�nø�s����@�ӵ���
    swapChainDesc.OutputWindow = hwnd;

    // �����h������(multisampling off)
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    // �]�w�O�_���ù�
    if (fullscreen) {
        swapChainDesc.Windowed = false;
    }
    else {
        swapChainDesc.Windowed = true;
    }

    // �]�w���y����(scan line oredering)
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

    // �]�w�Y��
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Back Buffer ��ܫ�A���e�N���
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // ���]�w�i���\��
    swapChainDesc.Flags = 0;

    // �]�w�ϥ� DirectX 11 ���\��
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    /********************************************************************
     * �إ� swap chain, Direct3D device, Direct3D device context
     */
    result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, numFeatureLevels,
        D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
    if (FAILED(result)) {
        return false;
    }

    // ���o Back Buffer ����
    result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
    if (FAILED(result)) {
        return false;
    }

    // �ϥ� Back Buffer �إ� Render Target View
    result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
    if (FAILED(result)) {
        return false;
    }

    // ���� Back Buffer ����
    backBufferPtr->Release();
    backBufferPtr = NULL;

    // ��l�� depth buffer description
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

    // �]�w depth buffer description
    depthBufferDesc.Width = screenWidth;
    depthBufferDesc.Height = screenHeight;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    // �إ� depth buffer
    result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
    if (FAILED(result)) {
        return false;
    }

    // ��l�� stencil state description
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    // �]�w stencil state description
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // �إ� depth stencil state
    result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
    if (FAILED(result)) {
        return false;
    }

    // �Ұ� depth stencil state
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

    // ��l�� depth stencil view description
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // �]�w depth stencil view description
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // �إ� depth stencil view
    result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
    if (FAILED(result)) {
        return false;
    }

    // �j�w render target view �� depth stencil buffer �� render pipeline
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

    // �]�w raster description�A����DirectX �p��ø�s polygons
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.ScissorEnable = false;
    rasterDesc.SlopeScaledDepthBias = 0.0f;

    // �إ� rasterizer state
    result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
    if (FAILED(result))
    {
        return false;
    }

    // �]�w rasterizer state
    m_deviceContext->RSSetState(m_rasterState);

    // �]�w viewport
    viewport.Width = (float)screenWidth;
    viewport.Height = (float)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    // �إ� viewport
    m_deviceContext->RSSetViewports(1, &viewport);

    // �]�w��v�x�}(projection matrix)
    fieldOfView = 3.141592654f / 4.0f; // 45��
    screenAspect = (float)screenWidth / (float)screenHeight;

    // �إߧ�v�x�}(projection matrix)
    m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

    // ��l�� world matrix �����x�}
    m_worldMatrix = XMMatrixIdentity();

    // �إ� orthographic projection matrix�A�Ψ�ø�s2D�e���A�Ҧp��r
    m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);

    // Clear the second depth stencil state before setting the parameters.
    ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

    // �إ߲ĤG�� depth stencil state ��description�A��2Dø�Ϯɤ��ϥ� Z buffer �\��C�Ҧ��]�w����e�@�� depth stencil state �@�ˡA���F DepthEnable �]�� false�C
    depthDisabledStencilDesc.DepthEnable = false;
    depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthDisabledStencilDesc.StencilEnable = true;
    depthDisabledStencilDesc.StencilReadMask = 0xFF;
    depthDisabledStencilDesc.StencilWriteMask = 0xFF;
    depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // �إ� depth stencil state
    result = m_device->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void GraveD3D::Shutdown()
{
    // �������e���������^�����Ҧ��A�_�h�b���� swap chain �ɷ|���� exception
    if (m_swapChain)
    {
        m_swapChain->SetFullscreenState(false, NULL);
    }

    if (m_rasterState)
    {
        m_rasterState->Release();
        m_rasterState = 0;
    }

    if (m_depthStencilView)
    {
        m_depthStencilView->Release();
        m_depthStencilView = 0;
    }

    if (m_depthDisabledStencilState)
    {
        m_depthDisabledStencilState->Release();
        m_depthDisabledStencilState = 0;
    }

    if (m_depthStencilState)
    {
        m_depthStencilState->Release();
        m_depthStencilState = 0;
    }

    if (m_depthStencilBuffer)
    {
        m_depthStencilBuffer->Release();
        m_depthStencilBuffer = 0;
    }

    if (m_renderTargetView)
    {
        m_renderTargetView->Release();
        m_renderTargetView = 0;
    }

    if (m_deviceContext)
    {
        m_deviceContext->Release();
        m_deviceContext = 0;
    }

    if (m_device)
    {
        m_device->Release();
        m_device = 0;
    }

    if (m_swapChain)
    {
        m_swapChain->Release();
        m_swapChain = 0;
    }
}

void GraveD3D::BeginScene(float red, float green, float blue, float alpha)
{
    float color[4];

    // �]�w�I���C��
    color[0] = red;
    color[1] = green;
    color[2] = blue;
    color[3] = alpha;

    // �M�� back buffer
    m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

    // �M�� depth buffer
    m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    return;
}

void GraveD3D::EndScene()
{
    // ���(Present) back buffer ��ù��W
    if (m_vsync_enabled)
    {
        // ��w��s�W�v
        m_swapChain->Present(1, 0);
    }
    else
    {
        // ����w��s�W�v�A��]�h�ִN�]�h��
        m_swapChain->Present(0, 0);
    }
}

ID3D11Device* GraveD3D::GetDevice()
{
    return m_device;
}


ID3D11DeviceContext* GraveD3D::GetDeviceContext()
{
    return m_deviceContext;
}

void GraveD3D::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
    projectionMatrix = m_projectionMatrix;
    return;
}


void GraveD3D::GetWorldMatrix(XMMATRIX& worldMatrix)
{
    worldMatrix = m_worldMatrix;
    return;
}


void GraveD3D::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
    orthoMatrix = m_orthoMatrix;
    return;
}

void GraveD3D::GetVideoCardInfo(char* cardName, int& memory)
{
    strcpy_s(cardName, 128, m_videoCardDescription);
    memory = m_videoCardMemory;
    return;
}

void GraveD3D::TurnZBufferOn()
{
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
    return;
}

void GraveD3D::TurnZBufferOff()
{
    m_deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
    return;
}
