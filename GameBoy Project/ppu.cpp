#include "ppu.h"

#include <optional>

bool PPU::IsWindowOpen()
{
    do
    {
        std::optional<sf::Event> event = m_window.pollEvent();
        if (!event.has_value())
        {
            break;
        }
        if (event.value().is<sf::Event::Closed>())
        {
            m_window.close();
            return false;
        }
    } while (true);

    return true;
}

PPU::PPU()
    : m_window(sf::VideoMode( sf::Vector2u(160,144), 600), "SFML in Rider!")
{
    m_window.setFramerateLimit(60);
    m_window.setVerticalSyncEnabled(true);
}

bool PPU::RenderScreen()
{
    if (!IsWindowOpen()) return false;
    
    m_window.clear();
    
    return true;
}
