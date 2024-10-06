#pragma once

#include "ram.h"

#include <array>
#include <cstdint>
#include <string>

class BootRom
	: public MemoryBase
{
	std::array<uint8_t, 258> m_bytes;
public:
	BootRom(const std::string& filepath);

	size_t  Size() const override;
	uint8_t Read(uint16_t address) const override;
	void    Write(uint16_t address, uint8_t value) override;
};