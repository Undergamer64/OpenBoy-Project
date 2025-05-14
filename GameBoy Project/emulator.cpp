#include "emulator.h"

const static int MAXCYCLES = 4194304 / 60; // (number of cycles / frame rate)

Emulator::Emulator(CPU& cpu) 
    : m_cpu(cpu)
{
}

Emulator::~Emulator()
{
}

void Emulator::Map(MemoryBase* mem, uint16_t address) 
{
    m_cpu.Map(mem, address);
}

bool Emulator::operator()() 
{
    int cyclesThisUpdate = 0;
    
    int DebugStep = 0;
    int StepSkip = 1;
    
    while (cyclesThisUpdate < MAXCYCLES)
    {
        int cycles = m_cpu();

        if (cycles == -1)
        {
            std::cout << "Error : No Valide Instruction Family For Value !";
            return false;
        }
        if (cycles == -2)
        {
            std::cout << "Error : Force Exit !";
            return false;
        }

        //Sleep for "cycles" cycles time

        cyclesThisUpdate += cycles;
        //UpdateTimers(cycles);
        UpdateGraphics(cycles);
        //DoInterupts();

        DebugStep++;

        if (m_debug)
        {
            if (m_cpu.m_registers.PC > 0x6A && m_cpu.m_registers.PC < 0x93)
            {
                int Value = m_cpu.DumpRegisters(DebugStep < StepSkip);
            
                if (Value == -1)
                {
                    return false;
                }
                if (Value != 1)
                {
                    StepSkip = Value + DebugStep;
                }
            }
        }
    }

    //RenderScreen();
    //std::cout << "\nRefresh\n";
    
    return true;
}

void Emulator::UpdateGraphics(int cycles)
{
    //SetLCDStatus();

    if (IsLCDEnabled())
    {
        m_scanlineCounter -= cycles ;
    }
    else
    {
        return;
    }

    if (m_scanlineCounter <= 0)
    {
        // time to move onto next scanline
        uint8_t currentLine = m_cpu.Read(0xFF44) + 1;
        
        //std::cout << static_cast<int>(currentLine);
        m_cpu.Write(0xFF44, currentLine);

        m_scanlineCounter = 456;
        
        if (currentLine == 144)// we have entered vertical blank period
        {
            RequestInterupt(0);
        }
        
        else if (currentLine > 153)// if gone past scanline 153 reset to 0
        {
            m_cpu.Write(0xFF44, 0);
        }
        else if (currentLine < 144)// draw the current scanline
        {
            //DrawScanLine();
        }
    }
}

void Emulator::SetLCDStatus()
{
    uint8_t status = m_cpu.Read(0xFF41) ;
    if (false == IsLCDEnabled())
    {
        // set the mode to 1 during lcd disabled and reset scanline
        m_scanlineCounter = 456 ;
        m_cpu.Write(0xFF44, 0);
        status &= 252 ;
        status |= 0b01;
        m_cpu.Write(0xFF41,status) ;
        return ;
    }

    uint8_t currentline = m_cpu.Read(0xFF44) ;
    uint8_t currentmode = status & 0x3 ;

    uint8_t mode = 0 ;
    bool reqInt = false ;
    
    if (currentline >= 144)// in vblank so set mode to 1
    {
        mode = 1;
        status |= 0b01;
        status &= ~0b10;
        reqInt = status & (1 << 4);
    }
    else
    {
        int mode2bounds = 456-80;
        int mode3bounds = mode2bounds - 172;

        if (m_scanlineCounter >= mode2bounds) // mode 2
        {
            mode = 2;
            status |= 0b10;
            status &= ~0b01;
            reqInt = status & (1 << 5);
        }
        else if(m_scanlineCounter >= mode3bounds) // mode 3
        {
            mode = 3;
            status |= 0b10;
            status |= 0b01;
        }
        else // mode 0
        {
            mode = 0;
            status &= ~0b10;
            status &= ~0b01;
            reqInt = status & (1 << 3);
        }
    }
    
    if (reqInt && (mode != currentmode)) // just entered a new mode so request interrupt
    {
        RequestInterupt(1);
    }
    
    if (currentline == m_cpu.Read(0xFF45)) // check the coincidence flag
    {
        status |= 0b100;
        if (status & (1 << 6))
        {
            RequestInterupt(1);
        }
    }
    else
    {
        status &= ~0b100;
    }
    m_cpu.Write(0xFF41,status);
}

bool Emulator::IsLCDEnabled() const
{
    //return m_cpu.Read(0xFF40) >> 7;
    return true;
}

void Emulator::RequestInterupt(int interrupt)
{
    return;
}
