#pragma once
#include <cstdint>
#include <vector>

class MMU;
class Registers;

class InstructionFamily
{
public:
	virtual bool IsValid(uint8_t opcode) = 0;
	virtual void Execute(uint8_t opcode, MMU& mmu, Registers& registers) = 0;
};

class ALU 
{
public:
	std::vector<InstructionFamily*> m_instructionFamilies;
};