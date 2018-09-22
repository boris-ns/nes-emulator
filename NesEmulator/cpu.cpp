#include "cpu.h"

#include <fstream>

CPU::CPU()
{
	currentOpcode = 0;

	pc = 0x8000; 
	sp = 0xFF; // actually 0x01FF 
	A = 0;  
	X = 0;  
	Y = 0;  
	P = 0;  

	for (int i = 0; i < MEMORY_SIZE; ++i)
		memory[i] = 0;
}

CPU::~CPU()
{
}

bool CPU::LoadROM(const std::string& path)
{
	// @TODO: this is just basic reading from file
	// .nes files have specific format
	// implement that here!

	std::ifstream inputFile(path, std::ios_base::binary);

	uint16_t location = 0x8000;

	while (!inputFile.eof())
	{
		if (location > MEMORY_SIZE - 1)
			return false;

		uint8_t data = 0;
		inputFile.read((char*)&data, sizeof(uint8_t));
		memory[location] = data;
		++location;
	}

	return true;
}

void CPU::MainLoop()
{
	while (true)
	{
		EmulateCycle();
	}
}

void CPU::EmulateCycle()
{
	currentOpcode = FetchOpcode();
	DecodeExecuteOpcode();
	UpdatePC();
}

void CPU::UpdatePC()
{
	++pc;
}

uint8_t CPU::FetchOpcode()
{
	return memory[pc];
}

void CPU::DecodeExecuteOpcode()
{

}