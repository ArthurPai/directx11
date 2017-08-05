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

    // �k�V�q (normal) ��T 
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

    // �sĶ vertex shader �{���X
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "LightVertexShader", "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &vertexShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        // ������~�T��
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        // �p�G�S�����~�T���A�N��ܧ䤣�� shader �ɮ�
        else
        {
            MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // �sĶ pixel shader �{���X
    result = D3DCompileFromFile(psFilename, NULL, NULL, "LightPixelShader", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &pixelShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        // ������~�T��
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        // �p�G�S�����~�T���A�N��ܧ䤣�� shader �ɮ�
        else
        {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

    // �q buffer ���إ� vertex shader
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if (FAILED(result))
    {
        return false;
    }

    // �q buffer ���إ� pixel shader
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if (FAILED(result))
    {
        return false;
    }

    // �إ� vertex input layout description.
    // �����P ModelClass �� VertexType stucture �H�� shader �����@�P
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

    // �s�W�k�V�q (normal) ��T 
    polygonLayout[2].SemanticName = "NORMAL";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[2].InputSlot = 0;
    polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    // �p�� Layout �̪������ƶq
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // �إ� vertex input layout.
    result = device->CreateInputLayout(polygonLayout, numElements,
        vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(),
        &m_layout);
    if (FAILED(result))
    {
        return false;
    }

    // ���� vertex shader buffer �� pixel shader buffer
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // �]�w dynamic matrix constant buffer description
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // �إ� constant buffer pointer�A�o�ˤ~�i�H�q�{���ݦs�� vertex shader ���� constant buffer
    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // �]�w�K�Ϩ��˪��y�z (texture sampler state description)
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

    // �إ߶K�Ϩ��ˤ覡(texture sampler state)
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
    if (FAILED(result))
    {
        return false;
    }

    // �]�w camera dynamic matrix constant buffer description
    cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
    cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cameraBufferDesc.MiscFlags = 0;
    cameraBufferDesc.StructureByteStride = 0;

    // �إ� constant buffer pointer�A�o�ˤ~�i�H�q�{���ݦs�� vertex shader ���� constant buffer
    result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // �]�w light dynamic matrix constant buffer description
    // �`�N: �p�G�O�ϥ�D3D11_BIND_CONSTANT_BUFFER�AByteWidth�����O16������
    lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightBufferDesc.ByteWidth = sizeof(LightBufferType);
    lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lightBufferDesc.MiscFlags = 0;
    lightBufferDesc.StructureByteStride = 0;

    // �إ� constant buffer pointer�A�o�ˤ~�i�H�q�{���ݦs�� vertex shader ���� constant buffer
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

    // ���o���~�T���w�İϪ�����
    compileErrors = (char*)(errorMessage->GetBufferPointer());

    // ���o�T������
    bufferSize = errorMessage->GetBufferSize();

    // �}���ɮסA�Ψ��x�s���~�T��
    fout.open("shader-error.txt");

    // ��X���~�T��
    for (i = 0; i<bufferSize; i++)
    {
        fout << compileErrors[i];
    }

    // �����ɮ�
    fout.close();

    // ����
    errorMessage->Release();
    errorMessage = 0;

    // ��ܿ��~
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

    // ��� constant buffer�A�ǳƧ�s���
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // ���o constant buffer ��ƪ�����
    dataPtr = (MatrixBufferType*)mappedResource.pData;

    // ���� matrices �� constant buffer
    dataPtr->world = _worldMatrix;
    dataPtr->view = _viewMatrix;
    dataPtr->projection = _projectionMatrix;

    // �Ѷ} constant buffer
    deviceContext->Unmap(m_matrixBuffer, 0);

    // ��s vertex shader
    deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

    // ��� camera constant buffer�A�ǳƧ�s���
    result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // ���o constant buffer ��ƪ�����
    dataPtr3 = (CameraBufferType*)mappedResource.pData;

    // ���� camera position �� constant buffer
    dataPtr3->cameraPosition = cameraPosition;
    dataPtr3->padding = 0.0f;

    // ��� camera constant buffer�A�ǳƧ�s���
    deviceContext->Unmap(m_cameraBuffer, 0);

    // ��s vertex shader ���ĤG�� constant buffer
    deviceContext->VSSetConstantBuffers(1, 1, &m_cameraBuffer);

    // �]�w shader ���K�Ϩӷ�
    deviceContext->PSSetShaderResources(0, 1, &texture);

    // Lock the light constant buffer so it can be written to.
    result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // ���o constant buffer ��ƪ�����
    dataPtr2 = (LightBufferType*)mappedResource.pData;

    // �����O���Ѽƨ� constant buffer
    dataPtr2->ambientColor = ambientColor;
    dataPtr2->diffuseColor = diffuseColor;
    dataPtr2->lightDirection = lightDirection;
    dataPtr2->specularColor = specularColor;
    dataPtr2->specularPower = specularPower;

    // �Ѷ} constant buffer
    deviceContext->Unmap(m_lightBuffer, 0);

    // ��s pixel shader �� light constant buffer
    deviceContext->PSSetConstantBuffers(0, 1, &m_lightBuffer);

    return true;
}


void GraveLightShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // �]�w vertex input layout
    deviceContext->IASetInputLayout(m_layout);

    // �]�w�n�ϥΪ� vertex, pixel shaders �� ���ˤ覡
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    // ø�s�ҫ�
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}