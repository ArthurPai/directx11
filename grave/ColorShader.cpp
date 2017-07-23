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

    // ��l�� vertex �� pixel shaders
    result = InitializeShader(device, hwnd, L"./Data/shaders/color.vs", L"./Data/shaders/color.ps");
    if (!result) {
        return false;
    }

    return true;
}

void ColorShader::Shutdown()
{
    // ���� vertex, pixel shaders �ά�������
    ShutdownShader();

    return;
}

bool ColorShader::Render(ID3D11DeviceContext* deviceContext, int indexCount,
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix)
{
    bool result;

    // �]�w shader �Ѽ�
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
    if (!result)
    {
        return false;
    }

    // �ϥ� shader ø�s�w�ǳƦn�� buffers
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

    // �sĶ vertex shader �{���X
    result = D3DCompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &vertexShaderBuffer, &errorMessage);
    if (FAILED(result)) {
        // ������~�T��
        if (errorMessage) {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        // �p�G�S�����~�T���A�N��ܧ䤣�� shader �ɮ�
        else {
            MessageBox(hwnd, vsFilename, L"Missing Vertex Shader File", MB_OK);
        }

        return false;
    }

    // �sĶ pixel shader �{���X
    result = D3DCompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        &pixelShaderBuffer, &errorMessage);
    if (FAILED(result)) {
        // ������~�T��
        if (errorMessage) {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        // �p�G�S�����~�T���A�N��ܧ䤣�� shader �ɮ�
        else {
            MessageBox(hwnd, psFilename, L"Missing Pixel Shader File", MB_OK);
        }

        return false;
    }

    SIZE_T size = vertexShaderBuffer->GetBufferSize();
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

    polygonLayout[1].SemanticName = "COLOR";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    // �p�� polygonLayout �̪������ƶq
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // �إ� vertex input layout.
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
        vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result)) {
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

    // ��� constant buffer�A�ǳƧ�s���
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result)) {
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

    return true;
}

void ColorShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // �]�w vertex input layout
    deviceContext->IASetInputLayout(m_layout);

    // �]�w�n�ϥΪ� vertex �� pixel shaders
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // ø�s�ҫ�
    deviceContext->DrawIndexed(indexCount, 0, 0);
}

