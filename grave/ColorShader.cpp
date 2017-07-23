#include "ColorShader.h"

ColorShader::ColorShader()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;
}

ColorShader::~ColorShader()
{
}

bool ColorShader::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;

    // 初始化 vertex 及 pixel shaders
    result = InitializeShader(device, hwnd, L"./Data/shaders/color.vs", L"./Data/shaders/color.ps");
    if (!result) {
        return false;
    }

    return true;
}

void ColorShader::Shutdown()
{
    // 釋放 vertex, pixel shaders 及相關物件
    ShutdownShader();

    return;
}

bool ColorShader::Render(ID3D11DeviceContext* deviceContext, int indexCount,
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix)
{
    bool result;

    // 設定 shader 參數
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
    if (!result)
    {
        return false;
    }

    // 使用 shader 繪製已準備好的 buffers
    RenderShader(deviceContext, indexCount);

    return true;
}

bool ColorShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage = NULL;
    ID3D10Blob* vertexShaderBuffer = NULL;
    ID3D10Blob* pixelShaderBuffer = NULL;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;

    // 編譯 vertex shader 程式碼
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &vertexShaderBuffer, &errorMessage);
    if (FAILED(result)) {
        // 抓取錯誤訊息
        if (errorMessage) {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        // 如果沒有錯誤訊息，就顯示找不到 shader 檔案
        else {
            MessageBox(hwnd, vsFilename, L"Missing Vertex Shader File", MB_OK);
        }

        return false;
    }

    // 編譯 pixel shader 程式碼
    result = D3DCompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &pixelShaderBuffer, &errorMessage);
    if (FAILED(result)) {
        // 抓取錯誤訊息
        if (errorMessage) {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        // 如果沒有錯誤訊息，就顯示找不到 shader 檔案
        else {
            MessageBox(hwnd, psFilename, L"Missing Pixel Shader File", MB_OK);
        }

        return false;
    }

    SIZE_T size = vertexShaderBuffer->GetBufferSize();
    // 從 buffer 中建立 vertex shader
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if (FAILED(result))
    {
        return false;
    }

    // 從 buffer 中建立 pixel shader
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if (FAILED(result))
    {
        return false;
    }

    // 建立 vertex input layout description.
    // 必須與 ModelClass 的 VertexType stucture 以及 shader 中的一致
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "COLOR";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    // 計算 polygonLayout 裡的元素數量
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // 建立 vertex input layout.
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
        vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result)) {
        return false;
    }

    // 釋放 vertex shader buffer 及 pixel shader buffer
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // 設定 dynamic matrix constant buffer description
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // 建立 constant buffer pointer，這樣才可以從程式端取得 vertex shader 中的 constant buffer
    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result)) {
        return false;
    }

    return true;
}

void ColorShader::ShutdownShader()
{
    if (m_matrixBuffer) {
        m_matrixBuffer->Release();
        m_matrixBuffer = 0;
    }

    if (m_layout) {
        m_layout->Release();
        m_layout = 0;
    }

    if (m_pixelShader) {
        m_pixelShader->Release();
        m_pixelShader = 0;
    }

    if (m_vertexShader) {
        m_vertexShader->Release();
        m_vertexShader = 0;
    }
}

void ColorShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
    char* compileErrors;
    unsigned long long bufferSize, i;
    ofstream fout;

    // 取得錯誤訊息緩衝區的指標
    compileErrors = (char*)(errorMessage->GetBufferPointer());

    // 取得訊息長度
    bufferSize = errorMessage->GetBufferSize();

    // 開啟檔案，用來儲存錯誤訊息
    fout.open("shader-error.txt");

    // 輸出錯誤訊息
    for (i = 0; i<bufferSize; i++)
    {
        fout << compileErrors[i];
    }

    // 關閉檔案
    fout.close();

    // 釋放
    errorMessage->Release();
    errorMessage = 0;

    // 顯示錯誤
    MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

    return;
}

bool ColorShader::SetShaderParameters(ID3D11DeviceContext* deviceContext,
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;

    // Transpose the matrices to prepare them for the shader.
    XMMATRIX _worldMatrix = XMMatrixTranspose(worldMatrix);
    XMMATRIX _viewMatrix = XMMatrixTranspose(viewMatrix);
    XMMATRIX _projectionMatrix = XMMatrixTranspose(projectionMatrix);

    // 鎖住 constant buffer，準備更新資料
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) {
        return false;
    }

    // 取得 constant buffer 資料的指標
    dataPtr = (MatrixBufferType*)mappedResource.pData;

    // 拷貝 matrices 到 constant buffer
    dataPtr->world = _worldMatrix;
    dataPtr->view = _viewMatrix;
    dataPtr->projection = _projectionMatrix;

    // 解開 constant buffer
    deviceContext->Unmap(m_matrixBuffer, 0);

    // 更新 vertex shader
    deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

    return true;
}

void ColorShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // 設定 vertex input layout
    deviceContext->IASetInputLayout(m_layout);

    // 設定要使用的 vertex 及 pixel shaders
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // 繪製模型
    deviceContext->DrawIndexed(indexCount, 0, 0);
}

