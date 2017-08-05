#pragma once
#include <directxmath.h>

using namespace DirectX;

class GraveLight
{
public:
    GraveLight();
    ~GraveLight();

    void SetDiffuseColor(float, float, float, float);
    void SetDirection(float, float, float);

    XMFLOAT4 GetDiffuseColor();
    XMFLOAT3 GetDirection();

private:
    XMFLOAT4 m_diffuseColor;
    XMFLOAT3 m_direction;
};

