#pragma once
#include "Cartridge.h"
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

class ALU;

class Emulator
{
public:
	Cartridge m_cartidge;
	bool m_isRunning = true;

	int m_TimerCounter = 1024;
	int m_DividerCounter = 0;
	
	Emulator(MMU& mmu, ALU& alu);
	~Emulator();
    
    PPU m_ppu;
	CPU m_cpu;
    
    void SkipBootRom();
	
	void Map(MemoryBase* mem, uint16_t address);

	void LoadCartridge(const std::string& filepath);

	void UpdateTimers();
    void CheckLYFlag();
    void SetLCDStatus();

	bool IsClockEnabled();

	void RequestInterrupt(int interrupt);
	int DoInterupts();
	int ServiceInterrupt(int interrupt);

	void operator()();
	void Execute();

	void DebugSlowDown() const;

	void DoDividerRegister();

	bool Render();
};
