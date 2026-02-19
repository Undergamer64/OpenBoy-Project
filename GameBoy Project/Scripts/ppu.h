#pragma once
#include <SFML/Graphics.hpp>

class MMU;
class CPU;

static constexpr int GB_W = 160;
static constexpr int GB_H = 144;

class PPU
{
    sf::RenderWindow m_window;
    sf::Font m_font;
    sf::Text* m_debugRom;
    sf::RectangleShape* m_debugBackground;

    sf::RenderWindow m_debugWindow;
    sf::Text* m_debugText;
    
    //std::vector<std::vector<sf::RectangleShape*>> m_screenData;

    std::array<uint8_t, GB_W * GB_H * 4> m_framebuffer;
    sf::Texture m_screenTexture;
    sf::Sprite m_screenSprite;

    uint8_t m_currentPixel;
    int m_graphicPenalty;

    MMU& m_mmu;

    bool IsWindowOpen();

    void ScrollScreen(float delta);
public:

    PPU(MMU& mmu);
    void RecalculateDebugScreenSize();
    void RecalculateScreenSize();

    bool RenderScreen();

    void DrawCurrentPixel(uint8_t currentLine, int currentDot);

    void RenderDebug(CPU cpu, bool isRunning);
    sf::Color GetPixelColor(int y, int x);
    void SetPixelColor(int y, int x, sf::Color color);
    void Display();
    void Clear();

    void RenderTiles(uint8_t lcdControl);
    void RenderSprites(uint8_t lcdControl);
};
