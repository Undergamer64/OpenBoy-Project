#include "emulator.h"

const static int MAXCYCLES = 4194304 / 60; // (number of cycles / frame rate)

Emulator::Emulator(MMU& mmu, ALU& alu, CPU& cpu) 
    : m_mmu(mmu),
    m_alu(alu),
    m_cpu(cpu)
{
}

Emulator::~Emulator()
{
}

void Emulator::Map(MemoryBase* mem, uint16_t address) 
{
    m_mmu.Map(mem, address);
}

bool Emulator::operator()() 
{
    int cyclesThisUpdate = 0;
    int cycles = 0;

    if (m_debug)
    {
        m_cpu.BootDump();
        return false;
    }

    int DebugStep = 0;
    int StepSkip = 1;
    
    while (cyclesThisUpdate < MAXCYCLES)
    {
        cycles = m_cpu();

        if (cycles == -1)
        {
            std::cout << "  Error : No Valide Instruction Family For Value !";
            return false;
        }
        if (cycles == -2)
        {
            std::cout << "  Error : Force Exit !";
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

    //RenderScreen();
    //std::cout << "\nRefresh\n";
    
    return true;
}

void Emulator::UpdateGraphics(int cycles)
{
    SetLCDStatus();

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
        uint8_t currentLine = m_mmu.Read(0xFF44) + 1;
        m_mmu.Write(0xFF44, currentLine);

        m_scanlineCounter = 456;
        
        if (currentLine == 144)// we have entered vertical blank period
        {
            RequestInterupt(0);
        }
        
        else if (currentLine > 153)// if gone past scanline 153 reset to 0
        {
            m_mmu.Write(0xFF44, 0);
        }
        else if (currentLine < 144)// draw the current scanline
        {
            //DrawScanLine();
        }
    }
}

void Emulator::SetLCDStatus()
{
    uint8_t status = m_mmu.Read(0xFF41) ;
    if (false == IsLCDEnabled())
    {
        // set the mode to 1 during lcd disabled and reset scanline
        m_scanlineCounter = 456 ;
        m_mmu.Write(0xFF44, 0);
        status &= 252 ;
        status |= 0b01;
        m_mmu.Write(0xFF41,status) ;
        return ;
    }

    uint8_t currentline = m_mmu.Read(0xFF44) ;
    uint8_t currentmode = status & 0x3 ;

    uint8_t mode = 0 ;
    bool reqInt = false ;

    // in vblank so set mode to 1
    if (currentline >= 144)
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

        // mode 2
        if (m_scanlineCounter >= mode2bounds)
        {
            mode = 2;
            status |= 0b10;
            status &= ~0b01;
            reqInt = status & (1 << 5);
        }
        // mode 3
        else if(m_scanlineCounter >= mode3bounds)
        {
            mode = 3;
            status |= 0b10;
            status |= 0b01;
        }
        // mode 0
        else
        {
            mode = 0;
            status &= ~0b10;
            status &= ~0b01;
            reqInt = status & (1 << 3);
        }
    }

    // just entered a new mode so request interrupt
    if (reqInt && (mode != currentmode))
    {
        RequestInterupt(1);
    }

    // check the coincidence flag
    if (currentline == m_mmu.Read(0xFF45))
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
    m_mmu.Write(0xFF41,status);
}

bool Emulator::IsLCDEnabled() const
{
    //return m_mmu.Read(0xFF40) >> 7;
    return true;
}

void Emulator::RequestInterupt(int interrupt)
{
    return;
}
