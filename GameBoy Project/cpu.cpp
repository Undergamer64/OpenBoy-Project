#include "cpu.h"
#include "mmu.h"

#pragma region CPU

CPU::CPU(MMU& mmu, ALU& alu)
	: m_mmu(mmu)
	, m_alu(alu)
{
	m_registers.PC = 0;
	m_registers.BC = 0;
	m_registers.DE = 0;
	m_registers.HL = 0;
	m_registers.AF = 0;
	m_registers.SP = 0;
};

CPU::~CPU()
{
};


CPU& CPU::operator+=(InstrFamilyPtr&& f)
{
	m_alu.m_instructionFamilies.push_back(std::move(f));
	return *this;
}

int CPU::Execute()
{
	uint8_t opcode = m_mmu.Read(m_registers.PC++);

	for (auto& f : m_alu.m_instructionFamilies)
	{
		if (f == std::nullptr_t()) 
		{
			std::cout << "  Error : Null Pointer For Instruction Family !";
			continue;
		}
		if (f->IsValid(opcode)) 
		{
			return f->Execute(opcode, m_mmu, m_registers);
		}
	}
	return -1;
};

int CPU::operator()() 
{
	return Execute();
}

#pragma endregion