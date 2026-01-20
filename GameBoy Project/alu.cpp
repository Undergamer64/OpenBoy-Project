#include "alu.h"
#include "cpu.h"
#include "mmu.h"

#define PCREAD8() ([&]() { \
	return mmu.Read(registers.PC++); })()

#define MMUREAD8(REG) ([&]() { \
	return (mmu.Read(GETINVERTED16(registers.REG)));})()

#define PCREAD16() ([&]() {\
	uint8_t n = PCREAD8();\
	uint16_t nn = (PCREAD8());\
	return n + (nn << 8); })()

#define MMUWRITE8(ADDR, VAL) {\
	mmu.Write(ADDR, VAL); }

#define WRITE16(REG, VAL) {\
	registers.REG = VAL;}

#define GETINVERTED16(VAL) ( (((VAL) & 0x00FF) << 8) + (((VAL) & 0xFF00) >> 8) )

#define BC GETINVERTED16(registers.CB)
#define DE GETINVERTED16(registers.ED)
#define HL GETINVERTED16(registers.LH)
#define AF GETINVERTED16(registers.FA)

ALU::~ALU() 
{
}

#pragma region Load Instructions

#pragma region IF_LD_r16_imm16

bool IF_LD_r16_imm16::IsValid(uint8_t opcode)
{
	return (opcode & 0b11001111) == 0b00000001;
}

bool IF_LD_r16_imm16::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	switch (m_step)
	{
	case 0: //Fetch wait
		break;
	case 1:
		m_8bitRegister = PCREAD8();
		break;
	case 2:
		m_16bitRegister = m_8bitRegister + (PCREAD8() << 8);
		break;
	case 3:
		switch (opcode & 0b00110000)
		{
		case 0b00000000:
			WRITE16(CB, GETINVERTED16(m_16bitRegister))
			break;
		case 0b00010000:
			WRITE16(ED, GETINVERTED16(m_16bitRegister))
			break;
		case 0b00100000:
			WRITE16(LH, GETINVERTED16(m_16bitRegister))
			break;
		case 0b00110000:
			WRITE16(SP, m_16bitRegister)
			break;
		}
		m_step = 0;
		return true;
	}
	m_step++;
	return false;
}

#pragma endregion

#pragma region IF_LD_r8_imm8

bool IF_LD_r8_imm8::IsValid(uint8_t opcode) 
{
	return (opcode & 0b11000111) == 0b00000110;
}

bool IF_LD_r8_imm8::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	switch (m_step)
	{
	case 0://Fetch wait
		break;
	case 1: // if HL
		if ((opcode & 0b00111000) == 0b0011000) 
		{
			m_8bitRegister = PCREAD8();
			m_step++;
			return false; //Take 1 extra cycle for HL
		}

		if ((opcode & 0b00111000) == 0b00111000)
		{
			registers.m_registers[6] = PCREAD8();
		}
		else
		{
			registers.m_registers[(opcode & 0b00111000) >> 3] = PCREAD8();
		}
		m_step = 0;
		return true;
	case 2: // if HL
		MMUWRITE8(HL, m_8bitRegister);
		m_step = 0;
		return true;
	}
	m_step++;
	return false;
}

#pragma endregion

#pragma region IF_LD_rA_memory

bool IF_LD_rA_memory::IsValid(uint8_t opcode) 
{
	return (opcode & 0b11100111) == 0b00000010;
}

bool IF_LD_rA_memory::Tick(uint8_t opcode, MMU& mmu, Registers& registers) 
{
	switch (m_step)
	{
	case 0: //Fetch wait
		break;
	case 1:
		switch (opcode & 0b00011000) 
		{
		case 0b00000000:
			MMUWRITE8(BC, registers.m_registers[6]);
			break;
		case 0b00001000:
			MMUWRITE8(DE, registers.m_registers[6]);
			break;
		case 0b00010000:
			registers.m_registers[6] = MMUREAD8(CB);
			break;
		case 0b00011000:
			registers.m_registers[6] = MMUREAD8(ED);
			break;
		}
		
		m_step = 0;
		return true;
	}

	m_step++;
	return false;
}

