#pragma once

#include <SFML/Graphics.hpp>

#include "cpu.h"

class PPU
{
    sf::RenderWindow m_window;
    sf::Font m_font;
    sf::Text* m_debugRom;
    sf::RectangleShape* m_debugBackground;
    
    std::vector<std::vector<sf::RectangleShape*>> m_ScreenData;

    MMU& m_mmu;

    bool IsWindowOpen();
public:

    PPU(MMU& mmu);

    bool RenderScreen();

    void DrawScanLine(uint8_t currentLine);

    void RenderDebug(CPU cpu);
    void Display();
    void Clear();

    void RenderTiles(uint8_t lcdControl);
    void RenderSprites(uint8_t lcdControl);
};
