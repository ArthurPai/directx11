#pragma once
#include "GraveD3D.h"
#include "Camera.h"
#include "Model.h"
#include "GraveLightShader.h"
#include "GraveLight.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class GraveGraphics
{
public:
    GraveGraphics();
    ~GraveGraphics();

    bool Initialize(int, int, HWND);
    void Shutdown();
    bool Frame();

private:
    // 增加旋轉的參數
    bool Render(float rotation);

private:
    GraveD3D* m_Direct3D;
    Camera* m_Camera;
    Model* m_Model;

    // 新增燈光及燈光的Shader
    GraveLight* m_Light;
    GraveLightShader* m_LightShader;
};

