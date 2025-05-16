
#include "alu.h"
#include "cpu.h"
#include "mmu.h"


#define PCREAD8() ([&]() { \
	currentCycles += 4; \
	return mmu.Read(registers.PC++); })()

#define MMUREAD8(REG) ([&]() { \
	currentCycles += 4; \
	return mmu.Read(registers.REG); })()

#define READ16() ([&]() {\
	uint8_t n = PCREAD8();\
	uint16_t nn = (PCREAD8());\
	return n + (nn << 8); })()

#define MMUWRITE8(ADDR, VAL) {\
	currentCycles += 4; \
	mmu.Write(ADDR, VAL); }

#define WRITE16(REG, VAL) {\
	registers.REG = VAL; }

ALU::~ALU() 
{
}

#pragma region Load Instructions

#pragma region IF_LD_r16_imm16

bool IF_LD_r16_imm16::IsValid(uint8_t opcode)
{
	return (opcode & 0b11001111) == 0b00000001;
}

int IF_LD_r16_imm16::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "LOAD" << std::endl;
	int currentCycles = 0;

	switch (opcode & 0b00110000) {
		case 0b00000000:
			WRITE16(BC, READ16())
			break;
		case 0b00010000:
			WRITE16(DE, READ16())
			break;
		case 0b00100000:
			WRITE16(HL, READ16())
			break;
	case 0b00110000:
			WRITE16(SP, READ16())
			break;
	}

	return currentCycles;
}

#pragma endregion

#pragma region IF_LD_r8_imm8

bool IF_LD_r8_imm8::IsValid(uint8_t opcode) 
{
	return (opcode & 0b11000111) == 0b00000110;
}

int IF_LD_r8_imm8::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "LOAD" << std::endl;
	int currentCycles = 0;

	if ((opcode & 0b00111000) == 0b0011000) 
	{
		MMUWRITE8(registers.HL, PCREAD8());
	}
	else if ((opcode & 0b00111000) == 0b00111000)
	{
		registers.m_registers[6] = PCREAD8();
	}
	else
	{
		registers.m_registers[(opcode & 0b00111000) >> 3] = PCREAD8();
	}
	
	return currentCycles;
}

#pragma endregion

#pragma region IF_LD_rA_memory

bool IF_LD_rA_memory::IsValid(uint8_t opcode) 
{
	return (opcode & 0b11100111) == 0b00000010;
}

int IF_LD_rA_memory::Execute(uint8_t opcode, MMU& mmu, Registers& registers) 
{
	//std::cout << "LOAD" << std::endl;
	int currentCycles = 0;

	switch (opcode & 0b00011000) 
	{
		case 0b00000000:
			MMUWRITE8(registers.BC, registers.m_registers[6]);
			break;
		case 0b00001000:
			MMUWRITE8(registers.DE, registers.m_registers[6]);
			break;
		case 0b00010000:
			registers.m_registers[6] = MMUREAD8(BC);
			break;
		case 0b00011000:
			registers.m_registers[6] = MMUREAD8(DE);
			//std::cout << mmu.Read(registers.DE) << std::endl;
			break;
	}

	return currentCycles;
}

#pragma endregion

#pragma region IF_LD_rA_memory

bool IF_LD_rA_rHL::IsValid(uint8_t opcode)
{
	return (opcode & 0b11100111) == 0b00100010;
}

int IF_LD_rA_rHL::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "LOAD" << std::endl;
	int currentCycles = 0;

	switch (opcode & 0b00011000) 
	{
		case 0b00000000:
			//std::cout << static_cast<int>(registers.HL) << std::endl;
			registers.HL++;
			MMUWRITE8(registers.HL, registers.m_registers[6]);
			break;
		case 0b00001000:
			registers.HL++;
			registers.m_registers[6] = MMUREAD8(HL);
			break;
		case 0b00010000:
			//std::cout << static_cast<int>(registers.HL) << std::endl;
			registers.HL--;
			MMUWRITE8(registers.HL, registers.m_registers[6]);
			break;
		case 0b00011000:
			registers.HL--;
			registers.m_registers[6] = MMUREAD8(HL);
			break;
	}

	return currentCycles;
}

#pragma endregion

#pragma region IF_LD_r_r

bool IF_LD_r_r::IsValid(uint8_t opcode)
{
	return (opcode & 0b11000000) == 0b01000000;
}

int IF_LD_r_r::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "LOAD" << std::endl;
	int currentCycles = 4;

	uint8_t _r_num1;	
	
	if ((opcode & 0b00000111) == 0b00000110)
	{
		_r_num1 = MMUREAD8(HL);
	}
	else if ((opcode & 0b00000111) == 0b00000111)
	{
		_r_num1 = registers.m_registers[6];
	}
	else
	{
		_r_num1 = registers.m_registers[(opcode & 0b00000111)];
	}

	if ((opcode & 0b00111000) == 0b00110000)
	{
		MMUWRITE8(registers.HL, _r_num1);
	}
	else if ((opcode & 0b00111000) == 0b00111000)
	{
		registers.m_registers[6] = _r_num1;
	}
	else
	{
		registers.m_registers[(opcode & 0b00111000)>>3] = _r_num1;
	}

	return currentCycles;
}

