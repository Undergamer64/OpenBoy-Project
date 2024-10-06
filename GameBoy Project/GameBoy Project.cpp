#include <iostream>
#include "RAM.h"
#include "mmu.h"
#include "alu.h"
#include "cpu.h"
#include "BootRom.h"

int main()
{

    const int MAXCYCLES = 4194304 / 60; // (number of cycles / frame rate)

    BootRom        bootRom("dmg_boot.bin");
    Memory<0x1000> internalRam;
    Memory<0x2000> vram;
    Memory<0x00A0> oam;
    Memory<0x007F> zeroPage;

    MMU mmu;
    mmu.Map(&bootRom    , 0x0000);
    mmu.Map(&vram       , 0x8000);
    mmu.Map(&internalRam, 0xC000);
    mmu.Map(&oam        , 0x9E00);
    mmu.Map(&zeroPage   , 0xFF80);

    ALU alu;

    CPU cpu(mmu, alu);

    //cpu += ;

    while (true)
    {
        int cyclesThisUpdate = 0;

        while (cyclesThisUpdate < MAXCYCLES)
        {
            //Sleep for 4 cycles
            int cycles = cpu();
            //Sleep for "cycles" cycles time

            
            cyclesThisUpdate += cycles;
            //UpdateTimers(cycles);
            //UpdateGraphics(cycles);
            //DoInterupts();
        }
        //RenderScreen();
    }

    return 0;
}
