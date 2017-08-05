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


void GraveLight::SetSpecularColor(float red, float green, float blue, float alpha)
{
    m_specularColor = XMFLOAT4(red, green, blue, alpha);
    return;
}


void GraveLight::SetSpecularPower(float power)
{
    m_specularPower = power;
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


XMFLOAT4 GraveLight::GetSpecularColor()
{
    return m_specularColor;
}


float GraveLight::GetSpecularPower()
{
    return m_specularPower;
}