#pragma endregion

#pragma region IF_LD_SP_HL

bool IF_LD_SP_HL::IsValid(uint8_t opcode)
{
	return opcode == 0b10011111;
}

int IF_LD_SP_HL::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "LOAD" << std::endl;
	int currentCycles = 8;

	registers.SP = registers.HL;

	return currentCycles;
}

#pragma endregion

#pragma region IF_LD_ADR_r

bool IF_LD_ADR_r::IsValid(uint8_t opcode)
{
	return (opcode & 0b11100111) == 0b11100010;
}

int IF_LD_ADR_r::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "LOAD" << std::endl;
	int currentCycles = 8;

	uint16_t address;

	switch (opcode & 0b00011000)
	{
	case 0b00000000:
		mmu.Write(0xFF00 + registers.m_registers[1], registers.m_registers[6]);
		break;
	case 0b00001000:
		address = READ16();
		mmu.Write(address, registers.m_registers[6]);
		break;
	case 0b00010000:
		registers.m_registers[6] = mmu.Read(0xFF00 + registers.m_registers[1]);
		break;
	case 0b00011000:
		address = READ16();
		registers.m_registers[6] = mmu.Read(address);
		break;
	}	

	return currentCycles;
}

#pragma endregion

#pragma region IF_LD_ADRIMM_r

bool IF_LD_ADRIMM_r::IsValid(uint8_t opcode)
{
	return (opcode & 0b11101111) == 0b11100000;
}

int IF_LD_ADRIMM_r::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "LOAD" << std::endl;
	int currentCycles = 8;

	uint8_t address = PCREAD8();

	switch (opcode & 0b00010000)
	{
	case 0b00000000:
		mmu.Write(0xFF00 + address, registers.m_registers[6]);
		break;
	case 0b00010000:
		registers.m_registers[6] = mmu.Read(0xFF00 + address);
		break;
	}	

	return currentCycles;
}

#pragma endregion

#pragma endregion

#pragma region Arithmetique Instruction

bool IF_AR::IsValid(uint8_t opcode) 
{
	return ((opcode & 0b11100000) == 0b10000000 || (opcode & 0b11100000) == 0b10100000) || (opcode & 0b11000111) == 0b11000110;
}

