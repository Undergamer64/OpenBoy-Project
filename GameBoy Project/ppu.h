#pragma once

#include <SFML/Graphics.hpp>

#include "cpu.h"

class PPU
{
    sf::RenderWindow m_window;
    sf::Font m_font;
    sf::Text* m_debugRom;

    bool IsWindowOpen();
public:

    PPU();

    bool RenderScreen();

    void RenderDebug(CPU cpu);
    void Display();
    void Clear();
};
