#include "./emulator.h"

#include <iostream>

#define CLOCKSPEED 4194304
#define TIMA 0xFF05
#define TMA 0xFF06
#define TMC 0xFF07

static int MAXCYCLES = CLOCKSPEED / 59.7275f; //Debug

Emulator::Emulator(MMU& mmu, ALU& alu)
    : m_cartidge(Cartridge())
      , m_ppu(mmu)
      , m_cpu(mmu, alu)
{
}

Emulator::~Emulator() = default;

void Emulator::SkipBootRom()
{
    // Set registers to post boot rom values (BRUTE FORCE METHOD) source : Pan Docs
    
    m_cpu.m_registers.PC = 0x101;
    m_cpu.m_registers.SP = 0xFFFE;
    m_cpu.m_registers.IME = true;
    m_cpu.m_registers.m_registers[0] = 0x01; // A
    m_cpu.m_registers.m_registers[1] = 0xB0; // F
    m_cpu.m_registers.m_registers[2] = 0x13; // B
    m_cpu.m_registers.m_registers[3] = 0x00; // C
    m_cpu.m_registers.m_registers[4] = 0xD8; // D
    m_cpu.m_registers.m_registers[5] = 0x01; // E
    m_cpu.m_registers.m_registers[6] = 0x4D; // H
    m_cpu.m_registers.m_registers[7] = 0x00; // L
    
    m_cpu.ForceWrite(0xFF00, 0xCF); // P1
    m_cpu.ForceWrite(0xFF01, 0x00); // SB
    m_cpu.ForceWrite(0xFF02, 0x7E); // SC
    m_cpu.ForceWrite(0xFF04, 0xAB); // DIV
    m_cpu.ForceWrite(0xFF05, 0x00); // TIMA
    m_cpu.ForceWrite(0xFF06, 0x00); // TMA
    m_cpu.ForceWrite(0xFF07, 0xF8); // TAC
    m_cpu.ForceWrite(0xFF0F, 0xE1); // IF
    m_cpu.ForceWrite(0xFF10, 0x80); // NR10
    m_cpu.ForceWrite(0xFF11, 0xBF); // NR11
    m_cpu.ForceWrite(0xFF12, 0xF3); // NR12
    m_cpu.ForceWrite(0xFF14, 0xBF); // NR14
    m_cpu.ForceWrite(0xFF16, 0x3F); // NR21
    m_cpu.ForceWrite(0xFF17, 0x00); // NR22
    m_cpu.ForceWrite(0xFF19, 0xBF); // NR24
    m_cpu.ForceWrite(0xFF1A, 0x7F); // NR30
    m_cpu.ForceWrite(0xFF1B, 0xFF); // NR31
    m_cpu.ForceWrite(0xFF1C, 0x9F); // NR32
    m_cpu.ForceWrite(0xFF1E, 0xBF); // NR33
    m_cpu.ForceWrite(0xFF20, 0xFF); // NR41
    m_cpu.ForceWrite(0xFF21, 0x00); // NR42
    m_cpu.ForceWrite(0xFF22, 0x00); // NR43
    m_cpu.ForceWrite(0xFF23, 0xBF); // NR44
    m_cpu.ForceWrite(0xFF24, 0x77); // NR50
    m_cpu.ForceWrite(0xFF25, 0xF3); // NR51
    m_cpu.ForceWrite(0xFF40, 0x91); // LCDC
    m_cpu.ForceWrite(0xFF41, 0x85); // STAT
    m_cpu.ForceWrite(0xFF42, 0x00); // SCY
    m_cpu.ForceWrite(0xFF43, 0x00); // SCX
    m_cpu.ForceWrite(0xFF45, 0x00); // LYC
    m_cpu.ForceWrite(0xFF47, 0xFC); // BGP
    m_cpu.ForceWrite(0xFF48, 0xFF); // OBP0
    m_cpu.ForceWrite(0xFF49, 0xFF); // OBP1
    m_cpu.ForceWrite(0xFF4A, 0x00); // WY
    m_cpu.ForceWrite(0xFF4B, 0x00); // WX
    m_cpu.ForceWrite(0xFF50, 0x01); // BOOT
    m_cpu.ForceWrite(0xFFFF, 0x00); // IE
}

void Emulator::Map(MemoryBase* mem, uint16_t address) 
{
    m_cpu.Map(mem, address);
}

