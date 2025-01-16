#pragma once

#include <cstdint>
#include <array>

class MemoryBase
{
public:
	virtual size_t Size() const = 0;
	virtual uint8_t Read(uint16_t address) const = 0;
	virtual void Write(uint16_t address, uint8_t value) = 0;
};

template<size_t SIZE>
class Memory
	: public MemoryBase 
{
	std::array<uint8_t, SIZE> m_bytes;

public:
	size_t Size() const override 
	{
		return SIZE;
	}

	uint8_t Read(uint16_t address) const override 
	{
		if (address < SIZE) {
			return m_bytes[address];
		}
		return 0;
	};


	void Write(uint16_t address, uint8_t value) override 
	{
		if (address < SIZE)
		{
			m_bytes[address] = value;
		}
	};
};