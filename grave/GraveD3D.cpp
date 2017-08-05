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

    // 建立 DirectX graphics interface factory
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    if (FAILED(result)) {
        return false;
    }

    // 使用 factory 取得主要顯示卡(adapter) 
    result = factory->EnumAdapters(0, &adapter);
    if (FAILED(result)) {
        return false;
    }

    // 取得主顯示卡的主要輸出設備(螢幕)
    result = adapter->EnumOutputs(0, &adapterOutput);
    if (FAILED(result)) {
        return false;
    }

    // 取得主要輸出設備能夠支援 DXGI_FORMAT_R8G8B8A8_UNORM 模式的數量
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
    if (FAILED(result)) {
        return false;
    }

    // 建立用來儲存顯示卡/螢幕資訊能支援DXGI_FORMAT_R8G8B8A8_UNORM模式的陣列
    displayModeList = new DXGI_MODE_DESC[numModes];
    if (!displayModeList) {
        return false;
    }

    // 取得顯示卡/螢幕能支援DXGI_FORMAT_R8G8B8A8_UNORM模式的資訊
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
    if (FAILED(result)) {
        return false;
    }

    // 從取得的列表中，找出其中能支援指定解析度的顯示模式，然後紀錄刷新頻率的數值
    for (i = 0; i < numModes; i++) {
        if (displayModeList[i].Width == (unsigned int)screenWidth) {
            if (displayModeList[i].Height == (unsigned int)screenHeight) {
                numerator = displayModeList[i].RefreshRate.Numerator;
                denominator = displayModeList[i].RefreshRate.Numerator;
            }
        }
    }

    // 取得顯示卡的資訊
    result = adapter->GetDesc(&adapterDesc);
    if (FAILED(result)) {
        return false;
    }

    // 取得顯示卡的記憶體(MB)
    m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

    // 儲存顯示卡名稱
    error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
    if (error != 0)
    {
        return false;
    }

    // 釋放記憶體
    delete[] displayModeList;
    displayModeList = NULL;

    adapterOutput->Release();
    adapterOutput = NULL;

    adapter->Release();
    adapter = NULL;

    factory->Release();
    factory = NULL;

    // 初始化 swap chain description
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    // 設定一個 Back Buffer 
    swapChainDesc.BufferCount = 1;

    // 設定 Back Buffer 畫面大小
    swapChainDesc.BufferDesc.Width = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;

    // 設定 Back Buffer 的畫面格式
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // 設定 Back Buffer 更新頻率(refresh rate)
    if (m_vsync_enabled) {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
    }
    else {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }

    // 設定 Back Buffure 的用途為繪圖目標
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // 設定要繪製到哪一個視窗
    swapChainDesc.OutputWindow = hwnd;

    // 關閉多重取樣(multisampling off)
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    // 設定是否全螢幕
    if (fullscreen) {
        swapChainDesc.Windowed = false;
    }
    else {
        swapChainDesc.Windowed = true;
    }

    // 設定掃描順序(scan line oredering)
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

    // 設定縮放
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Back Buffer 顯示後，內容就丟棄
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // 不設定進階功能
    swapChainDesc.Flags = 0;

    // 設定使用 DirectX 11 的功能
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    /********************************************************************
     * 建立 swap chain, Direct3D device, Direct3D device context
     */
    result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, numFeatureLevels,
        D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
    if (FAILED(result)) {
        return false;
    }

    // 取得 Back Buffer 指標
    result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
    if (FAILED(result)) {
        return false;
    }

    // 使用 Back Buffer 建立 Render Target View
    result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
    if (FAILED(result)) {
        return false;
    }

    // 釋放 Back Buffer 指標
    backBufferPtr->Release();
    backBufferPtr = NULL;

    // 初始化 depth buffer description
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

    // 設定 depth buffer description
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

    // 建立 depth buffer
    result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
    if (FAILED(result)) {
        return false;
    }

    // 初始化 stencil state description
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    // 設定 stencil state description
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

    // 建立 depth stencil state
    result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
    if (FAILED(result)) {
        return false;
    }

    // 啟動 depth stencil state
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

    // 初始化 depth stencil view description
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // 設定 depth stencil view description
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // 建立 depth stencil view
    result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
    if (FAILED(result)) {
        return false;
    }

    // 綁定 render target view 及 depth stencil buffer 到 render pipeline
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

    // 設定 raster description，指示DirectX 如何繪製 polygons
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

    // 建立 rasterizer state
    result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
    if (FAILED(result))
    {
        return false;
    }

    // 設定 rasterizer state
    m_deviceContext->RSSetState(m_rasterState);

    // 設定 viewport
    viewport.Width = (float)screenWidth;
    viewport.Height = (float)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    // 建立 viewport
    m_deviceContext->RSSetViewports(1, &viewport);

    // 設定投影矩陣(projection matrix)
    fieldOfView = 3.141592654f / 4.0f; // 45度
    screenAspect = (float)screenWidth / (float)screenHeight;

    // 建立投影矩陣(projection matrix)
    m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

    // 初始化 world matrix 為單位矩陣
    m_worldMatrix = XMMatrixIdentity();

    // 建立 orthographic projection matrix，用來繪製2D畫面，例如文字
    m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);

    // Clear the second depth stencil state before setting the parameters.
    ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

    // 建立第二個 depth stencil state 的description，給2D繪圖時不使用 Z buffer 功能。所有設定都跟前一個 depth stencil state 一樣，除了 DepthEnable 設為 false。
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

    // 建立 depth stencil state
    result = m_device->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void GraveD3D::Shutdown()
{
    // 結束之前必須先切回視窗模式，否則在釋放 swap chain 時會產生 exception
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

    // 設定背景顏色
    color[0] = red;
    color[1] = green;
    color[2] = blue;
    color[3] = alpha;

    // 清除 back buffer
    m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

    // 清除 depth buffer
    m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    return;
}

void GraveD3D::EndScene()
{
    // 顯示(Present) back buffer 到螢幕上
    if (m_vsync_enabled)
    {
        // 鎖定更新頻率
        m_swapChain->Present(1, 0);
    }
    else
    {
        // 不鎖定更新頻率，能跑多快就跑多快
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