#pragma endregion

#pragma region IF_LD_rA_rHL

bool IF_LD_rA_rHL::IsValid(uint8_t opcode)
{
	return (opcode & 0b11100111) == 0b00100010;
}

bool IF_LD_rA_rHL::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	switch (m_step)
	{
	case 0: //Fetch wait
		break;
	case 1:
		switch (opcode & 0b00011000) 
		{
		case 0b00000000:
			MMUWRITE8(HL, registers.m_registers[6]);
	
			WRITE16(LH, GETINVERTED16(HL + 1)); //inc HL
			break;
		case 0b00001000:
			registers.m_registers[6] = MMUREAD8(LH);
	
			WRITE16(LH, GETINVERTED16(HL + 1)); //inc HL
			break;
		case 0b00010000:
			MMUWRITE8(HL, registers.m_registers[6]);
	
			WRITE16(LH, GETINVERTED16(HL - 1)); //dec HL
			break;
		case 0b00011000:
			registers.m_registers[6] = MMUREAD8(LH);
	
			WRITE16(LH, GETINVERTED16(HL - 1)); //dec HL
			break;
		}
		m_step = 0;
		return true;
	}
	
	m_step++;
	return false;
}

#pragma endregion

#pragma region IF_LD_r_r

bool IF_LD_r_r::IsValid(uint8_t opcode)
{
	return (opcode & 0b11000000) == 0b01000000 && (opcode & 0b00111111) != 0b00110110; // all load r,r except LD (HL), (HL), which is HALT instruction
}

bool IF_LD_r_r::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//note : this instruction only takes 1 M-cycle except when HL is involved (2 M-cycle)

	switch (m_step)
	{
	case 0:
		if ((opcode & 0b00111000) == 0b00110000 || (opcode & 0b00000111) == 0b00000110)
		{
			m_step++;
			return false; //Take 1 extra cycle for HL
		}
	
		if ((opcode & 0b00000111) == 0b00000111)
		{
			m_8bitRegister = registers.m_registers[6];
		}
		else
		{
			m_8bitRegister = registers.m_registers[(opcode & 0b00000111)];
		}

		if ((opcode & 0b00111000) == 0b00111000)
		{
			registers.m_registers[6] = m_8bitRegister;
		}
		else
		{
			registers.m_registers[(opcode & 0b00111000)>>3] = m_8bitRegister;
		}
		
		m_step = 0;
		return true;
	case 1: // if HL involved (as source or dest)
		if ((opcode & 0b00000111) == 0b00000110)
		{
			m_8bitRegister = MMUREAD8(LH);
		}
		else if ((opcode & 0b00000111) == 0b00000111)
		{
			m_8bitRegister = registers.m_registers[6];
		}
		else
		{
			m_8bitRegister = registers.m_registers[(opcode & 0b00000111)];
		}
		
		if ((opcode & 0b00111000) == 0b00110000)
		{
			MMUWRITE8(HL, m_8bitRegister);
		}
		else if ((opcode & 0b00111000) == 0b00111000)
		{
			registers.m_registers[6] = m_8bitRegister;
		}
		else
		{
			registers.m_registers[(opcode & 0b00111000)>>3] = m_8bitRegister;
		}
		
		m_step = 0;
		return true;
	}
	
	m_step++;
	return false;
}

#pragma endregion

#pragma region IF_LD_SP_HL

bool IF_LD_SP_HL::IsValid(uint8_t opcode)
{
	return opcode == 0b11111001;
}

bool IF_LD_SP_HL::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	switch (m_step)
	{
	case 0: //Fetch wait
		break;
	case 1:
		registers.SP = HL;
		m_step = 0;
		return true;
	}
	
	m_step++;
	return false;
}

#pragma endregion

#pragma region IF_LD_ADR_r

bool IF_LD_ADRC_r::IsValid(uint8_t opcode)
{
	return (opcode & 0b11100111) == 0b11100010;
}

