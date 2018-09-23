#pragma once

#include <iostream>

#define MEMORY_SIZE 65536

class CPU
{
public:
	CPU();
	~CPU();

	/* Loads ROM into memory */
	bool LoadROM(const std::string& path);

	/* Main loop. From here emulation starts */
	void MainLoop();

	/* Emulates one cpu cycle */
	void EmulateCycle();

	/* Updates program counter to the next instruction */
	void UpdatePC();

	/* Gets current opcode  */
	uint8_t FetchOpcode();

	/* Updates PC and gets next operand */
	uint8_t GetOperand();

	/* Decodes opcode and executes it */
	void DecodeExecuteOpcode();

private:
	/*RAM*/
	uint8_t memory[MEMORY_SIZE];

	uint8_t currentOpcode;

	uint8_t prg_rom_size;
	uint8_t prg_ram_size; // saving progress and highscore
	uint8_t chr_size; // sprites

	bool is_trainer;
	bool trainer_written = false;

	uint16_t pc; // program counter
	uint16_t sp; // stack pointer
	uint8_t  A;  // accumulator
	uint8_t  X;  // index register X
	uint8_t  Y;  // index register Y
	uint8_t  P;  // processor status

	/* Status flags */
	bool N;
	bool V;
	bool B;
	bool D;
	bool I;
	bool Z;
	bool C;

	/* Methods for setting status flags */
	void SetCarryFlag(uint8_t result);
	void SetZeroFlag(uint8_t result);
	void SetNegativeFlag(uint8_t result);
	void SetOverflowFlag(uint8_t result);

	/* CPU instructions */
	void ASL(uint8_t* operand);
	void ROL(uint8_t* operand);
	void ROR(uint8_t* operand);
	void LSR(uint8_t* operand);
	void STA(uint16_t address);
	void STX(uint16_t address);
	void STY(uint16_t address);
	void LDA(uint16_t address);
	void LDX(uint16_t address);
	void LDY(uint16_t address);
	void CMP(uint8_t operand);
	void CPX(uint8_t operand);
	void CPY(uint8_t operand);
	void ORA(uint8_t operand);
	void AND(uint8_t operand);
	void EOR(uint8_t operand);
	void ADC(uint8_t operand);
	void SBC(uint8_t operand);
	void BIT(uint8_t operand);
	void DEC(uint16_t address);
	void INC(uint16_t address);
};