int IF_AR::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "ARR" << std::endl;
	int currentCycles = 0;

	uint8_t _r_num = registers.m_registers[(opcode & 0b00000111)];
	if ((opcode & 0b00000111) == 0b00000110) {
		if ((opcode & 0b01000000) == 0b01000000)
		{
			_r_num = PCREAD8();
		}
		else
		{
			_r_num = MMUREAD8(HL);
		}
	}
	else if ((opcode & 0b00000111) == 0b0111)
	{
		_r_num = registers.m_registers[6];
	}
	uint8_t _res = 0;

	switch (opcode & 0b00100000)
	{
	case 0b00000000:
#pragma region Arithmetique
		switch (opcode & 0b00011000)
		{
		case 0b00000000:
			_res = registers.m_registers[6] + _r_num + (registers.m_registers[7] & 0b00000001);
#pragma region Flags
#pragma region Flag_S
			registers.m_registers[7] &= ~0b10000000; //Flag s (negatif)
#pragma endregion
#pragma region Flag_Z
			if (_res == 0) //Flag Z (zero)
			{
				registers.m_registers[7] |= 0b01000000;
			}
			else
			{
				registers.m_registers[7] &= ~0b01000000;
			}
#pragma endregion
#pragma region Flag_C
			if (_r_num > _res - (registers.m_registers[7] & 0b00000001) || registers.m_registers[6] > _res - (registers.m_registers[7] & 0b00000001)) //c (Carry for the 7 bit) ONLY FOR ADDS, NOT FOR SUBS !!!
			{
				registers.m_registers[7] |= 0b00000001;
			}
			else
			{
				registers.m_registers[7] &= ~0b00000001;
			}
#pragma endregion
#pragma region Flag_H
			if ((_r_num & 0b00001111) > ((_res - (registers.m_registers[7] & 0b00000001)) & 0b00001111) || (registers.m_registers[6] & 0b00001111) > ((_res - (registers.m_registers[7] & 0b00000001)) & 0b00001111)) //Flag h (half-carry) same method as Flag C but with mask
			{
				registers.m_registers[7] |= 0b00010000;
			}
			else
			{
				registers.m_registers[7] &= ~0b00010000;
			}
#pragma endregion
#pragma endregion
			break;
		case 0b00001000:
			_res = registers.m_registers[6] + _r_num;
#pragma region Flags
#pragma region Flag_S
			registers.m_registers[7] &= ~0b10000000; //Flag s (negatif)
#pragma endregion
#pragma region Flag_Z
			if (_res == 0) //Flag Z (zero)
			{
				registers.m_registers[7] |= 0b01000000;
			}
			else
			{
				registers.m_registers[7] &= ~0b01000000;
			}
#pragma endregion
#pragma region Flag_C
			if (_r_num > _res - (registers.m_registers[7] & 0b00000001) || registers.m_registers[6] > _res - (registers.m_registers[7] & 0b00000001)) //c (Carry for the 7 bit) ONLY FOR ADDS, NOT FOR SUBS !!!
			{
				registers.m_registers[7] |= 0b00000001;
			}
			else
			{
				registers.m_registers[7] &= ~0b00000001;
			}
#pragma endregion
#pragma region Flag_H
			if ((_r_num & 0b00001111) > ((_res - (registers.m_registers[7] & 0b00000001)) & 0b00001111) || (registers.m_registers[6] & 0b00001111) > ((_res - (registers.m_registers[7] & 0b00000001)) & 0b00001111)) //Flag h (half-carry) same method as Flag C but with mask
			{
				registers.m_registers[7] |= 0b00010000;
			}
			else
			{
				registers.m_registers[7] &= ~0b00010000;
			}
#pragma endregion
#pragma endregion
			break;
		case 0b00010000:
			_res = (registers.m_registers[6] - _r_num) - (registers.m_registers[7] & 0b00000001);
#pragma region Negatif_Flags
#pragma region Flag_S
			registers.m_registers[7] |= 0b10000000; //Flag s (negatif)
#pragma endregion
#pragma region Flag_Z
			if (_res == 0) //Flag Z (zero)
			{
				registers.m_registers[7] |= 0b01000000;
			}
			else
			{
				registers.m_registers[7] &= ~0b01000000;
			}
#pragma endregion
#pragma region Flag_C
			if (_r_num < _res + (registers.m_registers[7] & 0b00000001) || registers.m_registers[6] < _res + (registers.m_registers[7] & 0b00000001)) //Flag p/v (overflow) + c (Borrow for the 7 bit) ONLY FOR SUBS, NOT FOR ADDS !!!
			{
				registers.m_registers[7] |= 0b10000000;
				registers.m_registers[7] |= 0b00000001;
			}
			else
			{
				registers.m_registers[7] &= ~0b10000000;
				registers.m_registers[7] &= ~0b00000001;
			}
#pragma endregion
#pragma region Flag_H
			if ((_r_num & 0b00001111) < ((_res + (registers.m_registers[7] & 0b00000001)) & 0b00001111) || (registers.m_registers[6] & 0b00001111) < ((_res + (registers.m_registers[7] & 0b00000001)) & 0b00001111)) //Flag h (half-borrow) same method as Flag p/v but with mask
			{
				registers.m_registers[7] |= 0b00010000;
			}
			else
			{
				registers.m_registers[7] &= ~0b00010000;
			}
#pragma endregion
#pragma endregion
			break;
		case 0b00011000:
			_res = registers.m_registers[6] - _r_num;
#pragma region Negatif_Flags
#pragma region Flag_S
			registers.m_registers[7] |= 0b10000000; //Flag s (negatif)
#pragma endregion
#pragma region Flag_Z
			if (_res == 0) //Flag Z (zero)
			{
				registers.m_registers[7] |= 0b01000000;
			}
			else
			{
				registers.m_registers[7] &= ~0b01000000;
			}
#pragma endregion
#pragma region Flag_C
			if (_r_num < _res + (registers.m_registers[7] & 0b00000001) || registers.m_registers[6] < _res + (registers.m_registers[7] & 0b00000001)) //Flag p/v (overflow) + c (Borrow for the 7 bit) ONLY FOR SUBS, NOT FOR ADDS !!!
			{
				registers.m_registers[7] |= 0b10000000;
				registers.m_registers[7] |= 0b00000001;
			}
			else
			{
				registers.m_registers[7] &= ~0b10000000;
				registers.m_registers[7] &= ~0b00000001;
			}
#pragma endregion
#pragma region Flag_H
			if ((_r_num & 0b00001111) < ((_res + (registers.m_registers[7] & 0b00000001)) & 0b00001111) || (registers.m_registers[6] & 0b00001111) < ((_res + (registers.m_registers[7] & 0b00000001)) & 0b00001111)) //Flag h (half-borrow) same method as Flag p/v but with mask
			{
				registers.m_registers[7] |= 0b00010000;
			}
			else
			{
				registers.m_registers[7] &= ~0b00010000;
			}
#pragma endregion
#pragma endregion
			break;
		}

#pragma endregion
	case 0b00100000:
#pragma region Condition
		switch (opcode & 0b00011000)
		{
		case 0b00000000: //AND
			_res = registers.m_registers[6] & _r_num;
#pragma region Flags
#pragma region Flag_S
			registers.m_registers[7] &= ~0b10000000; //Flag s (negatif)
#pragma endregion
#pragma region Flag_Z
			if (_res == 0) //Flag Z (zero)
			{
				registers.m_registers[7] |= 0b01000000;
			}
			else
			{
				registers.m_registers[7] &= ~0b01000000;
			}
#pragma endregion
#pragma region Flag_C
			registers.m_registers[7] &= ~0b00000001;
#pragma endregion
#pragma region Flag_H
			registers.m_registers[7] |= 0b00010000;
#pragma endregion
#pragma endregion
			registers.m_registers[6] = _res;
			break;
		case 0b00001000: //XOR
			_res = registers.m_registers[6] ^ _r_num;
#pragma region Flags
#pragma region Flag_S
			registers.m_registers[7] &= ~0b10000000; //Flag s (negatif)
#pragma endregion
#pragma region Flag_Z
			if (_res == 0) //Flag Z (zero)
			{
				registers.m_registers[7] |= 0b01000000;
			}
			else
			{
				registers.m_registers[7] &= ~0b01000000;
			}
#pragma endregion
#pragma region Flag_C
			registers.m_registers[7] &= ~0b00000001;
#pragma endregion
#pragma region Flag_H
			registers.m_registers[7] &= ~0b00010000;
#pragma endregion
#pragma endregion
			registers.m_registers[6] = _res;
			break;
		case 0b00010000: //OR
			_res = registers.m_registers[6] | _r_num;
#pragma region Flags
#pragma region Flag_S
			registers.m_registers[7] &= ~0b10000000; //Flag s (negatif)
#pragma endregion
#pragma region Flag_Z
			if (_res == 0) //Flag Z (zero)
			{
				registers.m_registers[7] |= 0b01000000;
			}
			else
			{
				registers.m_registers[7] &= ~0b01000000;
			}
#pragma endregion
#pragma region Flag_C
			registers.m_registers[7] &= ~0b00000001;
#pragma endregion
#pragma region Flag_H
			registers.m_registers[7] &= ~0b00010000;
#pragma endregion
#pragma endregion
			registers.m_registers[6] = _res;
			break;
		case 0b00011000: //CP
			_res = (registers.m_registers[6] - _r_num) - (registers.m_registers[7] & 0b00000001);
			//std::cout << "CP : " << static_cast<int>(_res) << std::endl;
#pragma region Negatif_Flags
#pragma region Flag_S
			registers.m_registers[7] |= 0b10000000; //Flag s (negatif)
#pragma endregion
#pragma region Flag_Z
			if (_res == 0) //Flag Z (zero)
			{
				registers.m_registers[7] |= 0b01000000;
			}
			else
			{
				registers.m_registers[7] &= ~0b01000000;
			}
#pragma endregion
#pragma region Flag_C
			if (_r_num < _res + (registers.m_registers[7] & 0b00000001) || registers.m_registers[6] < _res + (registers.m_registers[7] & 0b00000001)) //Flag C (Borrow for the 7 bit) ONLY FOR SUBS, NOT FOR ADDS !!!
			{
				registers.m_registers[7] |= 0b00000001;
			}
			else
			{
				registers.m_registers[7] &= ~0b00000001;
			}
#pragma endregion
#pragma region Flag_H
			if ((_r_num & 0b00001111) < ((_res + (registers.m_registers[7] & 0b00000001)) & 0b00001111) || (registers.m_registers[6] & 0b00001111) < ((_res + (registers.m_registers[7] & 0b00000001)) & 0b00001111)) //Flag h (half-borrow) same method as Flag p/v but with mask
			{
				registers.m_registers[7] |= 0b00010000;
			}
			else
			{
				registers.m_registers[7] &= ~0b00010000;
			}
#pragma endregion
#pragma endregion
			break;
		}
		break;
#pragma endregion
	}
	if ((opcode & 0b00111000) != 0b00111000) {
		registers.m_registers[6] = _res;
	} 

	return currentCycles;
}

