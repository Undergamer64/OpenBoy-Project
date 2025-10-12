#include <chrono>
#include <iostream>
#include "emulator.h"
#include "RAM.h"
#include "mmu.h"
#include "alu.h"
#include "cpu.h"
#include "BootRom.h"

int main()
{
    BootRom bootRom("dmg_boot.bin");
    
    MMU mmu(bootRom);
    ALU alu;

    Emulator emulator(mmu, alu);

    Memory<0x4000> romZeroBank;
    Memory<0x4000> romBanks;
    Memory<0x2000> vram;
    Memory<0x2000> ram;
    Memory<0x1E00> echoRam;
    Memory<0x1000> internalRam;
    Memory<0x1000> switchableInternalRam;
    Memory<0x00A0> oam;
    Memory<0x0060> unusableMemory;
    Memory<0x0080> ioRegisters;
    Memory<0x007F> zeroPage;
    Memory<0x0001> interruptActivate;

    emulator.Map(&romZeroBank    , 0x0000); //Bank zero of the rom
    emulator.Map(&romBanks    , 0x4000); //Switchable banks for Rom (bank 1 to n)
    emulator.Map(&vram       , 0x8000);
    emulator.Map(&ram       , 0xA000);
    emulator.Map(&internalRam, 0xC000); //WRAM
    emulator.Map(&switchableInternalRam, 0xD000); //Switchable WRAM
    emulator.Map(&echoRam, 0xE000); //ECHO RAM (replicates to ram)
    emulator.Map(&oam        , 0xFE00);
    emulator.Map(&unusableMemory        , 0xFEA0); // prohibited by nintendo
    emulator.Map(&ioRegisters        , 0xFF00);
    emulator.Map(&zeroPage   , 0xFF80);
    emulator.Map(&interruptActivate   , 0xFFFF);
    emulator.m_cpu.Write(0xFFFF, 0xFF);

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
    emulator.m_cpu += std::make_unique<IF_LD_SP_HL>();
    /*
    emulator.m_cpu += std::make_unique<IF_Finish>();
    */
#pragma endregion

    emulator.LoadCartridge("Tetris.gb");

#if _DEBUG
    std::cout << emulator.m_cpu.DumpBoot().str() << std::endl;
#endif

    std::chrono::time_point<std::chrono::high_resolution_clock> lastFrame = std::chrono::high_resolution_clock::now();
    
    while (true)
    {
        std::chrono::time_point<std::chrono::high_resolution_clock> t;
        do
        {
            t = std::chrono::high_resolution_clock::now();
#if _DEBUG
        } while (std::chrono::duration_cast<std::chrono::microseconds>(t - lastFrame).count() < 16667); // ~60FPS
#else
        } while (std::chrono::duration_cast<std::chrono::milliseconds>(t - lastFrame).count() < 500); // ~5FPS
#endif
        lastFrame = std::chrono::high_resolution_clock::now();
        if (!emulator()) 
        {
            std::cout << "Emulator End" << std::endl;
            break;
        }
    }

    return 0;
}
