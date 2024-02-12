#include "alu.h"
#include "cpu.h"
#include "mmu.h"

#pragma region Load Instructions

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

#pragma endregion

#pragma region Arithmetique Instructions

class IF_AR final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return ((opcode & 0b11100000) == 0b10000000 || (opcode & 0b11100000) == 0b10100000) || (opcode & 0b11000111) == 0b11000110;
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
		uint8_t _r_num;

		switch (opcode & 0b00000111)
		{
		case 0b00000000:
			_r_num = registers.m_registers[2];
			break;
		case 0b00000001:
			_r_num = registers.m_registers[3];
			break;
		case 0b00000010:
			_r_num = registers.m_registers[4];
			break;
		case 0b00000011:
			_r_num = registers.m_registers[5];
			break;
		case 0b00000100:
			_r_num = registers.m_registers[6];
			break;
		case 0b00000101:
			_r_num = registers.m_registers[7];
			break;
		case 0b00000110:
			if ((opcode & 0b01000000) == 0b01000000)
			{
				_r_num = mmu.Read(registers.PC++);
			}
			else
			{
				_r_num = mmu.Read(registers.HL);
			}
			break;
		case 0b00000111:
			_r_num = registers.m_registers[0];
			break;
		}

		uint8_t _res;

		switch (opcode & 0b00100000)
		{
		case 0b00000000:
#pragma region Arithmetique
			switch (opcode & 0b00011000)
			{
			case 0b00000000:
				_res = registers.m_registers[0] + _r_num + (registers.m_registers[1] & 0b00000001);
#pragma region Flags

				if ((_res & 0b10000000) == 0b10000000) {
					registers.m_registers[1] |= 0b10000000; //Flag s (negatif)
				}

				if (_res == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				if (_r_num > _res - (registers.m_registers[1] & 0b00000001) || registers.m_registers[0] > _res - (registers.m_registers[1] & 0b00000001)) //Flag p/v (overflow) + c (Carry for the 7 bit) ONLY FOR ADDS, NOT FOR SUBS !!!
				{
					registers.m_registers[1] |= 0b10000000;
					registers.m_registers[1] |= 0b00000001;
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
					registers.m_registers[1] &= !0b00000001;
				}

				if ((_r_num & 0b00001111) > ((_res - (registers.m_registers[1] & 0b00000001)) & 0b00001111) || (registers.m_registers[0] & 0b00001111) > ((_res - (registers.m_registers[1] & 0b00000001)) & 0b00001111)) //Flag h (half-carry) same method as Flag p/v but with mask
				{
					registers.m_registers[1] |= 0b00010000;
				}
				else
				{
					registers.m_registers[1] &= !0b00010000;
				}

#pragma endregion
				break;
			case 0b00001000:
				_res = registers.m_registers[0] + _r_num;
#pragma region Flags

				if ((_res & 0b10000000) == 0b10000000) {
					registers.m_registers[1] |= 0b10000000; //Flag s (negatif)
				}

				if (_res == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				if ((_r_num & 0b00001111) > (_res & 0b00001111) || (registers.m_registers[0] & 0b00001111) > (_res & 0b00001111)) //Flag h (half-carry) same method as Flag p/v but with mask
				{
					registers.m_registers[1] |= 0b00010000;
				}
				else
				{
					registers.m_registers[1] &= !0b00010000;
				}

#pragma endregion
				break;
			case 0b00010000:
				_res = (registers.m_registers[0] - _r_num) - (registers.m_registers[1] & 0b00000001);
#pragma region Negatif_Flags

				if ((_res & 0b10000000) == 0b10000000) {
					registers.m_registers[1] |= 0b10000000; //Flag s (negatif)
				}

				if (_res == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				if (_r_num < _res + (registers.m_registers[1] & 0b00000001) || registers.m_registers[0] < _res + (registers.m_registers[1] & 0b00000001)) //Flag p/v (overflow) + c (Borrow for the 7 bit) ONLY FOR SUBS, NOT FOR ADDS !!!
				{
					registers.m_registers[1] |= 0b10000000;
					registers.m_registers[1] |= 0b00000001;
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
					registers.m_registers[1] &= !0b00000001;
				}

				if ((_r_num & 0b00001111) < ((_res + (registers.m_registers[1] & 0b00000001)) & 0b00001111) || (registers.m_registers[0] & 0b00001111) < ((_res + (registers.m_registers[1] & 0b00000001)) & 0b00001111)) //Flag h (half-borrow) same method as Flag p/v but with mask
				{
					registers.m_registers[1] |= 0b00010000;
				}
				else
				{
					registers.m_registers[1] &= !0b00010000;
				}

#pragma endregion
				break;
			case 0b00011000:
				_res = registers.m_registers[0] - _r_num;
#pragma region Negatif_Flags

				if ((_res & 0b10000000) == 0b10000000) {
					registers.m_registers[1] |= 0b10000000; //Flag s (negatif)
				}

				if (_res == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				if ((_r_num & 0b00001111) < (_res & 0b00001111) || (registers.m_registers[0] & 0b00001111) < (_res & 0b00001111)) //Flag h (half-borrow) same method as Flag p/v but with mask
				{
					registers.m_registers[1] |= 0b00010000;
				}
				else
				{
					registers.m_registers[1] &= !0b00010000;
				}

#pragma endregion
				break;
			}
			return;

#pragma endregion
		case 0b00100000:
#pragma region Condition
			switch (opcode & 0b00011000)
			{
			case 0b00000000:
				_res = registers.m_registers[0] & _r_num;
#pragma region Flags

				if ((_res & 0b10000000) == 0b10000000) {
					registers.m_registers[1] |= 0b10000000; //Flag s (negatif)
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
				}

				if (_res == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[1] |= 0b00010000; //Flag h set
				registers.m_registers[1] &= !0b00000001; //Flag c reset
				registers.m_registers[1] &= !0b00000100; //Flag p/v reset

#pragma endregion
				registers.m_registers[0] = _res;
				break;
			case 0b00001000:
				_res = registers.m_registers[0] ^ _r_num;
#pragma region Flags

				if ((_res & 0b10000000) == 0b10000000) {
					registers.m_registers[1] |= 0b10000000; //Flag s (negatif)
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
				}

				if (_res == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[1] &= !0b00010000; //Flag h reset
				registers.m_registers[1] &= !0b00000001; //Flag c reset
				registers.m_registers[1] &= !0b00000100; //Flag p/v reset

#pragma endregion
				registers.m_registers[0] = _res;
				break;
			case 0b00010000:
				_res = registers.m_registers[0] | _r_num;
#pragma region Flags

				if ((_res & 0b10000000) == 0b10000000) {
					registers.m_registers[1] |= 0b10000000; //Flag s (negatif)
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
				}

				if (_res == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[1] &= !0b00010000; //Flag h reset
				registers.m_registers[1] &= !0b00000001; //Flag c reset
				registers.m_registers[1] &= !0b00000100; //Flag p/v reset

#pragma endregion
				registers.m_registers[0] = _res;
				break;
			case 0b00011000:
				_res = (registers.m_registers[0] - _r_num) - (registers.m_registers[1] & 0b00000001);

#pragma region Negatif_Flags

				if ((_res & 0b10000000) == 0b10000000) {
					registers.m_registers[1] |= 0b10000000; //Flag s (negatif)
				}

				if (_res == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				if (_r_num < _res + (registers.m_registers[1] & 0b00000001) || registers.m_registers[0] < _res + (registers.m_registers[1] & 0b00000001)) //Flag p/v (overflow) + c (Borrow for the 7 bit) ONLY FOR SUBS, NOT FOR ADDS !!!
				{
					registers.m_registers[1] |= 0b10000000;
					registers.m_registers[1] |= 0b00000001;
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
					registers.m_registers[1] &= !0b00000001;
				}

				if ((_r_num & 0b00001111) < ((_res + (registers.m_registers[1] & 0b00000001)) & 0b00001111) || (registers.m_registers[0] & 0b00001111) < ((_res + (registers.m_registers[1] & 0b00000001)) & 0b00001111)) //Flag h (half-borrow) same method as Flag p/v but with mask
				{
					registers.m_registers[1] |= 0b00010000;
				}
				else
				{
					registers.m_registers[1] &= !0b00010000;
				}

#pragma endregion

				break;
			}
			break;
#pragma endregion
		}
		if ((opcode & 0b00011000) != 0b000110000) {
			registers.m_registers[0] = _res;
		}
	}
};

#pragma endregion

#pragma region Flow Instructions

class IF_FLOW_JR final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return (opcode & 0b11111111) == 0b00011000 || (opcode & 0b11111111) == 0b00010000 || (opcode & 0b11100111) == 0b00100000;
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
		int8_t e = mmu.Read(registers.PC++);
		if ((opcode & 0b11100111) == 0b00100000)//JR with cc (if cc = true)
		{
			switch (opcode & 0b00011000)
			{
			case 0b00000000:
				if ((registers.m_registers[1] & 0b01000000) != 0b01000000)
				{
					registers.PC -= e;
				}
				break;
			case 0b00001000:
				if ((registers.m_registers[1] & 0b01000000) == 0b01000000)
				{
					registers.PC -= e;
				}
				break;
			case 0b00010000:
				if ((registers.m_registers[1] & 0b00000001) != 0b00000001)
				{
					registers.PC -= e;
				}
				break;
			case 0b00011000:
				if ((registers.m_registers[1] & 0b00000001) == 0b00000001)
				{
					registers.PC -= e;
				}
				break;
			}
			return;
		}
		if ((opcode & 0b11111111) == 0b00010000)//JR with B (if B = 0) + B -= 1
		{
			if (registers.m_registers[2] == 0)
			{
				registers.PC -= e;
			}
			return;
		}
		if ((opcode & 0b11111111) == 0b00011000)//Always JR
		{
			registers.PC -= e;
			return;
		}
	}
};

class IF_FLOW_JP final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return (opcode & 0b11111111) == 0b11000011 || (opcode & 0b11111111) == 0b11101001 || (opcode & 0b11000111) == 0b11000010;
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
		if ((opcode & 0b11111111) == 0b11101001)
		{
			registers.PC = registers.HL;
			return;
		}
		uint16_t _nn = (mmu.Read(registers.PC++) + (mmu.Read(registers.PC++) << 8));
		if ((opcode & 0b11111111) == 0b11000011)
		{
			registers.PC = _nn;
			return;
		}
		if ((opcode & 0b11000111) == 0b11000010)
		{
			switch (opcode & 0b00111000)
			{
			case 0b00000000:
				if ((registers.m_registers[1] & 0b01000000) != 0b01000000)
				{
					registers.PC = _nn;
				}
				break;
			case 0b00001000:
				if ((registers.m_registers[1] & 0b01000000) == 0b01000000)
				{
					registers.PC = _nn;
				}
				break;
			case 0b00010000:
				if ((registers.m_registers[1] & 0b00000001) != 0b00000001)
				{
					registers.PC = _nn;
				}
				break;
			case 0b00011000:
				if ((registers.m_registers[1] & 0b00000001) == 0b00000001)
				{
					registers.PC = _nn;
				}
				break;
			case (0b00100000):
				if ((registers.m_registers[1] & 0b00000100) != 0b00000100)
				{
					registers.PC = _nn;
				}
				break;
			case (0b00101000):
				if ((registers.m_registers[1] & 0b00000100) == 0b00000100)
				{
					registers.PC = _nn;
				}
				break;
			case (0b00110000):
				if ((registers.m_registers[1] & 0b10000000) != 0b10000000)
				{
					registers.PC = _nn;
				}
				break;
			case (0b00111000):
				if ((registers.m_registers[1] & 0b10000000) == 0b10000000)
				{
					registers.PC = _nn;
				}
				break;
			}
			return;
		}
		return;
	}
};

class IF_FLOW_CALL final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return (opcode & 0b11111111) == 0b11001101 || (opcode & 0b11000111) == 0b11000100;
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
		uint16_t _nn = (mmu.Read(registers.PC++) + (mmu.Read(registers.PC++) << 8));
		bool condition = false;
		if ((opcode & 0b11111111) == 0b11001101)
		{
			condition = true;
		}
		else
		{
			switch (opcode & 0b00111000)
			{
			case 0b00000000:
				if ((registers.m_registers[1] & 0b01000000) != 0b01000000)
				{
					condition = true;
				}
				break;
			case 0b00001000:
				if ((registers.m_registers[1] & 0b01000000) == 0b01000000)
				{
					condition = true;
				}
				break;
			case 0b00010000:
				if ((registers.m_registers[1] & 0b00000001) != 0b00000001)
				{
					condition = true;
				}
				break;
			case 0b00011000:
				if ((registers.m_registers[1] & 0b00000001) == 0b00000001)
				{
					condition = true;
				}
				break;
			case (0b00100000):
				if ((registers.m_registers[1] & 0b00000100) != 0b00000100)
				{
					condition = true;
				}
				break;
			case (0b00101000):
				if ((registers.m_registers[1] & 0b00000100) == 0b00000100)
				{
					condition = true;
				}
				break;
			case (0b00110000):
				if ((registers.m_registers[1] & 0b10000000) != 0b10000000)
				{
					condition = true;
				}
				break;
			case (0b00111000):
				if ((registers.m_registers[1] & 0b10000000) == 0b10000000)
				{
					condition = true;
				}
				break;
			}
		}
		if (condition)
		{
			mmu.Write(registers.SP - 1, registers.PC >> 8);
			mmu.Write(registers.SP - 2, registers.PC);
			registers.SP -= 2;
		}
		registers.PC = _nn;
		return;
	}
};

