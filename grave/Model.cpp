#include "Model.h"

Model::Model()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
    m_Texture = 0;
    m_model = 0;
};

Model::~Model()
{
}

bool Model::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* modelFilename, char* textureFilename)
{
    bool result;

    // ���J�ҫ�
    result = LoadModel(modelFilename);
    if (!result)
    {
        return false;
    }

    // ��l�� vertex �� index buffer
    result = InitializeBuffers(device);
    if (!result) {
        return false;
    }

    // Load the texture for this model.
    result = LoadTexture(device, deviceContext, textureFilename);
    if (!result)
    {
        return false;
    }

    return true;
}

void Model::Shutdown()
{
    // ����K�ϸ귽
    ReleaseTexture();

    // ���� vertex �� index buffer
    ShutdownBuffers();

    // ����ҫ����
    ReleaseModel();

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

ID3D11ShaderResourceView* Model::GetTexture()
{
    return m_Texture->GetTexture();
}

bool Model::InitializeBuffers(ID3D11Device* device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;
    int i;

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

    // �q�ҫ���Ƹ��J vertex �}�C�� index �}�C
    for (i = 0; i<m_vertexCount; i++)
    {
        vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
        vertices[i].texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
        vertices[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

        indices[i] = i;
    }

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

bool Model::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
    bool result;

    // Create the texture object.
    m_Texture = new GraveTexture;
    if (!m_Texture)
    {
        return false;
    }

    // Initialize the texture object.
    result = m_Texture->Initialize(device, deviceContext, filename);
    if (!result)
    {
        return false;
    }

    return true;
}

void Model::ReleaseTexture()
{
    // Release the texture object.
    if (m_Texture)
    {
        m_Texture->Shutdown();
        delete m_Texture;
        m_Texture = 0;
    }

    return;
}

bool Model::LoadModel(char* filename)
{
    ifstream fin;
    char input;
    int i;

    // �}�Ҽҫ��ɮ�
    fin.open(filename);

    if (fin.fail())
    {
        return false;
    }

    // Ū���r������:�X�{
    fin.get(input);
    while (input != ':')
    {
        fin.get(input);
    }

    // Ū�����I�ƶq
    fin >> m_vertexCount;

    // �]�w index �ƶq
    m_indexCount = m_vertexCount;

    // �إ� model vertics ���
    m_model = new VextexData[m_vertexCount];
    if (!m_model)
    {
        return false;
    }

    // ���vertex��ƪ��_�I
    fin.get(input);
    while (input != ':')
    {
        fin.get(input);
    }
    fin.get(input);
    fin.get(input);

    // Ū�� vertex ���
    for (i = 0; i<m_vertexCount; i++)
    {
        fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
        fin >> m_model[i].tu >> m_model[i].tv;
        fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
    }

    // ����
    fin.close();

    return true;
}

void Model::ReleaseModel()
{
    if (m_model)
    {
        delete[] m_model;
        m_model = 0;
    }

    return;
}
