#include "BootRom.h"

#include <iostream>
#include <fstream>

BootRom::BootRom(const std::string& filepath)
{
	std::ifstream ifs(filepath, std::ios::binary);
	if (ifs.good()) 
	{
		ifs.read(reinterpret_cast<char*>(m_bytes.data()), BootRom::Size()-1);
		if (ifs.fail())
		{
			std::cout << std::to_string(ifs.gcount()) << " failure" << std::endl;
		}
		ifs.close();
	}
	else 
	{
		throw std::runtime_error("Failed to open boot rom file: " + filepath);
	}
};

size_t BootRom::Size() const
{
	return 256;
};

uint8_t BootRom::Read(uint16_t address) const
{
	if (address < BootRom::Size()) {
		return m_bytes[address];
	}
	return 0;
};

void BootRom::Write(uint16_t address, uint8_t value)
{
};