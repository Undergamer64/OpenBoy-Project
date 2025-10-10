#include "emulator.h"

#include <Windows.h>
#include <fstream>

#if _DEBUG
const static int MAXCYCLES = 4194304 / 60; // (number of cycles / frame rate)
#else
const static int MAXCYCLES = 1; //Debug
#endif

Emulator::Emulator(MMU& mmu, ALU& alu) 
    : m_cartidge(Cartidge())
    , m_cpu(mmu, alu)
    , m_ppu()
{
}

Emulator::~Emulator()
{
}

void Emulator::Map(MemoryBase* mem, uint16_t address) 
{
    m_cpu.Map(mem, address);
}

void Emulator::LoadCartridge(const std::string& filepath)
{
    m_cpu.LoadCartridge(filepath);
}

bool Emulator::operator()() 
{
    int cyclesThisUpdate = 0;
    
    while (cyclesThisUpdate < MAXCYCLES)
    {
        int cycles = m_cpu.Execute();

        if (cycles == -1)
        {
            std::cout << "Error : No Valide Instruction Family For Value !" << std::endl;
            return false;
        }
        if (cycles == -2)
        {
            std::cout << "Error : Force Exit !" << std::endl;
            return false;
        }

        cyclesThisUpdate += cycles;
        UpdateTimers(cycles);
        UpdateGraphics(cycles); 
        cyclesThisUpdate += DoInterupts();
    }
    
    if (!m_ppu.RenderScreen())
    {
        return false;  
    }
    //std::cout << "\nRefresh\n";

#if _DEBUG
    //Render Debug values here
    m_ppu.RenderDebug(m_cpu);
#endif

    m_ppu.Display();
    m_ppu.Clear();
    
    return true;
}

void Emulator::UpdateTimers(int cycles)
{
    
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
        uint8_t currentLine = m_cpu.Read(0xFF44) + 1;
        
        //std::cout << static_cast<int>(currentLine) << std::endl;
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
    uint8_t req = m_cpu.Read(0xFF0F);
    req |= (1 << interrupt);
    m_cpu.Write(0xFF0F, req);
}

int Emulator::DoInterupts()
{
    if (m_cpu.m_registers.IME == true)
    {
        uint8_t req = m_cpu.Read(0xFF0F) ;
        uint8_t enabled = m_cpu.Read(0xFFFF) ;
        if (req > 0)
        {
            for (int i = 0 ; i < 5; i++)
            {
                if ((req & (1 << i)) != 0)
                {
                    if ((enabled & (1 << i)) != 0)
                    {
                        std::cout << "Servicing Interrupt " << i << std::endl;
                        return ServiceInterupt(i);
                    }
                }
            }
        }
    }
    return 0;
}

int Emulator::ServiceInterupt(int interrupt)
{
    m_cpu.m_registers.IME = false;
    uint8_t req = m_cpu.Read(0xFF0F) ;
    req = req & ~(1 << interrupt);
    m_cpu.Write(0xFF0F,req) ;

    /// we must save the current execution address by pushing it onto the stack
    m_cpu.Push(m_cpu.m_registers.PC);

    switch (interrupt)
    {
    case 0:
        m_cpu.m_registers.PC = 0x40;
        break;
    case 1:
        m_cpu.m_registers.PC = 0x48;
        break;
    case 2:
        m_cpu.m_registers.PC = 0x50;
        break;
    case 4:
        m_cpu.m_registers.PC = 0x60;
        break;
    }
    return 20;
}
