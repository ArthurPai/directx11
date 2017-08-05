#include "GraveLightShader.h"

GraveLightShader::GraveLightShader()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_sampleState = 0;
    m_matrixBuffer = 0;
    m_cameraBuffer = 0;
    m_lightBuffer = 0;
}

GraveLightShader::~GraveLightShader()
{

}


bool GraveLightShader::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;

    // Initialize the vertex and pixel shaders.
    result = InitializeShader(device, hwnd, L"./Data/shaders/light.vs", L"./Data/shaders/light.ps");
    if (!result)
    {
        return false;
    }

    return true;
}

void GraveLightShader::Shutdown()
{
    // Shutdown the vertex and pixel shaders as well as the related objects.
    ShutdownShader();

    return;
}

bool GraveLightShader::Render(ID3D11DeviceContext* deviceContext, int indexCount,
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix,
    ID3D11ShaderResourceView* texture, XMFLOAT3 lightDirection, XMFLOAT4 lightAmbientColor, XMFLOAT4 lightDiffuseColor,
    XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower)
{
    bool result;

    // Set the shader parameters that it will use for rendering.
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix,
        texture, lightDirection, lightAmbientColor, lightDiffuseColor,
        cameraPosition, specularColor, specularPower);
    if (!result)
    {
        return false;
    }

    // Now render the prepared buffers with the shader.
    RenderShader(deviceContext, indexCount);

    return true;
}

bool GraveLightShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;

    // 法向量 (normal) 資訊 
    D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
    unsigned int numElements;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_BUFFER_DESC cameraBufferDesc;
    D3D11_BUFFER_DESC lightBufferDesc;

    // Initialize the pointers this function will use to null.
    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    // 編譯 vertex shader 程式碼
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "LightVertexShader", "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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
    result = D3DCompileFromFile(psFilename, NULL, NULL, "LightPixelShader", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

    // 新增法向量 (normal) 資訊 
    polygonLayout[2].SemanticName = "NORMAL";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[2].InputSlot = 0;
    polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    // 計算 Layout 裡的元素數量
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // 建立 vertex input layout.
    result = device->CreateInputLayout(polygonLayout, numElements,
        vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
        &m_layout);
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

    // 建立 constant buffer pointer，這樣才可以從程式端存取 vertex shader 中的 constant buffer
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

    // 設定 camera dynamic matrix constant buffer description
    cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
    cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cameraBufferDesc.MiscFlags = 0;
    cameraBufferDesc.StructureByteStride = 0;

    // 建立 constant buffer pointer，這樣才可以從程式端存取 vertex shader 中的 constant buffer
    result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // 設定 light dynamic matrix constant buffer description
    // 注意: 如果是使用D3D11_BIND_CONSTANT_BUFFER，ByteWidth必須是16的倍數
    lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightBufferDesc.ByteWidth = sizeof(LightBufferType);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lightBufferDesc.MiscFlags = 0;
    lightBufferDesc.StructureByteStride = 0;

    // 建立 constant buffer pointer，這樣才可以從程式端存取 vertex shader 中的 constant buffer
    result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void GraveLightShader::ShutdownShader()
{
    if (m_cameraBuffer)
    {
        m_cameraBuffer->Release();
        m_cameraBuffer = 0;
    }

    if (m_lightBuffer)
    {
        m_lightBuffer->Release();
        m_lightBuffer = 0;
    }

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


void GraveLightShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
    char* compileErrors;
    unsigned long bufferSize, i;
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

bool GraveLightShader::SetShaderParameters(ID3D11DeviceContext* deviceContext,
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix,
    ID3D11ShaderResourceView* texture, XMFLOAT3 lightDirection, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor,
    XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    unsigned int bufferNumber;
    MatrixBufferType* dataPtr;
    LightBufferType* dataPtr2;
    CameraBufferType* dataPtr3;

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

    // 鎖住 camera constant buffer，準備更新資料
    result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 取得 constant buffer 資料的指標
    dataPtr3 = (CameraBufferType*)mappedResource.pData;

    // 拷貝 camera position 到 constant buffer
    dataPtr3->cameraPosition = cameraPosition;
    dataPtr3->padding = 0.0f;

    // 鎖住 camera constant buffer，準備更新資料
    deviceContext->Unmap(m_cameraBuffer, 0);

    // 更新 vertex shader 的第二個 constant buffer
    deviceContext->VSSetConstantBuffers(1, 1, &m_cameraBuffer);

    // 設定 shader 的貼圖來源
    deviceContext->PSSetShaderResources(0, 1, &texture);

    // Lock the light constant buffer so it can be written to.
    result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // 取得 constant buffer 資料的指標
    dataPtr2 = (LightBufferType*)mappedResource.pData;

    // 拷貝燈光參數到 constant buffer
    dataPtr2->ambientColor = ambientColor;
    dataPtr2->diffuseColor = diffuseColor;
    dataPtr2->lightDirection = lightDirection;
    dataPtr2->specularColor = specularColor;
    dataPtr2->specularPower = specularPower;

    // 解開 constant buffer
    deviceContext->Unmap(m_lightBuffer, 0);

    // 更新 pixel shader 的 light constant buffer
    deviceContext->PSSetConstantBuffers(0, 1, &m_lightBuffer);

    return true;
}


void GraveLightShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
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