class IF_FLOW_RET final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return (opcode & 0b11111111) == 0b11001001 || (opcode & 0b11000111) == 0b11000000;
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
		uint16_t _nn = (mmu.Read(registers.PC++) + (mmu.Read(registers.PC++) << 8));
		bool condition = false;
		if ((opcode & 0b11111111) == 0b11001001)
		{
			condition = true;
		}
		else
		{
			switch (opcode & 0b00111000)
			{
			case 0b00000000:
				if ((registers.m_registers[1] & 0b01000000) != 0b01000000)
				{
					condition = true;
				}
				break;
			case 0b00001000:
				if ((registers.m_registers[1] & 0b01000000) == 0b01000000)
				{
					condition = true;
				}
				break;
			case 0b00010000:
				if ((registers.m_registers[1] & 0b00000001) != 0b00000001)
				{
					condition = true;
				}
				break;
			case 0b00011000:
				if ((registers.m_registers[1] & 0b00000001) == 0b00000001)
				{
					condition = true;
				}
				break;
			case (0b00100000):
				if ((registers.m_registers[1] & 0b00000100) != 0b00000100)
				{
					condition = true;
				}
				break;
			case (0b00101000):
				if ((registers.m_registers[1] & 0b00000100) == 0b00000100)
				{
					condition = true;
				}
				break;
			case (0b00110000):
				if ((registers.m_registers[1] & 0b10000000) != 0b10000000)
				{
					condition = true;
				}
				break;
			case (0b00111000):
				if ((registers.m_registers[1] & 0b10000000) == 0b10000000)
				{
					condition = true;
				}
				break;
			}
		}
		if (condition)
		{
			registers.PC = ((mmu.Read(registers.SP + 1) << 8) + mmu.Read(registers.SP));
		}
		registers.SP += 2;
		return;
	}
};

