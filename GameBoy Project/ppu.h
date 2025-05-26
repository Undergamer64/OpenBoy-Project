#pragma once

#include <SFML/Graphics.hpp>

class PPU
{
    sf::RenderWindow m_window;

    bool IsWindowOpen();
public:

    PPU();

    bool RenderScreen();
};
