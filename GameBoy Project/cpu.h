#pragma once
#include "alu.h"
#include <memory>
#include <cstdint>

struct Registers
{
	uint8_t m_registers[8];
	uint16_t& BC = *(uint16_t*)(m_registers);
	uint16_t& DE = *(uint16_t*)(m_registers + 2);
	uint16_t& HL = *(uint16_t*)(m_registers + 4);
	uint16_t& AF = *(uint16_t*)(m_registers + 6);
	uint16_t SP, PC;
};

class InstructionFamily;

class CPU 
{
	MMU& m_mmu;

	using InstrFamilyPtr = std::unique_ptr<InstructionFamily>;
	ALU& m_alu;

public:
	Registers m_registers;
	CPU(MMU& mmu, ALU& alu);
	~CPU();

	CPU& operator+=(InstrFamilyPtr&& f);

	void DumpBoot();
	
	int Execute();

	int operator()();

	int DumpRegisters(bool skip);
};