#pragma endregion

#pragma region Rotate Instructions

class IF_ROTATE final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return (opcode & 0b11100111) == 0b00000111;
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
		switch (opcode & 0b00001000) 
		{
		case 0b00000000: // Left
			if ((opcode & 0b00010000) == 0b00010000) 
			{
				uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
				registers.m_registers[1] &= 0b11111110;
				registers.m_registers[1] |= (registers.m_registers[0] & 0b10000000) >> 7;
				registers.m_registers[0] = (registers.m_registers[0] << 1) + (_temp_carry);

			}
			else 
			{
				registers.m_registers[1] &= 0b11111110;
				registers.m_registers[1] |= (registers.m_registers[0] & 0b10000000) >> 7;
				registers.m_registers[0] = (registers.m_registers[0] << 1) + ((registers.m_registers[0] & 0b10000000) >> 7);
			}
			break;

		case 0b00001000: // Right
			if ((opcode & 0b00010000) == 0b00010000)
			{
				uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
				registers.m_registers[1] &= 0b11111110;
				registers.m_registers[1] |= (registers.m_registers[0] & 0b00000001);
				registers.m_registers[0] = (registers.m_registers[0] >> 1) + (_temp_carry << 7);

			}
			else
			{
				registers.m_registers[1] &= 0b11111110;
				registers.m_registers[1] |= (registers.m_registers[0] & 0b00000001);
				registers.m_registers[0] = (registers.m_registers[0] >> 1) + ((registers.m_registers[0] & 0b00000001) << 7);
			}
			break;
		}

