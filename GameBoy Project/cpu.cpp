#include "cpu.h"

#include <iomanip>
#include <sstream>

#include "mmu.h"

#define CLOCKSPEED 4194304
#define TIMA 0xFF05
#define TMA 0xFF06
#define TMC 0xFF07

#pragma region CPU

CPU::CPU(MMU& mmu, ALU& alu)
	: m_mmu(mmu)
	  , m_cartidge(Cartidge())
	  , m_alu(alu)
{
	m_registers.PC = 0;
	m_registers.BC = 0;
	m_registers.DE = 0;
	m_registers.HL = 0;
	m_registers.AF = 0;
	m_registers.SP = 0;
	m_registers.IME = 1;
};

CPU::~CPU() = default;


CPU& CPU::operator+=(InstrFamilyPtr&& f)
{
	m_alu.m_instructionFamilies.push_back(std::move(f));
	return *this;
}

void CPU::LoadCartridge(const std::string& filepath)
{
	std::cout << "Loading Cartridge: " << filepath << std::endl;
	
	m_cartidge = Cartidge(filepath);
	
	std::cout << "Done !" << "\n";
	
	std::cout << "Cartridge size : " << m_cartidge.Size() << std::endl;
	/*
#if _DEBUG
	for (size_t address = 0; address < m_cartidge.Size(); address++)
	{
		if (address > 0x8000)
		{
			break;
		}	
		m_mmu.Write(address, m_cartidge.Read(address));

		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(m_cartidge.Read(address)) << " ";

		if ((address + 1) % 0x0010 == 0)
		{
			std::cout << "\n";
		}
	}
#endif
	*/
	for (int i = 0; i < 20; i++)
	{
		std::cout << "--";
	}
	std::cout << std::endl;
}

std::stringstream CPU::DumpBoot(bool pointer)
{
	std::stringstream ss;
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			if (pointer && (j + i * 16) == m_registers.PC)
			{
				ss << "!";
			}
			else
			{
				ss << "  ";
			}
			//std::cout << "Index : " << j + i*16 << " ; ";
			uint8_t opcode = Read(j + (i*16));
			ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(opcode);
		}
		ss << "\n";
	}
	for (int i = 0; i < 20; i++)
	{
		ss << "--";
	}
	return ss;
}

uint8_t CPU::GetCurrentInstruction()
{
	return Read(m_registers.PC);
}

int CPU::Execute()
{
	uint8_t opcode = GetCurrentInstruction();
	m_registers.PC++;
	
	//std::cout << "Program Counter :" << std::hex << m_registers.PC << " | " << "Opcode : " << std::hex << static_cast<int>(opcode) << std::endl;

	/*std::cout << "At PC: 0x" << std::hex << static_cast<int>(m_registers.PC) 
			  << " got opcode: 0x" << static_cast<int>(opcode) << std::endl;*/

	
	if (m_registers.PC > 258  && Read(0xFF50) == 0) // booting overflow
	{
		std::cout << "PC overflow while booting" << std::endl;
		std::cout << std::hex << static_cast<int>(m_registers.PC) << std::endl;
		return -2;
	}

	for (auto& f : m_alu.m_instructionFamilies)
	{
		if (f == std::nullptr_t()) 
		{
			std::cout << "Error : Null Pointer For Instruction Family !";
			continue;
		}
		if (f->IsValid(opcode)) 
		{
			return f->Execute(opcode, m_mmu, m_registers);
		}
	}
	std::cout << "Error : No Valide Instruction Family For Value 0x"
		<< std::hex
		<< static_cast<int>(opcode)
		<< " At PC 0x"
		<< static_cast<int>(m_registers.PC)
		<< std::endl;
	
	return -1;
};

int CPU::operator()() 
{
	return Execute();
}

int CPU::DumpRegisters(bool skip)
{
	if (skip || (m_debugSkip
		&& (m_registers.PC != 0x51/*
		|| m_registers.m_registers[2] <= 1*/
		|| (m_registers.PC >= 0x95 && m_registers.PC <= 0xA7)
		|| (m_registers.PC >= 0x60 && m_registers.PC <= 0x6e))))
	{
		return 1;
	}
	
	m_debugSkip = false;
	
	for (int i = 0; i < 20; i++)
	{
		std::cout << "--";
	}
	std::cout << std::endl;
	
	std::cout << "Registers : " << std::endl;

	std::cout << std::endl;
	
	for (int i = 0; i < 8; i++)
	{
		std::cout << std::hex << static_cast<int>(m_registers.m_registers[i]) << " | ";
	}
	
	std::cout << std::endl;
	std::cout << "SP : " << std::hex << static_cast<int>(m_registers.SP) << " | ";
	std::cout << "PC : " << std::hex << static_cast<int>(m_registers.PC) << " | ";
	std::cout << "Opcode : " << std::hex << static_cast<int>(Read(m_registers.PC)) << std::endl;
	std::cout << "ScanLine : " << std::dec << static_cast<int>(Read(0xFF44)) << std::endl;
	
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

void CPU::Write(uint16_t address, uint8_t value)
{
	//CONDITIONS

	
	
	m_mmu.Write(address, value);
}

uint8_t CPU::Read(uint16_t address)
{
	if (TMC == address)
	{
		uint8_t currentfreq = m_mmu.GetClockFreq() ;
		uint8_t value = m_mmu.Read(address);
		uint8_t newfreq = m_mmu.GetClockFreq();

		if (currentfreq != newfreq)
		{
			m_mmu.SetClockFreq(m_TimerCounter);
		}
		return value;
	}
	
	return m_mmu.Read(address);
}

void CPU::Map(MemoryBase* mem, uint16_t address)
{
	m_mmu.Map(mem, address);
}

void CPU::Push(uint16_t address)
{
	m_mmu.Write(--m_registers.SP, address >> 8);
	m_mmu.Write(--m_registers.SP, address);
}

void CPU::SetClockFreq(int& TimerCounter)
{
	m_mmu.SetClockFreq(TimerCounter);
}

void CPU::ForceWrite(uint16_t address, uint8_t value)
{
	m_mmu.Write(address, value);
}