bool IF_LD_ADRC_r::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	switch (m_step)
	{
	case 0: //Fetch wait
		break;
	case 1:
		switch (opcode & 0b00011000)
		{
		case 0b00000000:
			mmu.Write(0xFF00 + registers.m_registers[1], registers.m_registers[6]);
			break;
		case 0b00001000:
			m_16bitRegister = PCREAD16();
			mmu.Write(m_16bitRegister, registers.m_registers[6]);
			break;
		case 0b00010000:
			registers.m_registers[6] = mmu.Read(0xFF00 + registers.m_registers[1]);
			break;
		case 0b00011000:
			m_16bitRegister = PCREAD16();
			registers.m_registers[6] = mmu.Read(m_16bitRegister);
			break;
		}	

		m_step = 0;
		return true;
	}

	m_step++;
	return false;
}

#pragma endregion

#pragma region IF_LD_ADRIMM_r

bool IF_LD_ADRIMM_r::IsValid(uint8_t opcode)
{
	return (opcode & 0b11101111) == 0b11100000;
}

bool IF_LD_ADRIMM_r::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	switch (m_step)
	{
	case 0: //Fetch wait
		break;
	case 1:
		m_8bitRegister = PCREAD8();
		break;
	case 2:
		switch (opcode & 0b00010000)
		{
		case 0b00000000:
			mmu.Write(0xFF00 + m_8bitRegister, registers.m_registers[6]);
			break;
		case 0b00010000:
			registers.m_registers[6] = mmu.Read(0xFF00 + m_8bitRegister);
			break;
		}	
		
		m_step = 0;
		return true;
	}
	
	m_step++;
	return false;
}

#pragma endregion

#pragma endregion

#pragma region Arithmetique Instruction

#pragma region IF_AR_8BIT
bool IF_AR_8BIT::IsValid(uint8_t opcode) 
{
	return ((opcode & 0b11100000) == 0b10000000 ||
		(opcode & 0b11100000) == 0b10100000) ||
			(opcode & 0b11000111) == 0b11000110;
}