#pragma region Flags

		if ((registers.m_registers[0] & 0b10000000) == 0b10000000) {//Flag s (negatif)
			registers.m_registers[1] |= 0b10000000; 
		}
		else 
		{
			registers.m_registers[1] &= !0b10000000;
		}

		if (registers.m_registers[0] == 0) //Flag Z (zero)
		{
			registers.m_registers[1] |= 0b01000000;
		}
		else
		{
			registers.m_registers[1] &= !0b01000000;
		}

		registers.m_registers[1] &= !0b00010000; //Flag H reset

		bool _even = (((registers.m_registers[0] & 0b00000001) >> 0)
			+ ((registers.m_registers[0] & 0b00000010) >> 1)
			+ ((registers.m_registers[0] & 0b00000100) >> 2)
			+ ((registers.m_registers[0] & 0b00001000) >> 3)
			+ ((registers.m_registers[0] & 0b00010000) >> 4)
			+ ((registers.m_registers[0] & 0b00100000) >> 5)
			+ ((registers.m_registers[0] & 0b01000000) >> 6)
			+ ((registers.m_registers[0] & 0b10000000) >> 7)) % 2 == 0;


		if (_even) //Flag P/V (even = 1, odd = 0)
		{
			registers.m_registers[1] |= 0b00000100;
		}
		else
		{
			registers.m_registers[1] &= !0b00000100;
		}

#pragma endregion

	}
};

#pragma endregion

#pragma region CB Prefix Instructions

class IF_CB_Prefix final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return opcode == 0xCB;
	}

	void Execute(uint8_t _, MMU& mmu, Registers& registers) override
	{
		uint8_t _opcode = mmu.Read(registers.PC++);
		if ((_opcode & 0b11000000) == 0b00000000) 
		{
			switch (_opcode & 0b00000111)
			{
			case 0b00000000:
				if ((_opcode & 0b00001000) == 0b00000000)// Left
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[2] & 0b10000000) >> 7;
						registers.m_registers[2] = (registers.m_registers[2] << 1);
						if ((_opcode & 0b00100000) == 0b00000000) 
						{
							registers.m_registers[2] += (_temp_carry);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[2] & 0b10000000) >> 7;
						registers.m_registers[2] = (registers.m_registers[2] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[2] += ((registers.m_registers[2] & 0b10000000) >> 7);
						}
					}
				}
				else if ((_opcode & 0b00001000) == 0b00001000)// Right
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[2] & 0b00000001);
						registers.m_registers[2] = (registers.m_registers[2] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[3] += (_temp_carry << 7);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[2] & 0b00000001);
						registers.m_registers[2] = (registers.m_registers[2] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[3] += ((registers.m_registers[2] & 0b00000001) << 7);
						}
					}
				}

#pragma region Flags

				if ((registers.m_registers[2] & 0b10000000) == 0b10000000) {//Flag s (negatif)
					registers.m_registers[1] |= 0b10000000;
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
				}

				if (registers.m_registers[2] == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[1] &= !0b00010000; //Flag H reset

				bool _even = (((registers.m_registers[2] & 0b00000001) >> 0)
					+ ((registers.m_registers[2] & 0b00000010) >> 1)
					+ ((registers.m_registers[2] & 0b00000100) >> 2)
					+ ((registers.m_registers[2] & 0b00001000) >> 3)
					+ ((registers.m_registers[2] & 0b00010000) >> 4)
					+ ((registers.m_registers[2] & 0b00100000) >> 5)
					+ ((registers.m_registers[2] & 0b01000000) >> 6)
					+ ((registers.m_registers[2] & 0b10000000) >> 7)) % 2 == 0;


				if (_even) //Flag P/V (even = 1, odd = 0)
				{
					registers.m_registers[1] |= 0b00000100;
				}
				else
				{
					registers.m_registers[1] &= !0b00000100;
				}

#pragma endregion

				break;
			case 0b00000001:
				if ((_opcode & 0b00001000) == 0b00000000)// Left
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[3] & 0b10000000) >> 7;
						registers.m_registers[3] = (registers.m_registers[3] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[3] += (_temp_carry);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[3] & 0b10000000) >> 7;
						registers.m_registers[3] = (registers.m_registers[3] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[3] += ((registers.m_registers[3] & 0b10000000) >> 7);
						}
					}
				}
				else if ((_opcode & 0b00001000) == 0b00001000)// Right
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[3] & 0b00000001);
						registers.m_registers[3] = (registers.m_registers[3] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[2] += (_temp_carry << 7);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[3] & 0b00000001);
						registers.m_registers[3] = (registers.m_registers[3] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[2] += ((registers.m_registers[3] & 0b00000001) << 7);
						}
					}
				}

