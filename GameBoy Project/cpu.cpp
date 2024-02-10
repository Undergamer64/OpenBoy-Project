#include "cpu.h"
#include "mmu.h"

#pragma region CPU

CPU::CPU(MMU& mmu, ALU& alu)
	: m_mmu(mmu)
	, m_alu(alu)
{
	m_registers.PC = 0;
};

CPU::~CPU()
{
	for (InstructionFamily* f : m_alu.m_instructionFamilies)
	{
		delete f;
	}
};

void CPU::Execute()
{
	uint8_t opcode = m_mmu.Read(m_registers.PC++);

	for (InstructionFamily* f : m_alu.m_instructionFamilies)
	{
		if (f->IsValid(opcode)) 
		{
			f->Execute(opcode, m_mmu, m_registers);
			break;
		}
	}
};

#pragma endregion