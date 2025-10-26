#include "ppu.h"

#include <iostream>
#include <optional>

#include "mmu.h"

enum
{
    WHITE = 0b00,
    LIGHT_GRAY = 0b01,
    DARK_GRAY = 0b10,
    BLACK = 0b11
};

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

PPU::PPU(MMU& mmu)
    : m_window(sf::VideoMode( sf::Vector2u(160*8,144*8), 600), "GameBoy Project"), m_mmu(mmu)
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
    m_debugRom->setOutlineColor(sf::Color::Black);

    sf::Vector2f pixelSize(
        std::min(size.x / 160, size.y / 144) * 160,
        std::min(size.x / 160, size.y / 144) * 144
    );

    for (int y = 0; y < 144; y++)
    {
        std::vector<sf::RectangleShape*> row;
        for (int x = 0; x < 160; x++)
        {
            sf::RectangleShape* pixel = new sf::RectangleShape(pixelSize);
            pixel->setFillColor(sf::Color::White);
            pixel->setPosition(sf::Vector2f(static_cast<float>(x * 8), static_cast<float>(y * 8)));
            row.push_back(pixel);
        }
        m_ScreenData.push_back(row);
    }
    
}

bool PPU::RenderScreen()
{
    if (!IsWindowOpen()) return false;

    //Render the game here
    for (int y = 0; y < 144; y++)
    {
        for (int x = 0; x < 160; x++)
        {
            m_window.draw(*m_ScreenData[y][x]);
        }
    }
    
    return true;
}

