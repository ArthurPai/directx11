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

    // ��l�� vertex �� pixel shaders
    result = InitializeShader(device, hwnd, L"./Data/shaders/texture.vs", L"./Data/shaders/texture.ps");
    if (!result)
    {
        return false;
    }

    return true;
}

void GraveTextureShader::Shutdown()
{
    // ���� vertex, pixel shaders �ά�������
    ShutdownShader();
}

bool GraveTextureShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, 
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture)
{
    bool result;

    // �]�w shader �Ѽ�
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
    if (!result)
    {
        return false;
    }

    // �ϥ� shader ø�s�w�ǳƦn�� buffers
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

    // �sĶ vertex shader �{���X
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "TextureVertexShader", "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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
    result = D3DCompileFromFile(psFilename, NULL, NULL, "TexturePixelShader", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
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

    // �p�� Layout �̪������ƶq
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // �إ� vertex input layout.
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
        vertexShaderBuffer->GetBufferSize(), &m_layout);
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

    // �إ� constant buffer pointer�A�o�ˤ~�i�H�q�{���ݨ��o vertex shader ���� constant buffer
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

    // �]�w shader ���K�Ϩӷ�
    deviceContext->PSSetShaderResources(0, 1, &texture);

    return true;
}

void GraveTextureShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
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
