#include <iostream>
#include "RAM.h"
#include "mmu.h"
#include "alu.h"
#include "cpu.h"
#include "BootRom.h"

#pragma region TEST
///TEST///
#define PCREAD8() ([&]() { \
	currentCycles += 4; \
	return mmu.Read(registers.PC++); })()

#define MMUREAD8(REG) ([&]() { \
	currentCycles += 4; \
	return mmu.Read(registers.REG); })()

#define READ16() ([&]() {\
	return PCREAD8() + (PCREAD8() << 8); })()

#define MMUWRITE8(ADDR, VAL) {\
	currentCycles += 4; \
	mmu.Write(ADDR, VAL); }

#define WRITE16(REG, VAL) {\
	registers.REG = VAL; }

class IF_LD_r16_imm16 final
    : public InstructionFamily
{
public:

    bool IsValid(uint8_t opcode) override
    {
        return (opcode & 0b11001111) == 0b00000001;
    }

    int Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
    {
        int currentCycles = 0;

        std::cout << "Start";

        switch (opcode & 0b00110000) {
        case 0b00000000:
            WRITE16(BC, READ16());
            break;
        case 0b00010000:
            WRITE16(DE, READ16());
            break;
        case 0b00100000:
            WRITE16(HL, READ16());
            break;
        case 0b00110000:
            WRITE16(SP, READ16());
            break;
        }

        return currentCycles;
    }
};
///TEST///
#pragma endregion

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

    //cpu += std::unique_ptr<IF_LD_r16_imm16>();

    while (true)
    {
        int cyclesThisUpdate = 0;

        while (cyclesThisUpdate < MAXCYCLES)
        {
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