#pragma endregion

#pragma region Flow Instructions

#pragma region IF_FLOW_JR

bool IF_FLOW_JR::IsValid(uint8_t opcode)
{
	return (opcode & 0b11111111) == 0b00011000 || (opcode & 0b11111111) == 0b00010000 || (opcode & 0b11100111) == 0b00100000;
}

int IF_FLOW_JR::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "JUMP R" << std::endl;
	int currentCycles = 0;

	int8_t e = PCREAD8();
	if ((opcode & 0b11100111) == 0b00100000)//JR with cc (if cc = true)
	{
		switch (opcode & 0b00011000)
		{
		case 0b00000000:
			if ((registers.m_registers[7] & 0b01000000) == 0b00000000)
			{
				registers.PC += e;
				currentCycles += 4;
			}
			break;
		case 0b00001000:
			if ((registers.m_registers[7] & 0b01000000) == 0b01000000)
			{
				registers.PC += e;
				currentCycles += 4;
			}
			break;
		case 0b00010000:
			if ((registers.m_registers[7] & 0b00000001) == 0b00000000)
			{
				registers.PC += e;
				currentCycles += 4;
			}
			break;
		case 0b00011000:
			if ((registers.m_registers[7] & 0b00000001) == 0b00000001)
			{
				registers.PC += e;
				currentCycles += 4;
			}
			break;
		}
	}
	else if ((opcode & 0b11111111) == 0b00010000)//JR with B (if B = 0) + B -= 1
	{
		if (registers.m_registers[0] != 0)
		{
			registers.PC -= e;
			currentCycles += 4;
		}
	}
	else if ((opcode & 0b11111111) == 0b00011000)//Always JR
	{
		registers.PC -= e;
		currentCycles += 4;
	}

	return currentCycles;
}

#pragma endregion

#pragma region IF_FLOW_JP

bool IF_FLOW_JP::IsValid(uint8_t opcode)
{
	return (opcode & 0b11111111) == 0b11000011 || (opcode & 0b11111111) == 0b11101001 || (opcode & 0b11100111) == 0b11000010;
}