#pragma region Flags

				if ((registers.m_registers[3] & 0b10000000) == 0b10000000) {//Flag s (negatif)
					registers.m_registers[1] |= 0b10000000;
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
				}

				if (registers.m_registers[3] == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[1] &= !0b00010000; //Flag H reset

				bool _even = (((registers.m_registers[3] & 0b00000001) >> 0)
					+ ((registers.m_registers[3] & 0b00000010) >> 1)
					+ ((registers.m_registers[3] & 0b00000100) >> 2)
					+ ((registers.m_registers[3] & 0b00001000) >> 3)
					+ ((registers.m_registers[3] & 0b00010000) >> 4)
					+ ((registers.m_registers[3] & 0b00100000) >> 5)
					+ ((registers.m_registers[3] & 0b01000000) >> 6)
					+ ((registers.m_registers[3] & 0b10000000) >> 7)) % 2 == 0;


				if (_even) //Flag P/V (even = 1, odd = 0)
				{
					registers.m_registers[1] |= 0b00000100;
				}
				else
				{
					registers.m_registers[1] &= !0b00000100;
				}

#pragma endregion

				break;
			case 0b00000010:
				if ((_opcode & 0b00001000) == 0b00000000)// Left
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[4] & 0b10000000) >> 7;
						registers.m_registers[4] = (registers.m_registers[4] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[4] += (_temp_carry);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[4] & 0b10000000) >> 7;
						registers.m_registers[4] = (registers.m_registers[4] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[4] += ((registers.m_registers[4] & 0b10000000) >> 7);
						}
					}
				}
				else if ((_opcode & 0b00001000) == 0b00001000)// Right
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[4] & 0b00000001);
						registers.m_registers[4] = (registers.m_registers[4] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[4] += (_temp_carry << 7);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[4] & 0b00000001);
						registers.m_registers[4] = (registers.m_registers[4] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[4] += ((registers.m_registers[4] & 0b00000001) << 7);
						}
					}
				}

#pragma region Flags

				if ((registers.m_registers[4] & 0b10000000) == 0b10000000) {//Flag s (negatif)
					registers.m_registers[1] |= 0b10000000;
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
				}

				if (registers.m_registers[4] == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[1] &= !0b00010000; //Flag H reset

				bool _even = (((registers.m_registers[4] & 0b00000001) >> 0)
					+ ((registers.m_registers[4] & 0b00000010) >> 1)
					+ ((registers.m_registers[4] & 0b00000100) >> 2)
					+ ((registers.m_registers[4] & 0b00001000) >> 3)
					+ ((registers.m_registers[4] & 0b00010000) >> 4)
					+ ((registers.m_registers[4] & 0b00100000) >> 5)
					+ ((registers.m_registers[4] & 0b01000000) >> 6)
					+ ((registers.m_registers[4] & 0b10000000) >> 7)) % 2 == 0;


				if (_even) //Flag P/V (even = 1, odd = 0)
				{
					registers.m_registers[1] |= 0b00000100;
				}
				else
				{
					registers.m_registers[1] &= !0b00000100;
				}

#pragma endregion

				break;
			case 0b00000011:
				if ((_opcode & 0b00001000) == 0b00000000)// Left
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[5] & 0b10000000) >> 7;
						registers.m_registers[5] = (registers.m_registers[5] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[5] += (_temp_carry);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[5] & 0b10000000) >> 7;
						registers.m_registers[5] = (registers.m_registers[5] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[5] += ((registers.m_registers[5] & 0b10000000) >> 7);
						}
					}
				}
				else if ((_opcode & 0b00001000) == 0b00001000)// Right
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[5] & 0b00000001);
						registers.m_registers[5] = (registers.m_registers[5] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[5] += (_temp_carry << 7);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[5] & 0b00000001);
						registers.m_registers[5] = (registers.m_registers[5] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[5] += ((registers.m_registers[5] & 0b00000001) << 7);
						}
					}
				}

#pragma region Flags

				if ((registers.m_registers[5] & 0b10000000) == 0b10000000) {//Flag s (negatif)
					registers.m_registers[1] |= 0b10000000;
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
				}

				if (registers.m_registers[5] == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[1] &= !0b00010000; //Flag H reset

				bool _even = (((registers.m_registers[5] & 0b00000001) >> 0)
					+ ((registers.m_registers[5] & 0b00000010) >> 1)
					+ ((registers.m_registers[5] & 0b00000100) >> 2)
					+ ((registers.m_registers[5] & 0b00001000) >> 3)
					+ ((registers.m_registers[5] & 0b00010000) >> 4)
					+ ((registers.m_registers[5] & 0b00100000) >> 5)
					+ ((registers.m_registers[5] & 0b01000000) >> 6)
					+ ((registers.m_registers[5] & 0b10000000) >> 7)) % 2 == 0;


				if (_even) //Flag P/V (even = 1, odd = 0)
				{
					registers.m_registers[1] |= 0b00000100;
				}
				else
				{
					registers.m_registers[1] &= !0b00000100;
				}

#pragma endregion

				break;
			case 0b00000100:
				if ((_opcode & 0b00001000) == 0b00000000)// Left
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[6] & 0b10000000) >> 7;
						registers.m_registers[6] = (registers.m_registers[6] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[6] += (_temp_carry);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[6] & 0b10000000) >> 7;
						registers.m_registers[6] = (registers.m_registers[6] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[6] += ((registers.m_registers[6] & 0b10000000) >> 7);
						}
					}
				}
				else if ((_opcode & 0b00001000) == 0b00001000)// Right
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[6] & 0b00000001);
						registers.m_registers[6] = (registers.m_registers[6] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[6] += (_temp_carry << 7);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[6] & 0b00000001);
						registers.m_registers[6] = (registers.m_registers[6] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[6] += ((registers.m_registers[6] & 0b00000001) << 7);
						}
					}
				}

