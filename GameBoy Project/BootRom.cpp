#include "BootRom.h"

#include <fstream>

BootRom::BootRom(const std::string& filepath)
{
	std::ifstream ifs(filepath);
	if (ifs.good()) 
	{
		ifs.read(reinterpret_cast<char*>(m_bytes.data()), 258);
		ifs.close();
	}
	else 
	{
		//throw std::exception("BootRom is Invalid !");
	}
};

size_t BootRom::Size() const
{
	return 258;
};

uint8_t BootRom::Read(uint16_t address) const
{
	if (address < 258) {
		return m_bytes[address];
	}
	return 0;
};

void BootRom::Write(uint16_t address, uint8_t value)
{
};