#include "mmu.h"
#include "ram.h"

#define CLOCKSPEED 4194304
#define TIMA 0xFF05
#define TMA 0xFF06
#define TMC 0xFF07

MMU::MMU(BootRom& bootRom)
	: m_bootRom(bootRom)
{
}

void MMU::Map(MemoryBase* mem, uint16_t address) 
{
	if (!m_allMaps.contains(address)) {
		m_allMaps[address] = mem;
	}
	else {
		throw std::exception("This address is already mapped");
	}
}


uint8_t MMU::Read(uint16_t address) 
{
	if (address < 0x0100 && Read(0xFF50) == 0) // If is booting up
	{
		return m_bootRom.Read(address);
	}
	
	for (auto [startAddr, mem] : m_allMaps) 
	{
		uint16_t endAddr = startAddr + mem->Size();
		if (startAddr == 0xffff && address == 0xffff)
		{
			return mem->Read(0);
		}
		if (address >= startAddr && address < endAddr) {
			return mem->Read(address - startAddr);
		}
	}
	std::cout << "Reading in non mapped address " << address << std::endl;
	return 0;
}
void MMU::Write(uint16_t address, uint8_t value) 
{
	for (auto [startAddr, mem] : m_allMaps)
	{
		uint16_t endAddr = startAddr + mem->Size();
		if (address >= startAddr && address < endAddr) {
			mem->Write(address - startAddr, value);
			return;
		}
		if (startAddr == 0xFFFF && address == 0xffff)
		{
			mem->Write(0, value);
			return;
		}
	}
	std::cout << "Writing in non mapped address " << address << std::endl;
	std::cout << "Value was " << static_cast<int>(value) << std::endl;
}

void MMU::SetClockFreq(int& TimerCounter)
{
	uint8_t freq = Read(TMC) & 3;
	switch (freq)
	{
	case 0:
		TimerCounter = 1024;
		break; // freq 4096
	case 1:
		TimerCounter = 16;
		break;// freq 262144
	case 2:
		TimerCounter = 64;
		break;// freq 65536
	case 3:
		TimerCounter = 256;
		break;// freq 16382
	}
}

uint8_t MMU::GetClockFreq()
{
	return Read(TMC) & 3;
}
