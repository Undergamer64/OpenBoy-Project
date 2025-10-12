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
    : m_window(sf::VideoMode( sf::Vector2u(160*8,144*8), 600), "GameBoy Project")
{
    m_window.setFramerateLimit(60);
    m_window.setVerticalSyncEnabled(true);
    sf::Vector2u size = sf::VideoMode::getDesktopMode().size;
    size.x = std::min(size.x / 160, size.y / 144) * 160;
    size.y = std::min(size.x / 160, size.y / 144) * 144;
    m_window.setSize(size);
    m_window.setPosition(sf::Vector2i(
        (sf::VideoMode::getDesktopMode().size.x - size.x) / 2,
        (sf::VideoMode::getDesktopMode().size.y - size.y) / 2)
        );
    if (!m_font.openFromFile("EarlyGameBoy.ttf"))
    {
        m_window.close();
    }
    
    m_debugRom = new sf::Text(m_font);
}

bool PPU::RenderScreen()
{
    if (!IsWindowOpen()) return false;

    //Render the game here


    
    return true;
}

void PPU::RenderDebug(CPU cpu)
{
    m_debugRom->setCharacterSize(16);

    std::stringstream ur;
    ur << cpu.DumpBoot(true).str();

    ur << "\nPC : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.PC) << "\n";
    ur << "SP : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.SP) << "\n";
    ur << "A" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[6]) << "\n";
    ur << "F" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[7]) << "\n";
    ur << "B" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[0]) << "\n";
    ur << "C" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[1]) << "\n";
    ur << "D" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[2]) << "\n";
    ur << "E" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[3]) << "\n";
    ur << "H" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[4]) << "\n";
    ur << "L" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[5]) << "\n";
    ur << "IME : " << (cpu.m_registers.IME ? "true" : "false") << "\n";
    ur << "Current instruction : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.GetCurrentInstruction());
    
    m_debugRom->setString(ur.str());
    
    m_window.draw(*m_debugRom);
}

void PPU::Display()
{
    m_window.display();
}

void PPU::Clear()
{
    m_window.clear();
}

