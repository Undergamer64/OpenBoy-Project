#include <iostream>
#include "RAM.h"
#include "mmu.h"
#include "BootRom.h"

int main()
{
    std::cout << "Debug" << std::endl;
    uint8_t a = 0b11111111;
    uint8_t res = a + a + 0b00000001;
    std::cout << std::to_string(res) << std::endl;

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
