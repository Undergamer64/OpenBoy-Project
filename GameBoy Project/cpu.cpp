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

void CPU::BootDump()
{
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			//std::cout << "Index : " << j + i*16 << " ; ";
			uint8_t opcode = m_mmu.Read(j + (i*16));
			std::cout << /*"Opcode : " <<*/ static_cast<int>(opcode);
			std::cout << " ";
		}
		std::cout << std::endl;
	}
}

int CPU::Execute()
{
	uint8_t opcode = m_mmu.Read(m_registers.PC++);
	std::cout << "Opcode :";
	std::cout << static_cast<int>(opcode) << std::endl;

	if (m_registers.PC > 258) 
	{
		std::cout << static_cast<int>(m_registers.PC) << std::endl;
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
			return f->Execute(opcode, m_mmu, m_registers);
		}
	}
	std::cout << static_cast<int>(opcode) << std::endl;
	return -1;
};

int CPU::operator()() 
{
	return Execute();
}

int CPU::DumpRegisters(bool skip)
{
	if (skip)
	{
		return 1;
	}
	
	for (int i = 0; i < 20; i++)
	{
		std::cout << "--";
	}
	std::cout << std::endl;
	
	std::cout << "Registers : " << std::endl;

	std::cout << std::endl;
	
	for (int i = 0; i < 8; i++)
	{
		std::cout << static_cast<int>(m_registers.m_registers[i]) << " | ";
	}
	
	std::cout << std::endl;
	std::cout << "SP : " << static_cast<int>(m_registers.SP) << " | ";
	std::cout << "PC : " << static_cast<int>(m_registers.PC) << std::endl;
	
	for (int i = 0; i < 20; i++)
	{
		std::cout << "--";
	}
	std::cout << std::endl;

	std::cout << "Next ?" << std::endl;

	char n;
	std::cin >> n;

	if (n == 'c' || n == 'C')
	{
		system("cls");
	}
	else if (n == 'q' || n == 'Q')
	{
		std::cout << "Bye Bye !" << std::endl;
		return -1;
	}
	else if (n == 't')
	{
		return 25;
	}
	else if (n == 'f')
	{
		return 2000;
	}

	std::cout << std::endl;

	return 1;
}

#pragma endregion