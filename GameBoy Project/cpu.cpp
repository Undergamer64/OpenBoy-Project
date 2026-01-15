#include "cpu.h"

#include <iomanip>
#include <sstream>

#include "mmu.h"

#define CLOCKSPEED 4194304
#define TIMA 0xFF05
#define TMA 0xFF06
#define TMC 0xFF07

CPU::CPU(MMU& mmu, ALU& alu)
	: m_mmu(mmu)
	  , m_alu(alu)
{
	m_registers.PC = 0;
	m_registers.CB = 0;
	m_registers.ED = 0;
	m_registers.LH = 0;
	m_registers.FA = 0;
	m_registers.SP = 0;
	m_registers.IME = 1;

	m_debugAddresses = std::vector<uint16_t>();
};

CPU::~CPU() = default;


CPU& CPU::operator+=(InstrFamilyPtr&& f)
{
	m_alu.m_instructionFamilies.push_back(std::move(f));
	return *this;
}

void CPU::LoadCartridge(const std::string& filepath)
{
    m_mmu.LoadCartridge(filepath);
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
	if (std::find(m_debugAddresses.begin(), m_debugAddresses.end(), m_registers.PC) == m_debugAddresses.end())
	{
		m_debugAddresses.push_back(m_registers.PC);
	}
	uint8_t opcode = GetCurrentInstruction();
	m_registers.PC++;
	
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
	m_registers.PC--;
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

void CPU::Write(uint16_t address, uint8_t value)
{
	//TODO : CONDITIONS

	
	
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

uint8_t CPU::ForceRead(uint16_t address)
{
	return m_mmu.Read(address);
}
