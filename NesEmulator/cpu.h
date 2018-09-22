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

	/* Decodes opcode and executes it */
	void DecodeExecuteOpcode();

private:
	uint8_t memory[MEMORY_SIZE];

	uint8_t currentOpcode;

	uint16_t pc; // program counter
	uint8_t  sp; // stack pointer
	uint8_t  A;  // accumulator
	uint8_t  X;  // index register X
	uint8_t  Y;  // index register Y
	uint8_t  P;  // processor status
};