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

    // �]�w�ҥH�����䬰���񪬺A
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