int IF_FLOW_JP::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "JUMP P" << std::endl;
	int currentCycles = 0;

	if ((opcode & 0b11111111) == 0b11101001)
	{
		registers.PC = registers.HL;
	}
	else 
	{
		uint16_t _nn = PCREAD8();
		if ((opcode & 0b11111111) == 0b11000011)
		{
			registers.PC = _nn;
			currentCycles += 4;
		}
		else if ((opcode & 0b11100111) == 0b11000010)
		{
			switch (opcode & 0b00011000)
			{
			case 0b00000000:
				if ((registers.m_registers[7] & 0b01000000) != 0b01000000)
				{
					registers.PC = _nn;
					currentCycles += 4;
				}
				break;
			case 0b00001000:
				if ((registers.m_registers[7] & 0b01000000) == 0b01000000)
				{
					registers.PC = _nn;
					currentCycles += 4;
				}
				break;
			case 0b00010000:
				if ((registers.m_registers[7] & 0b00000001) != 0b00000001)
				{
					registers.PC = _nn;
					currentCycles += 4;
				}
				break;
			case 0b00011000:
				if ((registers.m_registers[7] & 0b00000001) == 0b00000001)
				{
					registers.PC = _nn;
					currentCycles += 4;
				}
				break;
			}
		}
	}
	return currentCycles;
}

#pragma endregion

#pragma region IF_FLOW_CALL

bool IF_FLOW_CALL::IsValid(uint8_t opcode)
{
	return (opcode & 0b11111111) == 0b11001101 || (opcode & 0b11000111) == 0b11000100;
}

int IF_FLOW_CALL::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "CALL" << std::endl;
	
	int currentCycles = 0;
		
	uint16_t _nn = READ16();

	//std::cout << static_cast<int>(_nn) << std::endl;
	
	bool condition = false;
	
	if (opcode == 0b11001101)
	{
		condition = true;
	}
	else
	{
		switch (opcode & 0b00111000)
		{
		case 0b00000000:
			if ((registers.m_registers[7] & 0b01000000) != 0b01000000)
			{
				condition = true;
			}
			break;
		case 0b00001000:
			if ((registers.m_registers[7] & 0b01000000) == 0b01000000)
			{
				condition = true;
			}
			break;
		case 0b00010000:
			if ((registers.m_registers[7] & 0b00000001) != 0b00000001)
			{
				condition = true;
			}
			break;
		case 0b00011000:
			if ((registers.m_registers[7] & 0b00000001) == 0b00000001)
			{
				condition = true;
			}
			break;
		case 0b00100000:
			if ((registers.m_registers[7] & 0b00000100) != 0b00000100)
			{
				condition = true;
			}
			break;
		case 0b00101000:
			if ((registers.m_registers[7] & 0b00000100) == 0b00000100)
			{
				condition = true;
			}
			break;
		case 0b00110000:
			if ((registers.m_registers[7] & 0b10000000) != 0b10000000)
			{
				condition = true;
			}
			break;
		case 0b00111000:
			if ((registers.m_registers[7] & 0b10000000) == 0b10000000)
			{
				condition = true;
			}
			break;
		}
	}
	if (condition)
	{
		MMUWRITE8(registers.SP - 1, registers.PC >> 8)
		MMUWRITE8(registers.SP - 2, registers.PC)
		registers.SP -= 2;
		//std::cout << registers.SP << std::endl;
		currentCycles += 4;
	}
	registers.PC = _nn;
	return currentCycles;
}

#pragma endregion

#pragma region IF_FLOW_RET

bool IF_FLOW_RET::IsValid(uint8_t opcode) 
{
	return (opcode & 0b11111111) == 0b11001001 || (opcode & 0b11100111) == 0b11000000;
}

int IF_FLOW_RET::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "RET" << std::endl;
	int currentCycles = 8;

	bool condition = false;
	if ((opcode & 0b11111111) == 0b11001001)
	{
		condition = true;
	}
	else
	{
		currentCycles += 4;
		switch (opcode & 0b00011000)
		{
		case 0b00000000:
			if ((registers.m_registers[7] & 0b01000000) != 0b01000000)
			{
				condition = true;
			}
			break;
		case 0b00001000:
			if ((registers.m_registers[7] & 0b01000000) == 0b01000000)
			{
				condition = true;
			}
			break;
		case 0b00010000:
			if ((registers.m_registers[7] & 0b00000001) != 0b00000001)
			{
				condition = true;
			}
			break;
		case 0b00011000:
			if ((registers.m_registers[7] & 0b00000001) == 0b00000001)
			{
				condition = true;
			}
			break;
		}
	}
	if (condition)
	{
		registers.PC = MMUREAD8(SP++) + (MMUREAD8(SP++) << 8);
		currentCycles += 4;
	}
	else
	{
		registers.SP += 2;
	}
	return currentCycles;
}

#pragma endregion

#pragma endregion

#pragma region Rotate Instruction

bool IF_ROTATE::IsValid(uint8_t opcode) 
{
	return (opcode & 0b11100111) == 0b00000111;
}

