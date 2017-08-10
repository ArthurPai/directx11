#pragma once
#include <memory>
#include "GraveD3D.h"
#include "Camera.h"
#include "Model.h"
#include "GraveTextureShader.h"
#include "GraveLightShader.h"
#include "GraveLight.h"
#include "GraveBitmap.h"
#include "SpriteFont.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

using namespace std;

class GraveGraphics
{
public:
    GraveGraphics();
    ~GraveGraphics();

    bool Initialize(int, int, HWND);
    void Shutdown();
    bool Frame(int mouseX, int mouseY, int fps, int cpuPercentage, float frameTime);

	void MoveCamera(int x, int y, int z);
private:
    // 增加旋轉的參數
    bool Render(int mouseX, int mouseY, float rotation, float move, int fps, int cpuPercentage, float frameTime);

private:
    float m_screenWidth;
    float m_screenHeight;

    GraveD3D* m_Direct3D;
    Camera* m_Camera;
    Model* m_Model;

    // 新增燈光及燈光的Shader
    GraveLight* m_Light;
    GraveTextureShader* m_TextureShader;
    GraveLightShader* m_LightShader;

    // 新增2D 物件
    GraveBitmap* m_Bitmap;

    // 新增
    std::unique_ptr<DirectX::SpriteFont> m_font;
    DirectX::SimpleMath::Vector2 m_fontPos;
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
};
