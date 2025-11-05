#pragma once
#include "Cartidge.h"
#include "cpu.h"
#include "ppu.h"

class MMU;
class ALU;

class Emulator
{
public:
	Cartidge m_cartidge;
	
	int m_scanlineCounter = 456;
	int m_TimerCounter = 1024;
	int m_DividerCounter = 0;
	
	Emulator(MMU& mmu, ALU& alu);
	~Emulator();

	PPU m_ppu;
	CPU m_cpu;
	
	void Map(MemoryBase* mem, uint16_t address);

	void LoadCartridge(const std::string& filepath);

	void UpdateTimers(int cycles);
	void UpdateGraphics(int cycles);
	void SetLCDStatus();

	bool IsLCDEnabled();

	bool IsClockEnabled();
	
	void RequestInterupt(int interrupt);
	int DoInterupts();
	int ServiceInterupt(int interrupt);

	bool operator()();

	void DebugSlowDown();

	void DoDividerRegister(int cycles);
};