int IF_ROTATE::Execute(uint8_t opcode, MMU& mmu, Registers& registers) 
{
	//std::cout << "ROT" << std::endl;
	int currentCycles = 4;

	switch (opcode & 0b00001000) 
	{
	case 0b00000000: // Left
		if ((opcode & 0b00010000) == 0b00010000) 
		{
			uint8_t _temp_carry = (registers.m_registers[7] & 0b00000001);
			registers.m_registers[7] &= 0b11111110;
			registers.m_registers[7] |= (registers.m_registers[6] & 0b10000000) >> 7; //Flags C
			registers.m_registers[6] = (registers.m_registers[6] << 1) + (_temp_carry);
		}
		else 
		{
			registers.m_registers[7] &= 0b11111110;
			registers.m_registers[7] |= (registers.m_registers[6] & 0b10000000) >> 7; //Flags C
			registers.m_registers[6] = (registers.m_registers[6] << 1) + ((registers.m_registers[6] & 0b10000000) >> 7);
		}
		break;
	case 0b00001000: // Right
		if ((opcode & 0b00010000) == 0b00010000)
		{
			uint8_t _temp_carry = (registers.m_registers[7] & 0b00000001);
			registers.m_registers[7] &= 0b11111110;
			registers.m_registers[7] |= (registers.m_registers[6] & 0b00000001); //Flags C
			registers.m_registers[6] = (registers.m_registers[6] >> 1) + (_temp_carry << 7);

		}
		else
		{
			registers.m_registers[7] &= 0b11111110;
			registers.m_registers[7] |= (registers.m_registers[6] & 0b00000001); //Flags C
			registers.m_registers[6] = (registers.m_registers[6] >> 1) + ((registers.m_registers[6] & 0b00000001) << 7);
		}
		break;
	}
#pragma region Flags

	registers.m_registers[7] &= ~0b10000000; //Flag S reset

	registers.m_registers[7] &= ~0b01000000; //Flag Z reset

	registers.m_registers[7] &= ~0b00010000; //Flag H reset

#pragma endregion
	return currentCycles;
}

#pragma endregion

#pragma region CB Prefix Instructions

bool IF_CB_Prefix::IsValid(uint8_t opcode)
{
	return opcode == 0xCB;
}