void Emulator::LoadCartridge(const std::string& filepath)
{
    m_cpu.LoadCartridge(filepath);
}

void Emulator::operator()() 
{
    Render();
    
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFrame = std::chrono::high_resolution_clock::now();
    
    while (true)
    {
        std::chrono::time_point<std::chrono::high_resolution_clock> t;
        
        do
        {
            t = std::chrono::high_resolution_clock::now();
        } while (std::chrono::duration_cast<std::chrono::milliseconds>(t - lastFrame).count() < (1.f/59.7275f) * 1000);
        
        lastFrame = std::chrono::high_resolution_clock::now();
        Execute();

        if (!Render()) return;
    }
}

void Emulator::Execute()
{
    if (!m_isRunning)
    {
        return;
    }
    
    int cyclesThisUpdate = 0; // cycles in T-cycles (1 M-cycle = 4 T-cycles)
    
    while (cyclesThisUpdate < MAXCYCLES)
    {
        if (cyclesThisUpdate % 4 == 0)// only tick cpu every 4 t-cycles (or every 1 m-cycle)
        {
            int errorCode = m_cpu.Tick();
        
            if (errorCode == -1)
            {
                m_isRunning = false;
                std::cout << "Emulation Stopped !" << std::endl;
                break;
            }
            if (errorCode == -2)
            {
                m_isRunning = false;
                std::cout << "Forced Exit !" << std::endl;
                break;
            }
        }

        UpdateTimers();

        //had to organize it like that because of cyclic dependencies
        m_ppu.SetLCDStatus();
        CheckLYFlag();
        m_ppu.UpdateGraphics();

        // TODO : intergrate interrupts properly in the cpu tick function
        //cyclesThisUpdate += DoInterupts();
        
        cyclesThisUpdate++;
    }
}

void Emulator::DebugSlowDown() const {
    if (m_cpu.m_registers.PC == 0x6a)
    {
        MAXCYCLES = 1;
        std::cout << "Debug Slow Down" << std::endl;
    }
}

void Emulator::UpdateTimers()
{
    DoDividerRegister();

    // the clock must be enabled to update the clock
    if (IsClockEnabled())
    {
        m_TimerCounter --;

        // enough cpu clock cycles have happened to update the timer
        if (m_TimerCounter <= 0)
        {
            // reset m_TimerTracer to the correct value
            m_cpu.SetClockFreq(m_TimerCounter);

            // timer about to overflow
            if (m_cpu.Read(TIMA) == 255)
            {
                m_cpu.Write(TIMA,m_cpu.Read(TMA)) ;
                RequestInterrupt(2) ;
            }
            else
            {
                m_cpu.Write(TIMA, m_cpu.Read(TIMA)+1) ;
            }
        }
    }
}

void Emulator::CheckLYFlag()
{
    uint8_t status = m_cpu.Read(0xFF41);
    uint8_t currentLine = m_cpu.Read(0xFF44) ;
    if (currentLine == m_cpu.Read(0xFF45)) // check LYC = LY and set bit 2 of status accordingly
    {
        status |= 0b100;
        if (status & (1 << 6))
        {
            RequestInterrupt(1);
        }
    }
    else
    {
        status &= ~0b100;
    }
    m_cpu.Write(0xFF41,status);
}

bool Emulator::IsClockEnabled()
{
    return (m_cpu.Read(TMC) & 0b00000100) != 0;
}

void Emulator::RequestInterrupt(const int interrupt)
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
                        //std::cout << "Servicing Interrupt " << i << std::endl;
                        return ServiceInterrupt(i);
                    }
                }
            }
        }
    }
    return 0;
}

int Emulator::ServiceInterrupt(int interrupt)
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
    default:
        break;
    }
    return 20;
}

void Emulator::DoDividerRegister()
{
    m_DividerCounter ++;
    if (m_DividerCounter >= 255)
    {
        m_DividerCounter = 0;
        m_cpu.ForceWrite(0xFF04, m_cpu.Read(0xFF04) + 1);
    }
}

bool Emulator::Render()
{
    if (!m_ppu.RenderScreen())
    {
        std::cout << "No Render !" << std::endl;
        return false;
    }
    //std::cout << "\nRefresh\n";

    //Render Debug values here
    m_ppu.RenderDebug(m_cpu, m_isRunning);
    
    m_ppu.Display();
    m_ppu.Clear();
    return true;
}