#pragma region Flags

				if ((registers.m_registers[6] & 0b10000000) == 0b10000000) {//Flag s (negatif)
					registers.m_registers[1] |= 0b10000000;
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
				}

				if (registers.m_registers[6] == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[1] &= !0b00010000; //Flag H reset

				bool _even = (((registers.m_registers[6] & 0b00000001) >> 0)
					+ ((registers.m_registers[6] & 0b00000010) >> 1)
					+ ((registers.m_registers[6] & 0b00000100) >> 2)
					+ ((registers.m_registers[6] & 0b00001000) >> 3)
					+ ((registers.m_registers[6] & 0b00010000) >> 4)
					+ ((registers.m_registers[6] & 0b00100000) >> 5)
					+ ((registers.m_registers[6] & 0b01000000) >> 6)
					+ ((registers.m_registers[6] & 0b10000000) >> 7)) % 2 == 0;


				if (_even) //Flag P/V (even = 1, odd = 0)
				{
					registers.m_registers[1] |= 0b00000100;
				}
				else
				{
					registers.m_registers[1] &= !0b00000100;
				}

#pragma endregion

				break;
			case 0b00000101:
				if ((_opcode & 0b00001000) == 0b00000000)// Left
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[7] & 0b10000000) >> 7;
						registers.m_registers[7] = (registers.m_registers[7] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[7] += (_temp_carry);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[7] & 0b10000000) >> 7;
						registers.m_registers[7] = (registers.m_registers[7] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[7] += ((registers.m_registers[7] & 0b10000000) >> 7);
						}
					}
				}
				else if ((_opcode & 0b00001000) == 0b00001000)// Right
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[7] & 0b00000001);
						registers.m_registers[7] = (registers.m_registers[7] >> 1) + (_temp_carry << 7);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[7] += (_temp_carry << 7);
						}

					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[7] & 0b00000001);
						registers.m_registers[7] = (registers.m_registers[7] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[7] += ((registers.m_registers[7] & 0b00000001) << 7);
						}
					}
				}

#pragma region Flags

				if ((registers.m_registers[7] & 0b10000000) == 0b10000000) {//Flag s (negatif)
					registers.m_registers[1] |= 0b10000000;
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
				}

				if (registers.m_registers[7] == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[1] &= !0b00010000; //Flag H reset

				bool _even = (((registers.m_registers[7] & 0b00000001) >> 0)
					+ ((registers.m_registers[7] & 0b00000010) >> 1)
					+ ((registers.m_registers[7] & 0b00000100) >> 2)
					+ ((registers.m_registers[7] & 0b00001000) >> 3)
					+ ((registers.m_registers[7] & 0b00010000) >> 4)
					+ ((registers.m_registers[7] & 0b00100000) >> 5)
					+ ((registers.m_registers[7] & 0b01000000) >> 6)
					+ ((registers.m_registers[7] & 0b10000000) >> 7)) % 2 == 0;


				if (_even) //Flag P/V (even = 1, odd = 0)
				{
					registers.m_registers[1] |= 0b00000100;
				}
				else
				{
					registers.m_registers[1] &= !0b00000100;
				}

#pragma endregion

				break;
			case 0b00000110:
				if ((_opcode & 0b00001000) == 0b00000000)// Left
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						mmu.Write(registers.HL, mmu.Read(registers.HL) & 0b11111110);
						registers.m_registers[1] |= (mmu.Read(registers.HL) & 0b10000000) >> 7;
						mmu.Write(registers.HL, (mmu.Read(registers.HL) << 1));
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							mmu.Write(registers.HL, mmu.Read(registers.HL) + (_temp_carry));
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (mmu.Read(registers.HL) & 0b10000000) >> 7;
						mmu.Write(registers.HL, (mmu.Read(registers.HL) << 1));
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							mmu.Write(registers.HL, mmu.Read(registers.HL) + (mmu.Read(registers.HL) & 0b10000000) >> 7);
						}
					}
				}
				else if ((_opcode & 0b00001000) == 0b00001000)// Right
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (mmu.Read(registers.HL) & 0b00000001);
						mmu.Write(registers.HL, mmu.Read(registers.HL) >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							mmu.Write(registers.HL, mmu.Read(registers.HL) + (_temp_carry << 7));
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (mmu.Read(registers.HL) & 0b00000001);
						mmu.Write(registers.HL, (mmu.Read(registers.HL) >> 1));
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							mmu.Write(registers.HL, mmu.Read(registers.HL) + (mmu.Read(registers.HL) & 0b00000001) << 7);
						}
					}
				}

#pragma region Flags

				if ((mmu.Read(registers.HL) & 0b10000000) == 0b10000000) {//Flag s (negatif)
					registers.m_registers[1] |= 0b10000000;
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
				}

				if (mmu.Read(registers.HL) == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[1] &= !0b00010000; //Flag H reset

				bool _even = (((mmu.Read(registers.HL) & 0b00000001) >> 0)
					+ ((mmu.Read(registers.HL) & 0b00000010) >> 1)
					+ ((mmu.Read(registers.HL) & 0b00000100) >> 2)
					+ ((mmu.Read(registers.HL) & 0b00001000) >> 3)
					+ ((mmu.Read(registers.HL) & 0b00010000) >> 4)
					+ ((mmu.Read(registers.HL) & 0b00100000) >> 5)
					+ ((mmu.Read(registers.HL) & 0b01000000) >> 6)
					+ ((mmu.Read(registers.HL) & 0b10000000) >> 7)) % 2 == 0;


				if (_even) //Flag P/V (even = 1, odd = 0)
				{
					registers.m_registers[1] |= 0b00000100;
				}
				else
				{
					registers.m_registers[1] &= !0b00000100;
				}

#pragma endregion

				break;
			case 0b00000111:
				if ((_opcode & 0b00001000) == 0b00000000)// Left
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[0] & 0b10000000) >> 7;
						registers.m_registers[0] = (registers.m_registers[0] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[0] += (_temp_carry);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[0] & 0b10000000) >> 7;
						registers.m_registers[0] = (registers.m_registers[0] << 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[0] += ((registers.m_registers[0] & 0b10000000) >> 7);
						}
					}
				}
				else if ((_opcode & 0b00001000) == 0b00001000)// Right
				{
					if ((_opcode & 0b00010000) == 0b00010000)
					{
						uint8_t _temp_carry = (registers.m_registers[1] & 0b00000001);
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[0] & 0b00000001);
						registers.m_registers[0] = (registers.m_registers[0] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[0] += (_temp_carry << 7);
						}
					}
					else
					{
						registers.m_registers[1] &= 0b11111110;
						registers.m_registers[1] |= (registers.m_registers[0] & 0b00000001);
						registers.m_registers[0] = (registers.m_registers[0] >> 1);
						if ((_opcode & 0b00100000) == 0b00000000)
						{
							registers.m_registers[0] += ((registers.m_registers[0] & 0b00000001) << 7);
						}
					}
				}

