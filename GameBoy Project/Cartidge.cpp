#include "Cartidge.h"

#include <fstream>
#include <iostream>

Cartidge::Cartidge() = default;

Cartidge::Cartidge(const std::string& filepath)
{
    std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
    if (ifs.good()) 
    {
        std::streamsize size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        data.resize(size);
        ifs.read(reinterpret_cast<char*>(data.data()), size-1);
        if (ifs.fail())
        {
            std::cout << std::to_string(ifs.gcount()) << " failure" << std::endl;
        }
        ifs.close();
        std::cout << "Successfully loaded cartridge: " << filepath << " (0x" << std::hex << size << " bytes)" << std::endl;
    }
    else 
    {
        data.resize(0x8000);
        for (int i = 0; i < 0x8000; i++)
        {
            data[i] = 0xFF;
        }
        std::cout << "Failed to load cartridge: " << filepath << std::endl;
    }
}

size_t Cartidge::Size() const
{
    return data.size();
}

uint8_t Cartidge::Read(uint16_t address) const
{
    if (address < Cartidge::Size()) {
        return data[address];
    }
    return 0;
}

void Cartidge::Write(uint16_t address, uint8_t value){ }
