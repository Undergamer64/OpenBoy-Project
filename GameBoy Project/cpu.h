#pragma once

#include <cstdint>
#include <vector>

class MMU;

struct Registers
{
	uint8_t m_registers[8];
	uint16_t& AF = *(uint16_t*)(m_registers);
	uint16_t& BC = *(uint16_t*)(m_registers + 2);
	uint16_t& DE = *(uint16_t*)(m_registers + 4);
	uint16_t& HL = *(uint16_t*)(m_registers + 6);
	uint16_t SP, PC;
};

class InstructionFamily
{
public:
	virtual bool IsValid(uint8_t opcode) = 0;
	virtual void Execute(uint8_t opcode, MMU& mmu, Registers& registers) = 0;
};

class CPU 
{
	Registers m_registers;
	MMU& m_mmu;

	std::vector<InstructionFamily*> m_instructionFamilies;

public:
	CPU(MMU& mmu);
	~CPU();
	void Execute();
};