#pragma region Flags

				if ((registers.m_registers[0] & 0b10000000) == 0b10000000) {//Flag s (negatif)
					registers.m_registers[1] |= 0b10000000;
				}
				else
				{
					registers.m_registers[1] &= !0b10000000;
				}

				if (registers.m_registers[0] == 0) //Flag Z (zero)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}

				registers.m_registers[1] &= !0b00010000; //Flag H reset

				bool _even = (((registers.m_registers[0] & 0b00000001) >> 0)
					+ ((registers.m_registers[0] & 0b00000010) >> 1)
					+ ((registers.m_registers[0] & 0b00000100) >> 2)
					+ ((registers.m_registers[0] & 0b00001000) >> 3)
					+ ((registers.m_registers[0] & 0b00010000) >> 4)
					+ ((registers.m_registers[0] & 0b00100000) >> 5)
					+ ((registers.m_registers[0] & 0b01000000) >> 6)
					+ ((registers.m_registers[0] & 0b10000000) >> 7)) % 2 == 0;


				if (_even) //Flag P/V (even = 1, odd = 0)
				{
					registers.m_registers[1] |= 0b00000100;
				}
				else
				{
					registers.m_registers[1] &= !0b00000100;
				}

#pragma endregion

				break;
			}
			return;
		}
		if ((_opcode & 0b11000000) == 0b01000000) 
		{
			registers.m_registers[1] |= 0b00010000;
			registers.m_registers[1] &= !0b00000010;

			int _offset_bit;
			switch (_opcode & 0b00111000)
			{
			case 0b00000000:
				_offset_bit = 0;
				break;
			case 0b00001000:
				_offset_bit = 1;
				break;
			case 0b00010000:
				_offset_bit = 2;
				break;
			case 0b00011000:
				_offset_bit = 3;
				break;
			case 0b00100000:
				_offset_bit = 4;
				break;
			case 0b00101000:
				_offset_bit = 5;
				break;
			case 0b00110000:
				_offset_bit = 6;
				break;
			case 0b00111000:
				_offset_bit = 7;
				break;
			}

			switch (_opcode & 0b00000111)
			{
			case 0b00000000:
				if ((registers.m_registers[2] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				break;
			case 0b00000001:
				if ((registers.m_registers[3] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				break;
			case 0b00000010:
				if ((registers.m_registers[4] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				break;
			case 0b00000011:
				if ((registers.m_registers[5] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				break;
			case 0b00000100:

				if ((registers.m_registers[6] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				break;
			case 0b00000101:
				if ((registers.m_registers[7] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				break;
			case 0b00000110:
				if ((mmu.Read(registers.HL) & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				break;
			case 0b00000111:
				if ((registers.m_registers[0] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				break;
			}

			return;
		}
		if ((_opcode & 0b11000000) == 0b11000000 || (_opcode & 0b11000000) == 0b10000000)
		{
			int _offset_bit;
			switch (_opcode & 0b00111000) 
			{
			case 0b00000000:
				_offset_bit = 0;
				break;
			case 0b00001000:
				_offset_bit = 1;
				break;
			case 0b00010000:
				_offset_bit = 2;
				break;
			case 0b00011000:
				_offset_bit = 3;
				break;
			case 0b00100000:
				_offset_bit = 4;
				break;
			case 0b00101000:
				_offset_bit = 5;
				break;
			case 0b00110000:
				_offset_bit = 6;
				break;
			case 0b00111000:
				_offset_bit = 7;
				break;
			}

			switch (_opcode & 0b00000111)
			{
			case 0b00000000:
				if ((registers.m_registers[2] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				if ((_opcode & 0b01000000) == 0b01000000)
				{
					registers.m_registers[2] |= (0b00000001 << _offset_bit);
				}
				else 
				{
					uint8_t _res = (0b00000001 << _offset_bit);
					registers.m_registers[2] &= !_res;
				}
				break;
			case 0b00000001:
				if ((registers.m_registers[3] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				if ((_opcode & 0b01000000) == 0b01000000)
				{
					registers.m_registers[3] |= (0b00000001 << _offset_bit);
				}
				else
				{
					uint8_t _res = (0b00000001 << _offset_bit);
					registers.m_registers[3] &= !_res;
				}
				break;
			case 0b00000010:
				if ((registers.m_registers[4] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				if ((_opcode & 0b01000000) == 0b01000000)
				{
					registers.m_registers[4] |= (0b00000001 << _offset_bit);
				}
				else
				{
					uint8_t _res = (0b00000001 << _offset_bit);
					registers.m_registers[4] &= !_res;
				}
				break;
			case 0b00000011:
				if ((registers.m_registers[5] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				if ((_opcode & 0b01000000) == 0b01000000)
				{
					registers.m_registers[5] |= (0b00000001 << _offset_bit);
				}
				else
				{
					uint8_t _res = (0b00000001 << _offset_bit);
					registers.m_registers[5] &= !_res;
				}
				break;
			case 0b00000100:

				if ((registers.m_registers[6] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				if ((_opcode & 0b01000000) == 0b01000000)
				{
					registers.m_registers[6] |= (0b00000001 << _offset_bit);
				}
				else
				{
					uint8_t _res = (0b00000001 << _offset_bit);
					registers.m_registers[6] &= !_res;
				}
				break;
			case 0b00000101:
				if ((registers.m_registers[7] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				if ((_opcode & 0b01000000) == 0b01000000)
				{
					registers.m_registers[7] |= (0b00000001 << _offset_bit);
				}
				else
				{
					uint8_t _res = (0b00000001 << _offset_bit);
					registers.m_registers[7] &= !_res;
				}
				break;
			case 0b00000110:
				if ((mmu.Read(registers.HL) & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				if ((_opcode & 0b01000000) == 0b01000000)
				{
					mmu.Write(registers.HL, mmu.Read(registers.HL) | (0b00000001 << _offset_bit));
				}
				else
				{
					uint8_t _res = (0b00000001 << _offset_bit);
					mmu.Write(registers.HL, mmu.Read(registers.HL) & !_res);
				}
				break;
			case 0b00000111:
				if ((registers.m_registers[0] & (0b00000001 << _offset_bit)) == 0)
				{
					registers.m_registers[1] |= 0b01000000;
				}
				else
				{
					registers.m_registers[1] &= !0b01000000;
				}
				if ((_opcode & 0b01000000) == 0b01000000)
				{
					registers.m_registers[0] |= (0b00000001 << _offset_bit);
				}
				else
				{
					uint8_t _res = (0b00000001 << _offset_bit);
					registers.m_registers[0] &= !_res;
				}
				break;
			}
			return;
		}
	}
};

#pragma endregion

#pragma region INC/DEC Instructions

class IF_INC_DEC final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return (opcode & 0b11000111) == 0b00000011 || (opcode & 0b11000110) == 0b00000100;
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
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
				throw std::exception("INC/DEC opcode problem !");
				_res = 1;
			}

			switch (opcode & 0b00110000) 
			{
			case 0b00000000:
				registers.BC += _res;
				break;
			case 0b00010000:
				registers.DE += _res;
				break;
			case 0b00100000:
				registers.HL += _res;
				break;
			case 0b00110000:
				registers.m_registers[0] += _res;
				break;
			}
			return;
		}
		if ((opcode & 0b11000110) == 0b00000100) 
		{
			int D = (opcode & 0b00000001);
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
				throw std::exception("INC/DEC opcode problem !");
				_res = 1;
			}

			switch (opcode & 0b00111000)
			{
			case 0b00000000:
				registers.m_registers[2] += _res;
				break;
			case 0b00001000:
				registers.m_registers[3] += _res;
				break;
			case 0b00010000:
				registers.m_registers[4] += _res;
				break;
			case 0b00011000:
				registers.m_registers[5] += _res;
				break;
			case 0b00100000:
				registers.m_registers[6] += _res;
				break;
			case 0b00101000:
				registers.m_registers[7] += _res;
				break;
			case 0b00110000:
				mmu.Write(registers.HL, mmu.Read(registers.HL) + _res);
				break;
			case 0b00111000:
				registers.m_registers[0] += _res;
				break;
			}
			return;
		}
	}
};

#pragma endregion

#pragma region Push/Pop Instructions

class IF_PUSH_POP final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override
	{
		return (opcode & 0b11001011) == 0b11000001;
	}

	void Execute(uint8_t opcode, MMU& mmu, Registers& registers) override
	{
		switch (opcode & 0b00110000)
		{
		case 0b00000000:
			if ((opcode & 0b00000100) == 0b00000000) 
			{
				registers.BC = (((mmu.Read(registers.SP--)) << 4) + mmu.Read(registers.SP--));
			}
			else 
			{
				mmu.Write(registers.SP - 1, registers.BC >> 4);
				mmu.Write(registers.SP, registers.BC);
				registers.BC = (((mmu.Read(registers.SP - 1)) << 4) + mmu.Read(registers.SP - 2));
			}
			break;
		case 0b00010000:
			if ((opcode & 0b00000100) == 0b00000000)
			{
				registers.DE = (((mmu.Read(registers.SP--)) << 4) + mmu.Read(registers.SP--));
			}
			else
			{
				mmu.Write(registers.SP - 1, registers.DE >> 4);
				mmu.Write(registers.SP, registers.DE);
				registers.DE = (((mmu.Read(registers.SP - 1)) << 4) + mmu.Read(registers.SP - 2));
			}
			break;
		case 0b00100000:
			if ((opcode & 0b00000100) == 0b00000000)
			{
				registers.HL = (((mmu.Read(registers.SP--)) << 4) + mmu.Read(registers.SP--));
			}
			else
			{
				mmu.Write(registers.SP - 1, registers.HL >> 4);
				mmu.Write(registers.SP, registers.HL);
				registers.HL = (((mmu.Read(registers.SP - 1)) << 4) + mmu.Read(registers.SP - 2));
			}
			break;
		case 0b00110000:
			if ((opcode & 0b00000100) == 0b00000000)
			{
				registers.AF = (((mmu.Read(registers.SP--)) << 4) + mmu.Read(registers.SP--));
			}
			else
			{
				mmu.Write(registers.SP - 1, registers.AF >> 4);
				mmu.Write(registers.SP, registers.AF);
				registers.AF = (((mmu.Read(registers.SP - 1)) << 4) + mmu.Read(registers.SP - 2));
			}
			break;
		}
	}
};

#pragma endregion