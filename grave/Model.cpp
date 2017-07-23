#include "Model.h"

Model::Model()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
};

Model::~Model()
{
}

bool Model::Initialize(ID3D11Device* device)
{
    bool result;

    // ��l�� vertex �� index buffer
    result = InitializeBuffers(device);
    if (!result) {
        return false;
    }

    return true;
}

void Model::Shutdown()
{
    // ���� vertex �� index buffer
    ShutdownBuffers();

    return;
}

void Model::Render(ID3D11DeviceContext* deviceContext)
{
    // �N vertex �� index buffer ��J graphics pipeline �ǳ�ø�s
    RenderBuffers(deviceContext);

    return;
}

int Model::GetIndexCount()
{
    return m_indexCount;
}

bool Model::InitializeBuffers(ID3D11Device* device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;

    // �]�w���I�ƶq
    m_vertexCount = 4;

    // �]�w indices �ƶq
    m_indexCount = 6;

    // �إ� vertex array
    vertices = new VertexType[m_vertexCount];
    if (!vertices) {
        return false;
    }

    // �إ� index array
    indices = new unsigned long[m_indexCount];
    if (!indices) {
        return false;
    }

    // �]�w���I
    vertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // ���U
    vertices[0].color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

    vertices[1].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // �k�U
    vertices[1].color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

    vertices[2].position = XMFLOAT3(1.0f, 1.0f, 0.0f);  // �k�W
    vertices[2].color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

    vertices[3].position = XMFLOAT3(-1.0f, 1.0f, 0.0f);  // ���W
    vertices[3].color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

    // �]�w indices�A���Ӷ�����
    indices[0] = 0;  // ���U
    indices[1] = 2;  // �k�U
    indices[2] = 1;  // �k�W
    indices[3] = 0;  // ���U
    indices[4] = 3;  // �k�W
    indices[5] = 2;  // ���W

    // �]�w static vertex buffer description
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // �]�w���I��ƨӷ�
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // �إ� vertex buffer
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result)) {
        return false;
    }

    // �]�w static index buffer description
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // �]�w index ��ƨӷ�
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // �إ� index buffer
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result)) {
        return false;
    }

    // ����O����
    delete[] vertices;
    vertices = 0;

    delete[] indices;
    indices = 0;

    return true;
}

void Model::ShutdownBuffers()
{
    // ���� index buffer
    if (m_indexBuffer) {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }

    // ���� vertex buffer
    if (m_vertexBuffer)  {
        m_vertexBuffer->Release();
        m_vertexBuffer = 0;
    }

    return;
}

void Model::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride;
    unsigned int offset;

    // �]�w vertex buffer stride �� offset
    stride = sizeof(VertexType);
    offset = 0;

    // ���J vertex buffer �� input assembler
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // ���J index buffer �� input assembler
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // �]�w primitive type
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}
