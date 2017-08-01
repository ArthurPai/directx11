#include "GraveTextureShader.h"

GraveTextureShader::GraveTextureShader()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;
    m_sampleState = 0;
}


GraveTextureShader::~GraveTextureShader()
{
}

bool GraveTextureShader::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;

    // 初始化 vertex 及 pixel shaders
    result = InitializeShader(device, hwnd, L"./Data/shaders/texture.vs", L"./Data/shaders/texture.ps");
    if (!result)
    {
        return false;
    }

    return true;
}

void GraveTextureShader::Shutdown()
{
    // 釋放 vertex, pixel shaders 及相關物件
    ShutdownShader();
}

bool GraveTextureShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, 
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture)
{
    bool result;

    // 設定 shader 參數
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
    if (!result)
    {
        return false;
    }

    // 使用 shader 繪製已準備好的 buffers
    RenderShader(deviceContext, indexCount);

    return true;
}

bool GraveTextureShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;

    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    // 編譯 vertex shader 程式碼
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "TextureVertexShader", "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &vertexShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        // 抓取錯誤訊息
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        // 如果沒有錯誤訊息，就顯示找不到 shader 檔案
        else
        {
            MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // 編譯 pixel shader 程式碼
    result = D3DCompileFromFile(psFilename, NULL, NULL, "TexturePixelShader", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &pixelShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        // 抓取錯誤訊息
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        // 如果沒有錯誤訊息，就顯示找不到 shader 檔案
        else
        {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

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

    polygonLayout[1].SemanticName = "TEXCOORD";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    // 計算 Layout 裡的元素數量
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // 建立 vertex input layout.
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
        vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result))
    {
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
    if (FAILED(result))
    {
        return false;
    }

    // 設定貼圖取樣的描述 (texture sampler state description)
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // 建立貼圖取樣方式(texture sampler state)
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void GraveTextureShader::ShutdownShader()
{
    if (m_sampleState)
    {
        m_sampleState->Release();
        m_sampleState = 0;
    }

    if (m_matrixBuffer)
    {
        m_matrixBuffer->Release();
        m_matrixBuffer = 0;
    }

    if (m_layout)
    {
        m_layout->Release();
        m_layout = 0;
    }

    if (m_pixelShader)
    {
        m_pixelShader->Release();
        m_pixelShader = 0;
    }

    if (m_vertexShader)
    {
        m_vertexShader->Release();
        m_vertexShader = 0;
    }
}

void GraveTextureShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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
}

bool GraveTextureShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, 
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;

    // convert matrix from row-major to column-major
    XMMATRIX _worldMatrix = XMMatrixTranspose(worldMatrix);
    XMMATRIX _viewMatrix = XMMatrixTranspose(viewMatrix);
    XMMATRIX _projectionMatrix = XMMatrixTranspose(projectionMatrix);

    // 鎖住 constant buffer，準備更新資料
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
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

    // 設定 shader 的貼圖來源
    deviceContext->PSSetShaderResources(0, 1, &texture);

    return true;
}

void GraveTextureShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // 設定 vertex input layout
    deviceContext->IASetInputLayout(m_layout);

    // 設定要使用的 vertex, pixel shaders 及 取樣方式
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    // 繪製模型
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}
