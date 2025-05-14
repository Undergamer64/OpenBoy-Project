#pragma once
#include "RAM.h"
#include "mmu.h"
#include "alu.h"
#include "cpu.h"
#include "BootRom.h"

class Emulator
{
	
public:
	bool m_debug = false;
	int m_scanlineCounter = 456;
	
	Emulator(CPU& cpu);
	~Emulator();

	CPU& m_cpu;
	
	void Map(MemoryBase* mem, uint16_t address);

	void UpdateGraphics(int cycles);
	void SetLCDStatus();

	bool IsLCDEnabled() const;

	void RequestInterupt(int interrupt);

	bool operator()();
};