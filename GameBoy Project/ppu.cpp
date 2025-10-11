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
    if (!m_font.openFromFile("EarlyGameBoy.ttf"))
    {
        m_window.close();
    }
    
    m_debugRom = new sf::Text(m_font);
    m_debugInstruction = new sf::Text(m_font);
}

bool PPU::RenderScreen()
{
    if (!IsWindowOpen()) return false;

    //Render the game here


    
    return true;
}

void PPU::RenderDebug(CPU cpu)
{
    m_debugInstruction->setCharacterSize(16);
    m_debugRom->setCharacterSize(16);

    std::stringstream ur;
    ur << cpu.DumpBoot(true).str();

    ur << "\nPC : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.PC) << std::endl;
    ur << "SP : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.SP) << std::endl;
    ur << "A" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[6]) << std::endl;
    ur << "F" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[7]) << std::endl;
    ur << "B" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[0]) << std::endl;
    ur << "C" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[1]) << std::endl;
    ur << "D" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[2]) << std::endl;
    ur << "E" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[3]) << std::endl;
    ur << "H" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[4]) << std::endl;
    ur << "L" << " : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.m_registers.m_registers[5]) << std::endl;
    ur << "IME : " << (cpu.m_registers.IME ? "true" : "false") << std::endl;
    
    m_debugRom->setString(ur.str());
    
    std::stringstream ss;
    ss << "Current instruction : 0x" << std::hex << std::uppercase << static_cast<int>(cpu.GetCurrentInstruction());
    m_debugInstruction->setString(ss.str());
    
    m_window.draw(*m_debugRom);
    //m_window.draw(*m_debugInstruction);
}

void PPU::Display()
{
    m_window.display();
}

void PPU::Clear()
{
    m_window.clear();
}

