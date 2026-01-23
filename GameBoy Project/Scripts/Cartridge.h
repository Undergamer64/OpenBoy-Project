#pragma once
#include <string>
#include <vector>

#include "./ram.h"

class Cartridge
    : public MemoryBase
{
    std::vector<uint8_t> data;
public:
    Cartridge();
    Cartridge(const std::string& filepath);

    size_t  Size() const override;
    uint8_t Read(uint16_t address) const override;
    void    Write(uint16_t address, uint8_t value) override;
};