int IF_CB_Prefix::Execute(uint8_t _, MMU& mmu, Registers& registers)
{
	//std::cout << "CB" << std::endl;
	int currentCycles = 4;

	uint8_t _opcode = PCREAD8();
	if ((_opcode & 0b11000000) == 0b00000000) //Rotate
	{
		if ((_opcode & 0b00000111) != 0b0110) 
		{
			uint8_t* _r_num; 
			if ((_opcode & 0b00000111) == 0b0111)
			{
				_r_num = &registers.m_registers[0b00000110];
			}
			else
			{
				_r_num = &registers.m_registers[(_opcode & 0b00000111)];
			}
			if ((_opcode & 0b00001000) == 0b00000000)// Left
			{
				if ((_opcode & 0b00010000) == 0b00010000)
				{
					uint8_t _temp_carry = (registers.m_registers[7] & 0b00000001);
					registers.m_registers[7] &= ~0b00000001;
					registers.m_registers[7] |= (*_r_num & 0b10000000) >> 7; //Flag C
					*_r_num = *_r_num << 1;
					if ((_opcode & 0b00100000) == 0b00000000)
					{
						*_r_num += (_temp_carry);
					}
				}
				else
				{
					registers.m_registers[7] &= ~0b00000001;
					registers.m_registers[7] |= (*_r_num & 0b10000000) >> 7; //Flag C
					*_r_num = (*_r_num << 1);
					if ((_opcode & 0b00100000) == 0b00000000)
					{
						*_r_num += ((*_r_num & 0b10000000) >> 7);
					}
				}
			}
			else if ((_opcode & 0b00001000) == 0b00001000)// Right
			{
				if ((_opcode & 0b00010000) == 0b00010000)
				{
					uint8_t _temp_carry = (registers.m_registers[7] & 0b00000001);
					registers.m_registers[7] &= ~0b00000001;
					registers.m_registers[7] |= (*_r_num & 0b00000001); //Flag C
					*_r_num = (*_r_num >> 1);
					if ((_opcode & 0b00100000) == 0b00000000)
					{
						*_r_num += (_temp_carry << 7);
					}
				}
				else
				{
					registers.m_registers[7] &= ~0b00000001;
					registers.m_registers[7] |= (*_r_num & 0b00000001); //Flag C
					*_r_num = (*_r_num >> 1);
					if ((_opcode & 0b00100000) == 0b00000000)
					{
						*_r_num += ((*_r_num & 0b00000001) << 7);
					}
				}
			}
#pragma region Flags

			registers.m_registers[7] &= ~0b10000000; //Flag s (negatif)

			if (_r_num == 0) //Flag Z (zero)
			{
				registers.m_registers[7] |= 0b01000000;
			}
			else
			{
				registers.m_registers[7] &= ~0b01000000;
			}

			registers.m_registers[7] &= ~0b00010000; //Flag H reset
#pragma endregion
		}
		else 
		{
			currentCycles += 8;
			if ((_opcode & 0b00001000) == 0b00000000)// Left
			{
				if ((_opcode & 0b00010000) == 0b00010000)
				{
					uint8_t _temp_carry = (registers.m_registers[7] & 0b00000001);
					mmu.Write(registers.HL, mmu.Read(registers.HL) & 0b11111110);
					registers.m_registers[7] |= (mmu.Read(registers.HL) & 0b10000000) >> 7; //Flag C
					mmu.Write(registers.HL, (mmu.Read(registers.HL) << 1));
					if ((_opcode & 0b00100000) == 0b00000000)
					{
						mmu.Write(registers.HL, mmu.Read(registers.HL) + (_temp_carry));
					}
				}
				else
				{
					registers.m_registers[7] &= 0b11111110;
					registers.m_registers[7] |= (mmu.Read(registers.HL) & 0b10000000) >> 7; //Flag C
					mmu.Write(registers.HL, (mmu.Read(registers.HL) << 1));
					if ((_opcode & 0b00100000) == 0b00000000)
					{
						mmu.Write(registers.HL, mmu.Read(registers.HL) + ((mmu.Read(registers.HL) & 0b10000000) >> 7));
					}
				}
			}
			else // Right
			{
				if ((_opcode & 0b00010000) == 0b00010000)
				{
					uint8_t _temp_carry = (registers.m_registers[7] & 0b00000001);
					registers.m_registers[7] &= 0b11111110;
					registers.m_registers[7] |= (mmu.Read(registers.HL) & 0b00000001); //Flag C
					mmu.Write(registers.HL, mmu.Read(registers.HL) >> 1);
					if ((_opcode & 0b00100000) == 0b00000000)
					{
						mmu.Write(registers.HL, mmu.Read(registers.HL) + (_temp_carry << 7));
					}
				}
				else
				{
					registers.m_registers[7] &= 0b11111110;
					registers.m_registers[7] |= (mmu.Read(registers.HL) & 0b00000001); //Flag C
					mmu.Write(registers.HL, (mmu.Read(registers.HL) >> 1));
					if ((_opcode & 0b00100000) == 0b00000000)
					{
						mmu.Write(registers.HL, mmu.Read(registers.HL) + ((mmu.Read(registers.HL) & 0b00000001) << 7));
					}
				}
			}
#pragma region Flags

			registers.m_registers[7] &= ~0b10000000; //Flag s (negatif)

			if (mmu.Read(registers.HL) == 0) //Flag Z (zero)
			{
				registers.m_registers[7] |= 0b01000000;
			}
			else
			{
				registers.m_registers[7] &= ~0b01000000;
			}

			registers.m_registers[7] &= ~0b00010000; //Flag H reset
#pragma endregion
		}
	}
	else if ((_opcode & 0b11000000) == 0b01000000) //BIT
	{
		registers.m_registers[7] |= 0b00010000;
		registers.m_registers[7] &= ~0b10000000;

		int _offset_bit = (_opcode & 0b00111000) >> 3;

			
		if ((registers.m_registers[(_opcode & 0b00000111)] & (0b00000001 << _offset_bit)) == 0)
		{
			registers.m_registers[7] |= 0b01000000;
		}
		else
		{
			registers.m_registers[7] &= ~0b01000000;
		}
		if (registers.m_registers[(_opcode & 0b00000111)] == 0b0110) 
		{
			currentCycles += 4;
		}
	}
	else if ((_opcode & 0b11000000) == 0b11000000 || (_opcode & 0b11000000) == 0b10000000) //SET RESET
	{
		int _offset_bit = (_opcode & 0b00111000) >> 3;

		if ((_opcode & 0b01000000) == 0b01000000)
		{
			registers.m_registers[(_opcode & 0b00000111)] |= (0b00000001 << _offset_bit);
		}
		else 
		{
			uint8_t _res = (0b00000001 << _offset_bit);
			registers.m_registers[(_opcode & 0b00000111)] &= !_res;
		}
		if (registers.m_registers[(_opcode & 0b00000111)] == 0b0110)
		{
			currentCycles += 4;
		}
	}
	return currentCycles;
}

#pragma endregion

#pragma region INC/DEC Instruction

bool IF_INC_DEC::IsValid(uint8_t opcode)
{
	return (opcode & 0b11000111) == 0b00000011 || (opcode & 0b11000110) == 0b00000100;
}

