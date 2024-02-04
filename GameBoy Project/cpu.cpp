#include "cpu.h"
#include "mmu.h"

// --------------------------------------------------------------------------------------
// LD instructions

class IF_LD_r16_imm16 final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return (opcode & 0b11001111) == 0b00000001;
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
		switch (opcode & 0b00110000) {
		case 0b00000000:
			registers.BC = mmu.Read(registers.PC++);
			registers.BC += mmu.Read(registers.PC++) << 8;
			break;
		case 0b00010000:
			registers.DE = mmu.Read(registers.PC++);
			registers.DE += mmu.Read(registers.PC++) << 8;
			break;
		case 0b00100000:
			registers.HL = mmu.Read(registers.PC++);
			registers.HL += mmu.Read(registers.PC++) << 8;
			break;
		case 0b00110000:
			registers.SP = mmu.Read(registers.PC++);
			registers.SP += mmu.Read(registers.PC++) << 8;
			break;
		}
	}
};

class IF_LD_r8_imm8 final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return (opcode & 0b11000111) == 0b00000110;
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
		switch (opcode & 0b00111000) {
		case 0b00000000:
			registers.m_registers[2] = mmu.Read(registers.PC++);
			break;
		case 0b00001000:
			registers.m_registers[3] = mmu.Read(registers.PC++);
			break;
		case 0b00010000:
			registers.m_registers[4] = mmu.Read(registers.PC++);
			break;
		case 0b00011000:
			registers.m_registers[5] = mmu.Read(registers.PC++);
			break;
		case 0b00100000:
			registers.m_registers[6] = mmu.Read(registers.PC++);
			break;
		case 0b00101000:
			registers.m_registers[7] = mmu.Read(registers.PC++);
			break;
		case 0b00110000:
			mmu.Write(registers.HL, mmu.Read(registers.PC++));
			break;
		case 0b00111000:
			registers.m_registers[0] = mmu.Read(registers.PC++);
			break;
		}
	}
};

class IF_LD_rA_memory final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return (opcode & 0b11100111) == 0b00000010;
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
		switch (opcode & 0b00011000) {
		case 0b00000000:
			mmu.Write(registers.BC, registers.m_registers[0]);
			break;
		case 0b00001000:
			mmu.Write(registers.DE, registers.m_registers[0]);
			break;
		case 0b00010000:
			registers.m_registers[0] = mmu.Read(registers.BC);
			break;
		case 0b00011000:
			registers.m_registers[0] = mmu.Read(registers.DE);
			break;
		}
	}
};

class IF_LD_r16_immmemory16 final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return (opcode & 0b11100111) == 0b00100010;
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
		switch (opcode & 0b00011000) {
		case 0b00000000:
			mmu.Write((mmu.Read(registers.PC++) + (mmu.Read(registers.PC++) << 8)), registers.HL);
			break;
		case 0b00001000:
			mmu.Write((mmu.Read(registers.PC++) + (mmu.Read(registers.PC++) << 8)), registers.m_registers[0]);
			break;
		case 0b00010000:
			registers.HL = mmu.Read((mmu.Read(registers.PC++) + (mmu.Read(registers.PC++) << 8)));
			break;
		case 0b00011000:
			registers.m_registers[0] = mmu.Read((mmu.Read(registers.PC++) + (mmu.Read(registers.PC++) << 8)));;
			break;
		}
	}
};

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// Arithmetique instructions

class IF_AR_rA_r8 final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return ((opcode & 0b11100000) == 0b10000000 || (opcode & 0b11100000) == 0b10100000);
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
		uint8_t r_num;
		
		switch (opcode & 0b00000111)
		{
		case 0b00000000:
			r_num = registers.m_registers[2];
			break;
		case 0b00000001:
			r_num = registers.m_registers[3];
			break;
		case 0b00000010:
			r_num = registers.m_registers[4];
			break;
		case 0b00000011:
			r_num = registers.m_registers[5];
			break;
		case 0b00000100:
			r_num = registers.m_registers[6];
			break;
		case 0b00000101:
			r_num = registers.m_registers[7];
			break;
		case 0b00000110:
			r_num = mmu.Read(registers.HL);
			break;
		case 0b00000111:
			r_num = registers.m_registers[0];
			break;
		}
		
		switch (opcode & 0b00100000) 
		{
		case 0b00000000:
			uint8_t res;
			switch (opcode & 0b00011000) 
			{
			case 0b00000000:
				res = registers.m_registers[0] + r_num + (registers.m_registers[1] & 0b00000001);
				
				if (res == 0) {
					registers.m_registers[1] |= 0b01000000;
				}
				else {
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[0] = res;
				break;
			case 0b00001000:
				res = registers.m_registers[0] + r_num;

				if (res == 0) {
					registers.m_registers[1] |= 0b01000000;
				}
				else {
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[0] = res;
				break;
			case 0b00010000:
				res = registers.m_registers[0] - r_num - (registers.m_registers[1] & 0b00000001);

				if (res == 0) {
					registers.m_registers[1] |= 0b01000000;
				}
				else {
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[0] = res;
				break;
			case 0b00011000:
				res = registers.m_registers[0] - r_num;

				if (res == 0) {
					registers.m_registers[1] |= 0b01000000;
				}
				else {
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[0] = res;
				break;
			}
			break;


		case 0b00100000:

			uint8_t res;
			switch (opcode & 0b00011000)
			{
			case 0b00000000:
				res = registers.m_registers[0] & r_num;

				if (res == 0) {
					registers.m_registers[1] |= 0b01000000;
				}
				else {
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[0] = res;
				break;
			case 0b00001000:
				res = registers.m_registers[0] ^ r_num;

				if (res == 0) {
					registers.m_registers[1] |= 0b01000000;
				}
				else {
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[0] = res;
				break;
			case 0b00010000:
				res = registers.m_registers[0] | r_num ;

				if (res == 0) {
					registers.m_registers[1] |= 0b01000000;
				}
				else {
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[0] = res;
				break;
			case 0b00011000:
				
				if (res == 0) {
					registers.m_registers[1] |= 0b01000000;
				}
				else {
					registers.m_registers[1] &= !0b01000000;
				}

				break;
			}

			break;
		}
		
	}
};

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------

CPU::CPU(MMU& mmu)
	: m_mmu(mmu)
{
	m_registers.PC = 0;
};

CPU::~CPU()
{
	for (InstructionFamily* f : m_instructionFamilies)
	{
		delete f;
	}
};

void CPU::Execute()
{
	uint8_t opcode = m_mmu.Read(m_registers.PC++);

	for (InstructionFamily* f : m_instructionFamilies)
	{
		if (f->IsValid(opcode)) 
		{
			f->Execute(opcode, m_mmu, m_registers);
			break;
		}
	}
};