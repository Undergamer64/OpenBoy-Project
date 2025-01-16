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

	if (m_registers.PC > 258) 
	{
		return -2;
	}

	for (auto& f : m_alu.m_instructionFamilies)
	{
		if (f == std::nullptr_t()) 
		{
			std::cout << "  Error : Null Pointer For Instruction Family !";
			continue;
		}
		if (f->IsValid(opcode)) 
		{
			std::cout << opcode << std::endl;
			int cycles = f->Execute(opcode, m_mmu, m_registers);
			std::cout << cycles << std::endl;
			return cycles;
		}
	}
	std::cout << static_cast<int>(opcode) << std::endl;
	std::cout << static_cast<int>(m_registers.PC) << std::endl;
	m_registers.PC--;
	std::cout << static_cast<int>(m_mmu.Read(m_registers.PC)) << std::endl;
	return -1;
};

int CPU::operator()() 
{
	return Execute();
}

#pragma endregion