#pragma once
#include "alu.h"

#include <cstdint>

struct Registers
{
	uint8_t m_registers[8];
	uint16_t& AF = *(uint16_t*)(m_registers);
	uint16_t& BC = *(uint16_t*)(m_registers + 2);
	uint16_t& DE = *(uint16_t*)(m_registers + 4);
	uint16_t& HL = *(uint16_t*)(m_registers + 6);
	uint16_t SP, PC;
};

class InstructionFamily;

class CPU 
{
	Registers m_registers;
	MMU& m_mmu;
	ALU& m_alu;

public:
	CPU(MMU& mmu, ALU& alu);
	~CPU();
	void Execute();
};