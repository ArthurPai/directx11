#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
//#include <d3dx11async.h>
#include <fstream>

using namespace DirectX;
using namespace std;

class GraveLightShader
{
private:
    struct MatrixBufferType
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };

    struct CameraBufferType
    {
        XMFLOAT3 cameraPosition;
        float padding;
    };

    struct LightBufferType
    {
        XMFLOAT4 ambientColor;
        XMFLOAT4 diffuseColor;
        XMFLOAT3 lightDirection;
        float specularPower;
        XMFLOAT4 specularColor;
    };

public:
    GraveLightShader();
    ~GraveLightShader();

    bool Initialize(ID3D11Device* device, HWND hwnd);
    void Shutdown();
    bool Render(ID3D11DeviceContext* deviceContext, int indexCount,
        const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix,
        ID3D11ShaderResourceView* texture, XMFLOAT3 lightDirection, XMFLOAT4 lightAmbientColor, XMFLOAT4 lightDiffuseColor,
        XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower);

private:
    bool InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename);

    bool SetShaderParameters(ID3D11DeviceContext* deviceContext,
        const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix,
        ID3D11ShaderResourceView* texture, XMFLOAT3 lightDirection, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor,
        XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower);
    void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11SamplerState* m_sampleState;
    ID3D11Buffer* m_matrixBuffer;

    ID3D11Buffer* m_cameraBuffer;
    ID3D11Buffer* m_lightBuffer;
};

