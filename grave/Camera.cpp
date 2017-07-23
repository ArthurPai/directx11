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

    // 設定攝影機頭頂方向
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;

    // 轉換成 XMVECTOR 結構
    upVector = XMLoadFloat3(&up);

    // 設定攝影機的世界座標
    position.x = m_positionX;
    position.y = m_positionY;
    position.z = m_positionZ;

    // 轉換成 XMVECTOR 結構
    positionVector = XMLoadFloat3(&position);

    // 設定攝影機預設的注視方向
    lookAt.x = 0.0f;
    lookAt.y = 0.0f;
    lookAt.z = 1.0f;

    // 轉換成 XMVECTOR 結構
    lookAtVector = XMLoadFloat3(&lookAt);

    // 將攝影機的 yaw (Y axis), pitch (X axis), and roll (Z axis) 轉換為弧度(radians)
    pitch = m_rotationX * 0.0174532925f;
    yaw = m_rotationY * 0.0174532925f;
    roll = m_rotationZ * 0.0174532925f;

    // 建立旋轉矩陣
    rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    // 使用旋轉矩陣計算出 lookAt 及 up vector
    lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
    upVector = XMVector3TransformCoord(upVector, rotationMatrix);

    // 算出注視的位置
    lookAtVector = XMVectorAdd(positionVector, lookAtVector);

    // 使用上面三個資料計算 view Matrix
    m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}

void Camera::GetViewMatrix(XMMATRIX& viewMatrix)
{
    viewMatrix = m_viewMatrix;
    return;
}