bool IF_AR_8BIT::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "AR_8BIT" << std::endl;
	int currentCycles = 0;

	uint8_t _r_num = registers.m_registers[(opcode & 0b00000111)];
	if ((opcode & 0b00000111) == 0b00000110) {
		if ((opcode & 0b01000000) == 0b01000000)
		{
			_r_num = PCREAD8();
		}
		else
		{
			_r_num = MMUREAD8(LH);
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
	if ((opcode & 0b00111000) != 0b00111000)//don't apply on compare instruction
	{
		registers.m_registers[6] = _res;
	}

	return currentCycles;
}
#pragma endregion

#pragma region IF_ADD_HL
bool IF_ADD_HL::IsValid(uint8_t opcode) 
{
	return (opcode & 0b11001111) == 0b00001001;
}

bool IF_ADD_HL::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "ADD_HL" << std::endl;
	int currentCycles = 8;

	switch (opcode & 0b00110000)
	{
		case 0b00000000:
		{
			uint16_t _res = HL + BC;
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
			if (BC > _res - (registers.m_registers[7] & 0b00000001) || HL > _res - (registers.m_registers[7] & 0b00000001)) //c (Carry for the 7 bit) ONLY FOR ADDS, NOT FOR SUBS !!!
			{
				registers.m_registers[7] |= 0b00000001;
			}
			else
			{
				registers.m_registers[7] &= ~0b00000001;
			}
#pragma endregion
#pragma region Flag_H
			if ((BC & 0b00001111) > ((_res - (registers.m_registers[7] & 0b00000001)) & 0b00001111) || (HL & 0b00001111) > ((_res - (registers.m_registers[7] & 0b00000001)) & 0b00001111)) //Flag h (half-carry) same method as Flag C but with mask
			{
				registers.m_registers[7] |= 0b00010000;
			}
			else
			{
				registers.m_registers[7] &= ~0b00010000;
			}
#pragma endregion
#pragma endregion
			WRITE16(LH, GETINVERTED16(_res));
			break;
		}
		case 0b00010000:
		{
			uint16_t _res = HL + DE;
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
			if (DE > _res - (registers.m_registers[7] & 0b00000001) || HL > _res - (registers.m_registers[7] & 0b00000001)) //c (Carry for the 7 bit) ONLY FOR ADDS, NOT FOR SUBS !!!
			{
				registers.m_registers[7] |= 0b00000001;
			}
			else
			{
				registers.m_registers[7] &= ~0b00000001;
			}
#pragma endregion
#pragma region Flag_H
			if ((DE & 0b00001111) > ((_res - (registers.m_registers[7] & 0b00000001)) & 0b00001111) || (HL & 0b00001111) > ((_res - (registers.m_registers[7] & 0b00000001)) & 0b00001111)) //Flag h (half-carry) same method as Flag C but with mask
			{
				registers.m_registers[7] |= 0b00010000;
			}
			else
			{
				registers.m_registers[7] &= ~0b00010000;
			}
#pragma endregion
#pragma endregion
			WRITE16(LH, GETINVERTED16(_res));
			break;
		}
		case 0b00100000:
		{
			uint16_t _res = HL * 2;
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
			if (HL > _res - registers.m_registers[7] & 0b00000001) //c (Carry for the 7 bit) ONLY FOR ADDS, NOT FOR SUBS !!!
			{
				registers.m_registers[7] |= 0b00000001;
			}
			else
			{
				registers.m_registers[7] &= ~0b00000001;
			}
#pragma endregion
#pragma region Flag_H
			if ((HL & 0b00001111) > ((_res - (registers.m_registers[7] & 0b00000001)) & 0b00001111)) //Flag h (half-carry) same method as Flag C but with mask
			{
				registers.m_registers[7] |= 0b00010000;
			}
			else
			{
				registers.m_registers[7] &= ~0b00010000;
			}
#pragma endregion
#pragma endregion
			WRITE16(LH, GETINVERTED16(_res));
			break;
		}
		case 0b00110000:
		{
			uint16_t _res = HL + registers.SP;
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
			if (registers.SP > _res - (registers.m_registers[7] & 0b00000001) || HL > _res - (registers.m_registers[7] & 0b00000001)) //c (Carry for the 7 bit) ONLY FOR ADDS, NOT FOR SUBS !!!
			{
				registers.m_registers[7] |= 0b00000001;
			}
			else
			{
				registers.m_registers[7] &= ~0b00000001;
			}
#pragma endregion
#pragma region Flag_H
			if ((registers.SP & 0b00001111) > ((_res - (registers.m_registers[7] & 0b00000001)) & 0b00001111) || (HL & 0b00001111) > ((_res - (registers.m_registers[7] & 0b00000001)) & 0b00001111)) //Flag h (half-carry) same method as Flag C but with mask
			{
				registers.m_registers[7] |= 0b00010000;
			}
			else
			{
				registers.m_registers[7] &= ~0b00010000;
			}
#pragma endregion
#pragma endregion
			WRITE16(LH, GETINVERTED16(_res));
			break;
		}
	}
	
	return currentCycles;
}
#pragma endregion

#pragma endregion

#pragma region Flow Instructions

#pragma region IF_FLOW_JR

bool IF_FLOW_JR::IsValid(uint8_t opcode)
{
	return (opcode & 0b11111111) == 0b00011000 || (opcode & 0b11100111) == 0b00100000;
}

bool IF_FLOW_JR::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	switch (m_step)
	{
	case 0: //Fetch wait
		break;
	case 1:
		m_8bitRegister = PCREAD8(); // get the offset THEN check if condition is true

		bool condition = false;
		if ((opcode & 0b11100111) == 0b00100000)//JR with condition
		{
			switch (opcode & 0b00011000)
			{
			case 0b00000000:
				condition = (registers.m_registers[7] & 0b01000000) == 0b00000000;
				break;
			case 0b00001000:
				condition = (registers.m_registers[7] & 0b01000000) == 0b01000000;
				break;
			case 0b00010000:
				condition = (registers.m_registers[7] & 0b00000001) == 0b00000000;
				break;
			case 0b00011000:
				condition = (registers.m_registers[7] & 0b00000001) == 0b00000001;
				break;
			}
		}
		else if ((opcode & 0b11111111) == 0b00011000)//Always JR
		{
			condition = true;
		}

		if (condition) //If condition is true, proceed to execute the jump
		{
			break;
		}
		
		m_step = 0;
		return true;
	case 2: //Execute the jump
		
		registers.PC += m_8bitRegister; //TODO : check if this works with signed
		m_step = 0;
		return true;
	}

	m_step++;
	return false;
}

