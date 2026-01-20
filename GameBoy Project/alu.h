#pragma once
#include <cstdint>
#include <memory>
#include <vector>

class MMU;
struct Registers;

class InstructionFamily
{
protected:
	int m_step = 0;
	uint8_t m_8bitRegister = 0;
	uint16_t m_16bitRegister = 0;
	
public:
	virtual bool IsValid(uint8_t opcode) = 0;
	virtual bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) = 0;
};

class ALU 
{
public:
	~ALU();
	std::vector<std::unique_ptr<InstructionFamily>> m_instructionFamilies;
};

#pragma region Load Instructions

class IF_LD_r16_imm16 final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

class IF_LD_r8_imm8 final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

class IF_LD_rA_memory final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

class IF_LD_rA_rHL final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

class IF_LD_r_r final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

class IF_LD_SP_HL final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

class IF_LD_ADRC_r final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

class IF_LD_ADRIMM_r final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

#pragma endregion

#pragma region Arithmetique Instructions

class IF_AR_8BIT final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

class IF_ADD_HL final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

#pragma endregion

#pragma region Flow Instructions

class IF_FLOW_JR final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

class IF_FLOW_JP final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

class IF_FLOW_CALL final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

class IF_FLOW_RET final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

#pragma endregion

#pragma region Rotate Instructions

class IF_ROTATE final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

#pragma endregion

#pragma region CB Prefix Instructions

class IF_CB_Prefix final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t _, MMU& mmu, Registers& registers) override;
};

#pragma endregion

#pragma region INC/DEC Instructions

class IF_INC_DEC final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

#pragma endregion

#pragma region Push/Pop Instructions

class IF_POP final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

class IF_PUSH final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

#pragma endregion

#pragma region Interrupts Instructions

class IF_DI_EI final
	: public InstructionFamily
{
public:
	bool IsValid(uint8_t opcode) override;
	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

#pragma endregion

#pragma region Custom Instruction

class IF_Finish final //Deprecated
	: public InstructionFamily
{
public:

	bool IsValid(uint8_t opcode) override;

	bool Tick(uint8_t opcode, MMU& mmu, Registers& registers) override;
};

#pragma endregion
