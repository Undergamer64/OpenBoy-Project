#pragma once

#include <iostream>
#include <cstdint>
#include <map>

#include "BootRom.h"

class MemoryBase;

class MMU 
{
	
	std::map<uint16_t, MemoryBase*> m_allMaps;
	MemoryBase& m_bootRom;
public:
	MMU(BootRom& bootRom);
	
	void Map(MemoryBase* mem, uint16_t address);
	
	uint8_t Read(uint16_t address);
	void    Write(uint16_t address, uint8_t value);
};