int IF_INC_DEC::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "INC/DEC" << std::endl;
	int currentCycles = 4;

	if ((opcode & 0b11000111) == 0b00000011) 
	{
		int D = (opcode & 0b00001000) >> 3;
		int _res;
		if (D == 0)
		{
			_res = 1;
		}
		else if (D == 1)
		{
			_res = -1;
		}
		else
		{
			//throw std::exception("INC/DEC opcode problem !");
			_res = 1;
		}

		switch (opcode & 0b00110000) 
		{
		case 0b00000000:
			{
				uint8_t temp = registers.m_registers[0];
				registers.m_registers[0] = registers.m_registers[1];
				registers.m_registers[1] = temp;
			}
			
			registers.BC += _res;

			{
			uint8_t temp = registers.m_registers[0];
			registers.m_registers[0] = registers.m_registers[1];
			registers.m_registers[1] = temp;
			}
			break;
		case 0b00010000:
			{
				uint8_t temp = registers.m_registers[2];
				registers.m_registers[2] = registers.m_registers[3];
				registers.m_registers[3] = temp;
			}
			
			registers.DE += _res;

			{
			uint8_t temp = registers.m_registers[2];
			registers.m_registers[2] = registers.m_registers[3];
			registers.m_registers[3] = temp;
			}
			break;
		case 0b00100000:
			{
				uint8_t temp = registers.m_registers[4];
				registers.m_registers[4] = registers.m_registers[5];
				registers.m_registers[5] = temp;
			}
			
			registers.HL += _res;

			{
			uint8_t temp = registers.m_registers[4];
			registers.m_registers[4] = registers.m_registers[5];
			registers.m_registers[5] = temp;
			}
			break;
		case 0b00110000:
			//registers.SP += _res;
			std::cout << "SP INC/DEC not implemented ! NIQUE TA MERE !!!!!" << std::endl;
			break;
		}
		return currentCycles;
	}
	if ((opcode & 0b11000110) == 0b00000100) 
	{
		int D = (opcode & 0b00000001);
		int _res;
		if (D == 0)
		{
			_res = 1;
			registers.m_registers[7] &= ~0b10000000;
		}
		else if (D == 1)
		{
			_res = -1;
			registers.m_registers[7] |= 0b10000000;
		}
		else
		{
			//throw std::exception("INC/DEC opcode problem !");
			_res = 1;
		}

		uint8_t _r_num = 0;

		if ((opcode & 0b00111000) != 0b00110000) {
			if ((opcode & 0b00111000) == 0b00111000)
			{
				registers.m_registers[0b00110000 >> 3] += _res;
				_r_num = registers.m_registers[0b00110000 >> 3];
			}
			else
			{
				registers.m_registers[(opcode & 0b00111000) >> 3] += _res;
				_r_num = registers.m_registers[(opcode & 0b00111000) >> 3];
			}
		}
		else 
		{
			mmu.Write(registers.HL, mmu.Read(registers.HL) + _res);
			_r_num = mmu.Read(registers.HL);
			currentCycles += 8;
		}
#pragma region Flags
#pragma region Flag_Z
		if (_r_num == 0) //Flag Z (zero)
		{
			registers.m_registers[7] |= 0b01000000;
		}
		else
		{
			registers.m_registers[7] &= ~0b01000000;
		}
#pragma endregion
#pragma region Flag_H
		if (D == 0) 
		{
			if (((_r_num - _res) & 0b00001111) > (_r_num & 0b00001111)) //Flag h (half-carry) same method as Flag C but with mask
			{
				registers.m_registers[7] |= 0b00010000;
			}
			else
			{
				registers.m_registers[7] &= ~0b00010000;
			}
		}
		else 
		{
			if (((_r_num - _res) & 0b00001111) < (_r_num & 0b00001111)) //Flag h (half-borrow) same method as Flag C but with mask
			{
				registers.m_registers[7] |= 0b00010000;
			}
			else
			{
				registers.m_registers[7] &= ~0b00010000;
			}
		}
#pragma endregion
#pragma endregion
		return currentCycles;
	}
}

#pragma endregion

#pragma region Push/Pop Instruction

bool IF_PUSH_POP::IsValid(uint8_t opcode)
{
	return (opcode & 0b11001011) == 0b11000001;
}

int IF_PUSH_POP::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "PUSH/POP" << std::endl;
	int currentCycles = 12;

	switch (opcode & 0b00110000)
	{
	case 0b00000000:
		if ((opcode & 0b00000100) == 0b00000000) 
		{
			registers.BC = mmu.Read(registers.SP++) + (mmu.Read(registers.SP++) << 8);
		}
		else 
		{
			mmu.Write(--registers.SP, registers.BC >> 8);
			mmu.Write(--registers.SP, registers.BC);
			currentCycles += 4;
		}
		break;
	case 0b00010000:
		if ((opcode & 0b00000100) == 0b00000000)
		{
			registers.DE = mmu.Read(registers.SP++) + (mmu.Read(registers.SP++) << 8);
		}
		else
		{
			mmu.Write(--registers.SP, registers.DE >> 8);
			mmu.Write(--registers.SP, registers.DE);
			currentCycles += 4;
		}
		break;
	case 0b00100000:
		if ((opcode & 0b00000100) == 0b00000000)
		{
			registers.HL = mmu.Read(registers.SP++) + (mmu.Read(registers.SP++) << 8);
		}
		else
		{
			mmu.Write(--registers.SP, registers.HL >> 8);
			mmu.Write(--registers.SP, registers.HL);
			currentCycles += 4;
		}
		break;
	case 0b00110000:
		if ((opcode & 0b00000100) == 0b00000000)
		{
			registers.AF = mmu.Read(registers.SP++) + (mmu.Read(registers.SP++) << 8);
		}
		else
		{
			mmu.Write(--registers.SP, registers.AF >> 8);
			mmu.Write(--registers.SP, registers.AF);
			currentCycles += 4;
		}
		break;
	}
	return currentCycles;
}

#pragma endregion

#pragma region Custom Instruction

bool IF_Finish::IsValid(uint8_t opcode)
{
	return (opcode & 0b11111111) == 0xFD;
}

int IF_Finish::Execute(uint8_t opcode, MMU& mmu, Registers& registers)
{
	int currentCycles = -2;

	std::cout << "End of BootRom";

	return currentCycles;
}

#pragma endregion
