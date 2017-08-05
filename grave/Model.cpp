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

    // 載入模型
    result = LoadModel(modelFilename);
    if (!result)
    {
        return false;
    }

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

    // 釋放模型資料
    ReleaseModel();

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
    int i;

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

    // 從模型資料載入 vertex 陣列及 index 陣列
    for (i = 0; i<m_vertexCount; i++)
    {
        vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
        vertices[i].texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
        vertices[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

        indices[i] = i;
    }

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

bool Model::LoadModel(char* filename)
{
    ifstream fin;
    char input;
    int i;

    // 開啟模型檔案
    fin.open(filename);

    if (fin.fail())
    {
        return false;
    }

    // 讀取字元直到:出現
    fin.get(input);
    while (input != ':')
    {
        fin.get(input);
    }

    // 讀取頂點數量
    fin >> m_vertexCount;

    // 設定 index 數量
    m_indexCount = m_vertexCount;

    // 建立 model vertics 資料
    m_model = new VextexData[m_vertexCount];
    if (!m_model)
    {
        return false;
    }

    // 找到vertex資料的起點
    fin.get(input);
    while (input != ':')
    {
        fin.get(input);
    }
    fin.get(input);
    fin.get(input);

    // 讀取 vertex 資料
    for (i = 0; i<m_vertexCount; i++)
    {
        fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
        fin >> m_model[i].tu >> m_model[i].tv;
        fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
    }

    // 關檔
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
