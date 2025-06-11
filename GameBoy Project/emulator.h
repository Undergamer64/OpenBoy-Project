#pragma once
#include "RAM.h"
#include "mmu.h"
#include "alu.h"
#include "cpu.h"
#include "Cartidge.h"
#include "ppu.h"

class Emulator
{
public:
	Cartidge m_cartidge;
	
	bool m_debug = false;
	int m_scanlineCounter = 456;
	
	Emulator(MMU& mmu, ALU& alu);
	~Emulator();

	PPU m_ppu;
	CPU m_cpu;
	
	void Map(MemoryBase* mem, uint16_t address);

	void LoadCartridge(const std::string& filepath);
	
	void UpdateGraphics(int cycles);
	void SetLCDStatus();

	bool IsLCDEnabled() const;

	void RequestInterupt(int interrupt);

	bool operator()();
};
