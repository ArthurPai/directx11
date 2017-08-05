#include "GraveLight.h"



GraveLight::GraveLight()
{
}


GraveLight::~GraveLight()
{
}


void GraveLight::SetAmbientColor(float red, float green, float blue, float alpha)
{
    m_ambientColor = XMFLOAT4(red, green, blue, alpha);
    return;
}


void GraveLight::SetDiffuseColor(float red, float green, float blue, float alpha)
{
    m_diffuseColor = XMFLOAT4(red, green, blue, alpha);
    return;
}


void GraveLight::SetDirection(float x, float y, float z)
{
    m_direction = XMFLOAT3(x, y, z);
    return;
}


XMFLOAT4 GraveLight::GetAmbientColor()
{
    return m_ambientColor;
}


XMFLOAT4 GraveLight::GetDiffuseColor()
{
    return m_diffuseColor;
}


XMFLOAT3 GraveLight::GetDirection()
{
    return m_direction;
}
