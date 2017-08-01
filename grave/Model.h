#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include "GraveTexture.h"

using namespace DirectX;

class Model
{
private:
    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
    };

public:
    Model();
    ~Model();

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* textureFilename);
    void Shutdown();
    void Render(ID3D11DeviceContext* deviceContext);

    int GetIndexCount();

    ID3D11ShaderResourceView* GetTexture();

private:
    bool InitializeBuffers(ID3D11Device* device);
    void ShutdownBuffers();
    void RenderBuffers(ID3D11DeviceContext* deviceContext);

    bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, char*);
    void ReleaseTexture();
private:
    ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
    int m_vertexCount, m_indexCount;
    GraveTexture* m_Texture;
};

