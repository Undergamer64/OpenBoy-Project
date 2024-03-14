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
};


CPU& CPU::operator+=(InstrFamilyPtr&& f)
{
	m_alu.m_instructionFamilies.push_back(std::move(f));
	return *this;
}

void CPU::Execute()
{
	uint8_t opcode = m_mmu.Read(m_registers.PC++);

	for (auto& f : m_alu.m_instructionFamilies)
	{
		if (f->IsValid(opcode)) 
		{
			f->Execute(opcode, m_mmu, m_registers);
			break;
		}
	}
};

CPU& CPU::operator()() 
{
	Execute();
	return *this;
}

#pragma endregion