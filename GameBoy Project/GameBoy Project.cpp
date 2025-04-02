#include <iostream>
#include "emulator.h"
#include "RAM.h"
#include "mmu.h"
#include "alu.h"
#include "cpu.h"
#include "BootRom.h"

int main() {
    MMU mmu;
    ALU alu;
    CPU cpu(mmu, alu);

    Emulator emulator(mmu, alu, cpu);

    BootRom        bootRom("dmg_boot.bin");
    Memory<0x1000> internalRam;
    Memory<0x2000> vram;
    Memory<0x00A0> oam;
    Memory<0x007F> zeroPage;

    emulator.Map(&bootRom    , 0x0000);
    emulator.Map(&vram       , 0x8000);
    emulator.Map(&internalRam, 0xC000);
    emulator.Map(&oam        , 0x9E00);
    emulator.Map(&zeroPage   , 0xFF80);

#pragma region OpcodeDef
    emulator.m_cpu += std::make_unique<IF_LD_r16_imm16>();
    emulator.m_cpu += std::make_unique<IF_AR>();
    emulator.m_cpu += std::make_unique<IF_LD_rA_rHL>();
    emulator.m_cpu += std::make_unique<IF_CB_Prefix>();
    emulator.m_cpu += std::make_unique<IF_FLOW_JR>();
    emulator.m_cpu += std::make_unique<IF_LD_r8_imm8>();
    emulator.m_cpu += std::make_unique<IF_LD_ADR_r>();
    emulator.m_cpu += std::make_unique<IF_FLOW_JP>();
    emulator.m_cpu += std::make_unique<IF_INC_DEC>();
    emulator.m_cpu += std::make_unique<IF_LD_ADRIMM_r>();
    emulator.m_cpu += std::make_unique<IF_FLOW_CALL>();
    emulator.m_cpu += std::make_unique<IF_LD_rA_memory>();
    emulator.m_cpu += std::make_unique<IF_PUSH_POP>();
    emulator.m_cpu += std::make_unique<IF_ROTATE>();
    emulator.m_cpu += std::make_unique<IF_FLOW_RET>();
    emulator.m_cpu += std::make_unique<IF_LD_r_r>();
    /*
    emulator.m_cpu += std::make_unique<IF_LD_SP_HL>();
    emulator.m_cpu += std::make_unique<IF_Finish>();
    */
#pragma endregion

    //emulator.m_debug = true;
    
    while (true)
    {
        if (!emulator()) 
        {
            std::cout << "Emulator End" << std::endl;
            break;
        }
    }

    return 0;
}
