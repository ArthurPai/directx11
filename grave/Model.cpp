#include "Model.h"

Model::Model()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
    m_Texture = 0;
};

Model::~Model()
{
}

bool Model::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* textureFilename)
{
    bool result;

    // 初始化 vertex 及 index buffer
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
    // 釋放貼圖資源
    ReleaseTexture();

    // 釋放 vertex 及 index buffer
    ShutdownBuffers();

    return;
}

void Model::Render(ID3D11DeviceContext* deviceContext)
{
    // 將 vertex 及 index buffer 放入 graphics pipeline 準備繪製
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

    // 設定頂點數量
    m_vertexCount = 3;

    // 設定 indices 數量
    m_indexCount = 3;

    // 建立 vertex array
    vertices = new VertexType[m_vertexCount];
    if (!vertices) {
        return false;
    }

    // 建立 index array
    indices = new unsigned long[m_indexCount];
    if (!indices) {
        return false;
    }

    // 設定頂點
    vertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // 左下
    vertices[0].texture = XMFLOAT2(0.0f, 1.0f);

    vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // 上方
    vertices[1].texture = XMFLOAT2(0.5f, 0.0f);

    vertices[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // 右下
    vertices[2].texture = XMFLOAT2(1.0f, 1.0f);

    // 設定 indices，按照順時鐘
    indices[0] = 0;  // 左下
    indices[1] = 1;  // 上方
    indices[2] = 2;  // 右下

    // 設定 static vertex buffer description
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // 設定頂點資料來源
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // 建立 vertex buffer
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result)) {
        return false;
    }

    // 設定 static index buffer description
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // 設定 index 資料來源
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // 建立 index buffer
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result)) {
        return false;
    }

    // 釋放記憶體
    delete[] vertices;
    vertices = 0;

    delete[] indices;
    indices = 0;

    return true;
}

void Model::ShutdownBuffers()
{
    // 釋放 index buffer
    if (m_indexBuffer) {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }

    // 釋放 vertex buffer
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

    // 設定 vertex buffer stride 及 offset
    stride = sizeof(VertexType);
    offset = 0;

    // 載入 vertex buffer 到 input assembler
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // 載入 index buffer 到 input assembler
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // 設定 primitive type
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
