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

void MMU::LoadCartridge(const std::string& filepath)
{
#if _DEBUG
    std::cout << "Loading Cartridge: " << filepath << std::endl;
#endif
	
    m_cartidge = Cartidge(filepath);

#if _DEBUG
    std::cout << "Done !" << "\n";
	
    std::cout << "Cartridge size : " << std::hex << m_cartidge.Size() << "\n";
    /*
    std::cout << "Cartridge content (up to 0x8000) :" << std::endl;
    
    for (size_t address = 0; address < m_cartidge.Size(); address++)
    {
        if (address > 0x8000)
        {
            break;
        }	
        m_mmu.Write(address, m_cartidge.Read(address));

        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(m_cartidge.Read(address)) << " ";

        if ((address + 1) % 0x0010 == 0)
        {
            std::cout << "\n";
        }
    }
    
    for (int i = 0; i < 20; i++)
    {
        std::cout << "--";
    }
    std::cout << std::endl;*/
#endif
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
	if (address < 0x8000) // If is cartridge area
    {
	    return m_cartidge.Read(address);//TODO : fix this
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
