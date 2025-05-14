#include "mmu.h"
#include "ram.h"

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
	for (auto [startAddr, mem] : m_allMaps) 
	{
		uint16_t endAddr = startAddr + mem->Size();
		if (address >= startAddr && address < endAddr) {
			return mem->Read(address - startAddr);
		}
	}
	std::cout << " Not mapped, address : " << address << std::endl;
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
	}
	std::cout << " Not mapped" << address << std::endl;
}