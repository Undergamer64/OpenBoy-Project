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
            return false;
        }

        //Sleep for "cycles" cycles time

        cyclesThisUpdate += cycles;
        //UpdateTimers(cycles);
        //UpdateGraphics(cycles);
        //DoInterupts();
    }

    //RenderScreen();
    //std::cout << "\nRefresh\n";
    
    return true;
}