#pragma endregion

#pragma region IF_FLOW_JP

bool IF_FLOW_JP::IsValid(uint8_t opcode)
{
	return (opcode & 0b11111111) == 0b11000011 || (opcode & 0b11111111) == 0b11101001 || (opcode & 0b11100111) == 0b11000010;
}

bool IF_FLOW_JP::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "JUMP P" << std::endl;
	int currentCycles = 0;

	if ((opcode & 0b11111111) == 0b11101001)
	{
		registers.PC = HL;
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

bool IF_FLOW_CALL::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "CALL" << std::endl;
	
	int currentCycles = 0;
		
	uint16_t _nn = PCREAD16();
	
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
		MMUWRITE8(--registers.SP, static_cast<uint8_t>((registers.PC & 0xFF00) >> 8));
		MMUWRITE8(--registers.SP, static_cast<uint8_t>(registers.PC & 0x00FF));
		currentCycles += 4;
	}
	registers.PC = _nn;
	return currentCycles;
}

#pragma endregion

#pragma region IF_FLOW_RET

bool IF_FLOW_RET::IsValid(uint8_t opcode) 
{
	return (opcode & 0b11101111) == 0b11001001 || (opcode & 0b11100111) == 0b11000000;
}

bool IF_FLOW_RET::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//std::cout << "RET" << std::endl;
	int currentCycles = 8;

	bool condition = false;
	bool interruptEnable = false;
	if ((opcode & 0b11101111) == 0b11001001)
	{
		condition = true;
		if ((opcode & 0b11111111) == 0b11011001)
		{
			interruptEnable = true;
		}
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
		uint16_t value = mmu.Read(registers.SP++) + (mmu.Read(registers.SP++) << 8);
		if (registers.SP < 0xFF80)
		{
			throw std::runtime_error("Stack Pointer out of bounds on RET instruction");
		}
		registers.PC = value;
		currentCycles += 4;
	}
	else
	{
		registers.SP += 2;
		if (registers.SP < 0xFF80)
		{
			throw std::runtime_error("Stack Pointer out of bounds on RET instruction");
		}
	}

	if (interruptEnable)
	{
		registers.IME = true;
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

bool IF_ROTATE::Tick(uint8_t opcode, MMU& mmu, Registers& registers) 
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

bool IF_CB_Prefix::Tick(uint8_t _, MMU& mmu, Registers& registers)
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
					mmu.Write(HL, mmu.Read(HL) & 0b11111110);
					registers.m_registers[7] |= (mmu.Read(HL) & 0b10000000) >> 7; //Flag C
					mmu.Write(HL, (mmu.Read(HL) << 1));
					if ((_opcode & 0b00100000) == 0b00000000)
					{
						mmu.Write(HL, mmu.Read(HL) + (_temp_carry));
					}
				}
				else
				{
					registers.m_registers[7] &= 0b11111110;
					registers.m_registers[7] |= (mmu.Read(HL) & 0b10000000) >> 7; //Flag C
					mmu.Write(HL, (mmu.Read(HL) << 1));
					if ((_opcode & 0b00100000) == 0b00000000)
					{
						mmu.Write(HL, mmu.Read(HL) + ((mmu.Read(HL) & 0b10000000) >> 7));
					}
				}
			}
			else // Right
			{
				if ((_opcode & 0b00010000) == 0b00010000)
				{
					uint8_t _temp_carry = (registers.m_registers[7] & 0b00000001);
					registers.m_registers[7] &= 0b11111110;
					registers.m_registers[7] |= (mmu.Read(HL) & 0b00000001); //Flag C
					mmu.Write(HL, mmu.Read(HL) >> 1);
					if ((_opcode & 0b00100000) == 0b00000000)
					{
						mmu.Write(HL, mmu.Read(HL) + (_temp_carry << 7));
					}
				}
				else
				{
					registers.m_registers[7] &= 0b11111110;
					registers.m_registers[7] |= (mmu.Read(HL) & 0b00000001); //Flag C
					mmu.Write(HL, (mmu.Read(HL) >> 1));
					if ((_opcode & 0b00100000) == 0b00000000)
					{
						mmu.Write(HL, mmu.Read(HL) + ((mmu.Read(HL) & 0b00000001) << 7));
					}
				}
			}
