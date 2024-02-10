#include <iostream>
#include "RAM.h"
#include "mmu.h"
#include "alu.h"
#include "cpu.h"
#include "BootRom.h"

int main()
{
    //// TEST OF THEORIES

    std::cout << "Debug" << std::endl;
    uint16_t a = 0b0111111111111111;
    uint8_t b = a;
    std::cout << std::to_string(b) << std::endl;

    ////

    BootRom        bootRom("dmg_boot.bin");
    Memory<0x1000> internalRam;
    Memory<0x2000> vram;
    Memory<0x00A0> oam;
    Memory<0x007F> zeroPage;

    MMU mmu;
    mmu.Map(&bootRom, 0x0000);
    mmu.Map(&vram, 0x8000);
    mmu.Map(&internalRam, 0xC000);
    mmu.Map(&oam, 0x9E00);
    mmu.Map(&zeroPage, 0xFF80);

    

    return 0;
}
