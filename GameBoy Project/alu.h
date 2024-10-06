#pragma once
#include <iostream>
#include <cstdint>
#include <memory>
#include <vector>

class MMU;
struct Registers;

class InstructionFamily
{
public:
	virtual bool IsValid(uint8_t opcode) = 0;
	virtual int Execute(uint8_t opcode, MMU& mmu, Registers& registers) = 0;
};

class ALU 
{
public:
	~ALU();
	std::vector<std::unique_ptr<InstructionFamily>> m_instructionFamilies;
};