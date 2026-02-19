#pragma once
#include "Cartridge.h"
#include "cpu.h"
#include "./ppu.h"

class MMU;
class ALU;

class Emulator
{
public:
	Cartridge m_cartidge;
	bool m_isRunning = true;
	
	int m_dotInScanline = 456;
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
	void UpdateGraphics();
	void SetLCDStatus();

	bool IsLCDEnabled();

	bool IsClockEnabled();
	
	void RequestInterupt(int interrupt);
	int DoInterupts();
	int ServiceInterupt(int interrupt);

	void operator()();
	void Execute();

	void DebugSlowDown() const;

	void DoDividerRegister();

	bool Render();
};