#pragma region Flags

			registers.m_registers[7] &= ~0b10000000; //Flag s (negatif)

			if (mmu.Read(HL) == 0) //Flag Z (zero)
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

bool IF_INC_DEC::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	switch (m_step)
	{
	case 0: // Increase/Decrease directly register or get HL value 

		if ((opcode & 0b11000111) == 0b00000011) // using 16 bit registers
		{
			int D = (opcode & 0b00001000) >> 3;
			int _res;
			if (D == 0)
			{
				_res = 1;
			}
			else
			{
				_res = -1;
			}

			switch (opcode & 0b00110000) 
			{
			case 0b00000000:
				WRITE16(CB, GETINVERTED16(BC + _res));
				break;
			case 0b00010000:
				WRITE16(ED, GETINVERTED16(DE + _res));
				break;
			case 0b00100000:
				WRITE16(LH, GETINVERTED16(HL + _res));
				break;
			case 0b00110000:
				registers.SP += _res;
				break;
			}
			m_step = 0;
			return true;
		}
		
		if ((opcode & 0b11000110) == 0b00000100 && (opcode & 0b00111000) != 0b00110000) // using 8 bit registers and not HL
		{
			int D = (opcode & 0b00000001);
			int _res;
			if (D == 0)
			{
				_res = 1;
				registers.m_registers[7] &= ~0b10000000;
			}
			else
			{
				_res = -1;
				registers.m_registers[7] |= 0b10000000;
			}

			if ((opcode & 0b00111000) == 0b00111000)
			{
				registers.m_registers[0b00110000 >> 3] += _res;
				m_8bitRegister = registers.m_registers[0b00110000 >> 3];
			}
			else
			{
				registers.m_registers[(opcode & 0b00111000) >> 3] += _res;
				m_8bitRegister = registers.m_registers[(opcode & 0b00111000) >> 3];
			}
			
#pragma region Flags
#pragma region Flag_Z
			if (m_8bitRegister == 0) //Flag Z (zero)
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
				if (((m_8bitRegister - _res) & 0b00001111) > (m_8bitRegister & 0b00001111)) //Flag h (half-carry) same method as Flag C but with mask
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
				if (((m_8bitRegister - _res) & 0b00001111) < (m_8bitRegister & 0b00001111)) //Flag h (half-borrow) same method as Flag C but with mask
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

			m_step = 0;
			return true;
		}
		break;
	case 1: // If HL is involved

		m_8bitRegister = MMUREAD8(LH); // get HL value
		break;
	case 2: // Increase/Decrease HL value
		
		int D = (opcode & 0b00000001);
		int _res;
		if (D == 0)
		{
			_res = 1;
			registers.m_registers[7] &= ~0b10000000;
		}
		else
		{
			_res = -1;
			registers.m_registers[7] |= 0b10000000;
		}
		
		mmu.Write(HL, m_8bitRegister + _res);
		m_8bitRegister = mmu.Read(HL);
		
#pragma region Flags
#pragma region Flag_Z
		if (m_8bitRegister == 0) //Flag Z (zero)
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
			if (((m_8bitRegister - _res) & 0b00001111) > (m_8bitRegister & 0b00001111)) //Flag h (half-carry) same method as Flag C but with mask
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
			if (((m_8bitRegister - _res) & 0b00001111) < (m_8bitRegister & 0b00001111)) //Flag h (half-borrow) same method as Flag C but with mask
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
		
		m_step = 0;
		return true;
	}

	m_step++;
	return false;
}

