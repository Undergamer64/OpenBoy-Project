#pragma once
#include "alu.h"
#include "ram.h"
#include <memory>
#include <cstdint>

#include "Cartidge.h"


struct Registers
{
	uint8_t m_registers[8];
	uint16_t& BC = *(uint16_t*)(m_registers);
	uint16_t& DE = *(uint16_t*)(m_registers + 2);
	uint16_t& HL = *(uint16_t*)(m_registers + 4);
	uint16_t& AF = *(uint16_t*)(m_registers + 6);
	uint16_t SP, PC;
	uint8_t IME;
};

class InstructionFamily;

class CPU 
{
	MMU& m_mmu;
	bool m_debugSkip = true;
	Cartidge m_cartidge;
	
	using InstrFamilyPtr = std::unique_ptr<InstructionFamily>;
	ALU& m_alu;

public:
	Registers m_registers;
	int m_TimerCounter = 1024;

	CPU(MMU& mmu, ALU& alu);
	~CPU();

	CPU& operator+=(InstrFamilyPtr&& f);

	void LoadCartridge(const std::string& filepath);
	
	void Write(uint16_t address, uint8_t value);
	uint8_t Read(uint16_t address);
	void Map(MemoryBase* mem, uint16_t address);

	void Push(uint16_t address);

	uint8_t GetCurrentInstruction();

	std::stringstream DumpBoot(bool pointer = false);
	
	int Execute();

	int operator()();

	//int DumpRegisters(bool skip);

	void SetClockFreq(int& TimerCounter);

	void ForceWrite(uint16_t address, uint8_t value);
};