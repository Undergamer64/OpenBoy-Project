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
    
#pragma region OpcodeDef
    cpu += std::unique_ptr<IF_LD_r16_imm16>(new IF_LD_r16_imm16());
    cpu += std::unique_ptr<IF_LD_r8_imm8>(new IF_LD_r8_imm8());
    cpu += std::unique_ptr<IF_LD_rA_memory>(new IF_LD_rA_memory());
    cpu += std::unique_ptr<IF_LD_rA_rHL>(new IF_LD_rA_rHL());
    cpu += std::unique_ptr<IF_AR>(new IF_AR());
    cpu += std::unique_ptr<IF_FLOW_JR>(new IF_FLOW_JR());
    cpu += std::unique_ptr<IF_FLOW_JP>(new IF_FLOW_JP());
    cpu += std::unique_ptr<IF_FLOW_CALL>(new IF_FLOW_CALL());
    cpu += std::unique_ptr<IF_FLOW_RET>(new IF_FLOW_RET());
    cpu += std::unique_ptr<IF_ROTATE>(new IF_ROTATE());
    cpu += std::unique_ptr<IF_CB_Prefix>(new IF_CB_Prefix());
    cpu += std::unique_ptr<IF_INC_DEC>(new IF_INC_DEC());
    cpu += std::unique_ptr<IF_PUSH_POP>(new IF_PUSH_POP());
    cpu += std::unique_ptr<IF_Finish>(new IF_Finish());
#pragma endregion

    while (true)
    {
        int cyclesThisUpdate = 0;
        int cycles = 0;

        while (cyclesThisUpdate < MAXCYCLES)
        {
            cycles = cpu();
            
            if (cycles == -1)
            {
                std::cout << "  Error : No Valide Instruction Family For Value !";
                cycles = -2;
                break;
            }
            else if (cycles == -2) 
            {
                break;
            }
            
            //Sleep for "cycles" cycles time
            
            cyclesThisUpdate += cycles;
            //UpdateTimers(cycles);
            //UpdateGraphics(cycles);
            //DoInterupts();
        }
        if (cycles == -2) 
        {
            break;
        }
        cycles = 0;

        //RenderScreen();
        //std::cout << "\nRefresh\n";
    }

    return 0;
}
