#include "Camera.h"

Camera::Camera()
{
    m_positionX = 0.0f;
    m_positionY = 0.0f;
    m_positionZ = 0.0f;

    m_rotationX = 0.0f;
    m_rotationY = 0.0f;
    m_rotationZ = 0.0f;
}

Camera::~Camera()
{
}

void Camera::SetPosition(float x, float y, float z)
{
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
    return;
}

void Camera::SetRotation(float x, float y, float z)
{
    m_rotationX = x;
    m_rotationY = y;
    m_rotationZ = z;
    return;
}

XMFLOAT3 Camera::GetPosition()
{
    return XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}

XMFLOAT3 Camera::GetRotation()
{
    return XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}

void Camera::Render()
{
    XMFLOAT3 up, position, lookAt;
    XMVECTOR upVector, positionVector, lookAtVector;
    float yaw, pitch, roll;
    XMMATRIX rotationMatrix;

    // �]�w��v���Y����V
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;

    // �ഫ�� XMVECTOR ���c
    upVector = XMLoadFloat3(&up);

    // �]�w��v�����@�ɮy��
    position.x = m_positionX;
    position.y = m_positionY;
    position.z = m_positionZ;

    // �ഫ�� XMVECTOR ���c
    positionVector = XMLoadFloat3(&position);

    // �]�w��v���w�]���`����V
    lookAt.x = 0.0f;
    lookAt.y = 0.0f;
    lookAt.z = 1.0f;

    // �ഫ�� XMVECTOR ���c
    lookAtVector = XMLoadFloat3(&lookAt);

    // �N��v���� yaw (Y axis), pitch (X axis), and roll (Z axis) �ഫ������(radians)
    pitch = m_rotationX * 0.0174532925f;
    yaw = m_rotationY * 0.0174532925f;
    roll = m_rotationZ * 0.0174532925f;

    // �إ߱���x�}
    rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    // �ϥα���x�}�p��X lookAt �� up vector
    lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
    upVector = XMVector3TransformCoord(upVector, rotationMatrix);

    // ��X�`������m
    lookAtVector = XMVectorAdd(positionVector, lookAtVector);

    // �ϥΤW���T�Ӹ�ƭp�� view Matrix
    m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}

void Camera::GetViewMatrix(XMMATRIX& viewMatrix)
{
    viewMatrix = m_viewMatrix;
    return;
}