#pragma endregion

#pragma region Push/Pop Instruction

#pragma region IF_POP
bool IF_POP::IsValid(uint8_t opcode)
{
	return (opcode & 0b11001011) == 0b11000001;
}

bool IF_POP::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	switch (m_step)
	{
	case 0: //Fetch wait
		break;
	case 1:
		m_8bitRegister = mmu.Read(registers.SP);
		registers.SP++;
		break;
	case 2:
		m_16bitRegister = m_8bitRegister + (mmu.Read(registers.SP) << 8);
		registers.SP++;
		if (registers.SP < 0xFF80) //Stack Pointer overflow check
		{
			throw std::runtime_error("Stack Pointer out of bounds on POP instruction");
		}
		break;
	case 3:
		switch (opcode & 0b00110000)
		{
		case 0b00000000:
			registers.CB = GETINVERTED16(m_16bitRegister);
			break;
		case 0b00010000:
			registers.ED = GETINVERTED16(m_16bitRegister);
			break;
		case 0b00100000:
			registers.LH = GETINVERTED16(m_16bitRegister);
			break;
		case 0b00110000:
			registers.FA = GETINVERTED16(m_16bitRegister);
			break;
		}

		m_step = 0;
		return true;
	}
	
	m_step++;
	return false;
}
#pragma endregion

#pragma region IF_PUSH
bool IF_PUSH::IsValid(uint8_t opcode)
{
	return (opcode & 0b11001011) == 0b11000001;
}

bool IF_PUSH::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	switch (m_step)
	{
	case 0: //Fetch wait
		break;
	case 1:
		registers.SP--;
		break;
	case 2:
		switch (opcode & 0b00110000)
		{
		case 0b00000000:
			MMUWRITE8(registers.SP, static_cast<uint8_t>(BC >> 8));
			registers.SP--;
			break;
		case 0b00010000:
			MMUWRITE8(registers.SP, static_cast<uint8_t>(DE >> 8));
			registers.SP--;
			break;
		case 0b00100000:
			MMUWRITE8(registers.SP, static_cast<uint8_t>(HL >> 8));
			registers.SP--;
			break;
		case 0b00110000:
			MMUWRITE8(registers.SP, static_cast<uint8_t>(AF >> 8));
			registers.SP--;
			break;
		}
		break;
	case 3:
		switch (opcode & 0b00110000)
		{
		case 0b00000000:
			MMUWRITE8(registers.SP, static_cast<uint8_t>(BC & 0x00FF));
			break;
		case 0b00010000:
			MMUWRITE8(registers.SP, static_cast<uint8_t>(DE & 0x00FF));
			break;
		case 0b00100000:
			MMUWRITE8(registers.SP, static_cast<uint8_t>(HL & 0x00FF));
			break;
		case 0b00110000:
			MMUWRITE8(registers.SP, static_cast<uint8_t>(AF & 0x00FF));
			break;
		}

		m_step = 0;
		return true;
	}

	m_step++;
	return false;
}
#pragma endregion

#pragma endregion

#pragma region Interrupts Instructions

bool IF_DI_EI::IsValid(uint8_t opcode)
{
	return (opcode & 0b11110111) == 0b11110011;
}

bool IF_DI_EI::Tick(uint8_t opcode, MMU& mmu, Registers& registers)
{
	//note : this instruction only takes 1 M-cycle
	//TODO : implement delayed effect of EI
	
	if ((opcode & 0b00001000) == 0b00001000)
	{
		registers.IME = true;
	}
	else
	{
		registers.IME = false;
	}

	return true;
}

#pragma endregion

#pragma region Custom Instruction

bool IF_Finish::IsValid(uint8_t opcode) //Deprecated
{
	return (opcode & 0b11111111) == 0xFD;
}

bool IF_Finish::Tick(uint8_t opcode, MMU& mmu, Registers& registers) //Deprecated
{
	int currentCycles = -2;

	std::cout << "End of BootRom";

	return currentCycles;
}

#pragma endregion
