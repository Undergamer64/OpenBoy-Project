#pragma once

#include <iostream>
#include <cstdint>
#include <map>
#include "BootRom.h"
#include "Cartridge.h"

class MemoryBase;

class MMU 
{
	std::map<uint16_t, MemoryBase*> m_allMaps;
    Cartridge m_cartidge;
    
public:
	MemoryBase& m_bootRom;
	MMU(BootRom& bootRom);
    
    void LoadCartridge(const std::string& filepath);
	void DebugDumpMemory(MemoryBase* mem);

	void Map(MemoryBase* mem, uint16_t address);
	
	uint8_t Read(uint16_t address);
	void    Write(uint16_t address, uint8_t value);

	void SetClockFreq(int& TimerCounter);
	uint8_t GetClockFreq();
};