void PPU::DrawScanLine(uint8_t currentLine)
{
    uint8_t control = m_mmu.Read(0xFF40);
    if ((control & 1) == 1)
        RenderTiles(control);

    if ((control >> 1 & 1) == 1)
        RenderSprites(control);
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

void PPU::RenderTiles(uint8_t lcdControl)
{
    uint16_t tileData = 0;
    uint16_t backgroundMemory = 0;
    bool unsig = true;

    // where to draw the visual area and the window
    uint8_t scrollY = m_mmu.Read(0xFF42);
    uint8_t scrollX = m_mmu.Read(0xFF43);
    uint8_t windowY = m_mmu.Read(0xFF4A);
    uint8_t windowX = m_mmu.Read(0xFF4B) - 7;

    bool usingWindow = false;

    // is the window enabled?
    if ((lcdControl >> 5 & 1) == 1)
    {
        // is the current scanline we're drawing
        // within the windows Y pos?,
        if (windowY <= m_mmu.Read(0xFF44))
            usingWindow = true;
    }

    // which tile data are we using?
    if ((lcdControl >> 4 & 1) == 1)
    {
        tileData = 0x8000;
    }
    else
    {
        // IMPORTANT: This memory region uses signed
        // bytes as tile identifiers
        tileData = 0x8800;
        unsig = false;
    }

    // which background mem?
    if (false == usingWindow)
    {
        if ((lcdControl >> 3 & 1) == 1)
            backgroundMemory = 0x9C00;
        else
            backgroundMemory = 0x9800;
    }
    else
    {
        // which window memory?
        if ((lcdControl >> 6 & 1) == 1)
            backgroundMemory = 0x9C00;
        else
            backgroundMemory = 0x9800;
    }

    uint8_t yPos = 0;

    // yPos is used to calculate which of 32 vertical tiles the
    // current scanline is drawing
    if (!usingWindow)
        yPos = scrollY + m_mmu.Read(0xFF44);
    else
        yPos = m_mmu.Read(0xFF44) - windowY;

    // which of the 8 vertical pixels of the current
    // tile is the scanline on?
    uint16_t tileRow = (static_cast<uint8_t>(yPos / 8)*32);

    // time to start drawing the 160 horizontal pixels
    // for this scanline
    for (int pixel = 0; pixel < 160; pixel++)
    {
        uint8_t xPos = pixel + scrollX;

        // translate the current x pos to window space if necessary
        if (usingWindow)
        {
            if (pixel >= windowX)
            {
                xPos = pixel - windowX;
            }
        }

        // which of the 32 horizontal tiles does this xPos fall within?
        uint16_t tileCol = (xPos/8);

        //TODO : find how to integrate SIGNED_WORD
        int16_t tileNum;

        // get the tile identity number. Remember it can be signed
        // or unsigned
        uint16_t tileAddrss = backgroundMemory+tileRow+tileCol;
        if(unsig)
        {
            tileNum = m_mmu.Read(tileAddrss);
        }
        else
        {
            //TODO : find how to integrate SIGNED_BYTE
            tileNum =static_cast<int8_t>(m_mmu.Read(tileAddrss));
        }

        // deduce where this tile identifier is in memory.
        uint16_t tileLocation = tileData;

        if (unsig)
        {
            tileLocation += (tileNum * 16);
        }
        else
        {
            tileLocation += ((tileNum+128) *16);
        }

        // find the correct vertical line we're on of the
        // tile to get the tile data
        //from in memory
        uint8_t line = yPos % 8;
        line *= 2; // each vertical line takes up two bytes of memory
        uint8_t data1 = m_mmu.Read(tileLocation + line);
        uint8_t data2 = m_mmu.Read(tileLocation + line + 1);

        // pixel 0 in the tile is it 7 of data 1 and data2.
        // Pixel 1 is bit 6 etc..
        int colourBit = xPos % 8;
        colourBit -= 7;
        colourBit *= -1;

        // combine data 2 and data 1 to get the colour id for this pixel
        // in the tile
        int colourNum = data2 >> colourBit & 1;
        colourNum <<= 1;
        colourNum |= data1 >> colourBit & 1;

        // now we have the colour id get the actual
        // colour from palette 0xFF47
        uint8_t col = m_mmu.Read(0xFF47) >> colourNum * 2 & 0b11;
        int red = 0;
        int green = 0;
        int blue = 0;

        // setup the RGB values
        switch(col)
        {
        case WHITE:
            red = 255;
            green = 255;
            blue = 255;
            break;
        case LIGHT_GRAY:
            red = 0xCC;
            green = 0xCC;
            blue = 0xCC;
            break;
        case DARK_GRAY:
            red = 0x77;
            green = 0x77;
            blue = 0x77;
            break;
        default:
            red = 0;
            green = 0;
            blue = 0;
            break;
        }

        int scanline = m_mmu.Read(0xFF44);

        // safety check to make sure what im about
        // to set is in the 160x144 bounds
        if ((scanline<0)||(scanline>143)||(pixel<0)||(pixel>159))
        {
            continue;
        }

        if (col != WHITE)
        {
            std::cout << "Drawing tile pixel at (" << scanline << "," << pixel << ")\n";
            std::cout << "Colour is " << static_cast<int>(col) << "\n\n";
        }

        m_ScreenData[scanline][pixel]->setFillColor(sf::Color(red, green, blue));
    }
}

void PPU::RenderSprites(uint8_t lcdControl)
{
    bool use8x16 = false;
    if ((lcdControl >> 2 & 1) == 1)
        use8x16 = true;

    for (int sprite = 0; sprite < 40; sprite++)
    {
        // sprite occupies 4 bytes in the sprite attributes table
        uint8_t index = sprite * 4;
        uint8_t yPos = m_mmu.Read(0xFE00+index) - 16;
        uint8_t xPos = m_mmu.Read(0xFE00+index+1) - 8;
        uint8_t tileLocation = m_mmu.Read(0xFE00+index+2);
        uint8_t attributes = m_mmu.Read(0xFE00+index+3);

        bool yFlip = (attributes >> 6 & 1) == 1;
        bool xFlip = (attributes >> 5 & 1) == 1;

        int scanline = m_mmu.Read(0xFF44);

        int ysize = 8;
        if (use8x16)
            ysize = 16;

        // does this sprite intercept with the scanline?
        if ((scanline >= yPos) && (scanline < (yPos+ysize)))
        {
            int line = scanline - yPos;

            // read the sprite in backwards in the y axis
            if (yFlip)
            {
                line -= ysize;
                line *= -1;
            }

            line *= 2; // same as for tiles
            uint16_t dataAddress = (0x8000 + (tileLocation * 16)) + line;
            uint8_t data1 = m_mmu.Read(dataAddress);
            uint8_t data2 = m_mmu.Read(dataAddress + 1);

            // its easier to read in from right to left as pixel 0 is
            // bit 7 in the colour data, pixel 1 is bit 6 etc...
            for (int tilePixel = 7; tilePixel >= 0; tilePixel--)
            {
                int colourBit = tilePixel;
                // read the sprite in backwards for the x axis
                if (xFlip)
                {
                    colourBit -= 7;
                    colourBit *= -1;
                }

                // the rest is the same as for tiles
                int colourNum = data2 >> colourBit & 1;
                colourNum <<= 1;
                colourNum |= data1 >> colourBit & 1;

                uint16_t colourAddress = (attributes >> 4 & 1) == 1 ? 0xFF49:0xFF48;
                uint8_t col = m_mmu.Read(colourAddress) >> colourNum * 2 & 0b11;

                // // white is transparent for sprites.
                // if (col == WHITE)
                //   continue;

                int red = 0;
                int green = 0;
                int blue = 0;

                switch(col)
                {
                case WHITE:
                    red = 255;
                    green = 255;
                    blue = 255;
                    break;
                case LIGHT_GRAY:
                    red = 0xCC;
                    green = 0xCC;
                    blue = 0xCC;
                    break;
                case DARK_GRAY:
                    red = 0x77;
                    green = 0x77;
                    blue = 0x77;
                    break;
                default:
                    break;
                }

                int xPix = 0 - tilePixel;
                xPix += 7;

                int pixel = xPos+xPix;

                // sanity check
                if ((scanline<0)||(scanline>143)||(pixel<0)||(pixel>159))
                {
                    continue;
                }

                std::cout << "Drawing sprite pixel at (" << scanline << "," << pixel << ")\n";
                std::cout << "Colour is " << static_cast<int>(col) << "\n\n";
                m_ScreenData[scanline][pixel]->setFillColor(sf::Color(red, green, blue));
            }
        }
    }
}
