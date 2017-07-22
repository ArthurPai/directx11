#include "GraveInput.h"

GraveInput::GraveInput()
{
}


GraveInput::~GraveInput()
{
}

void GraveInput::Initialize()
{
    int i;

    // 設定所以有按鍵為釋放狀態
    for (i = 0; i < 256; i++) {
        m_keys[i] = false;
    }
}

void GraveInput::KeyDown(unsigned int input)
{
    m_keys[input] = true;
}

void GraveInput::KeyUp(unsigned int input)
{
    m_keys[input] = false;
}

bool GraveInput::IsKeyDown(unsigned int key)
{
    return m_keys[key];
}
