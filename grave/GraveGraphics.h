#pragma once
#include "GraveD3D.h"
#include "Camera.h"
#include "Model.h"
#include "GraveTextureShader.h"
#include "GraveLightShader.h"
#include "GraveLight.h"
#include "GraveBitmap.h"

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
    bool Render(float rotation, float move);

private:
    GraveD3D* m_Direct3D;
    Camera* m_Camera;
    Model* m_Model;

    // 新增燈光及燈光的Shader
    GraveLight* m_Light;
    GraveTextureShader* m_TextureShader;
    GraveLightShader* m_LightShader;

    // 新增2D 物件
    GraveBitmap* m_Bitmap;
};

