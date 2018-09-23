#include "cpu.h"

#include <fstream>

CPU::CPU()
{
	currentOpcode = 0;

	/*PRG start*/
	pc = 0x8000;

	prg_rom_size = 1;
	prg_ram_size = 1;
	chr_size = 1;

	sp = 0x01FF; // actually 0x01FF 
	A = 0;  
	X = 0;  
	Y = 0;  
	P = 0;

	memset(memory, 0, MEMORY_SIZE);
	/*for (int i = 0; i < MEMORY_SIZE; ++i)
		memory[i] = 0;*/
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
	uint16_t counter = 0;
	uint8_t data = 0;

	while (!inputFile.eof())
	{
		if (location >= MEMORY_SIZE - 1)
			return false;

		inputFile.read((char*)&data, sizeof(uint8_t));

		if (counter < 16) {
			if (counter == 4) prg_rom_size = data;

			if (counter == 5) chr_size = data;

			if (counter == 6) { 
				is_trainer = !((data & 4) == 0);
			}
			/*if 8th byte of the header is 0 -> 8KB PRG RAM
			  else -> 8KB * 8th byte value*/

			if (counter == 8) {
				if (data != 0) prg_ram_size = data;
			}

			++counter;
			continue;
		}

		//loading trainer into memory
		if (is_trainer && !trainer_written) {
			int i = 0;
			location = 0x7000;
			while (i < 512) {
				memory[location] = data;
				inputFile.read((char*)&data, sizeof(uint8_t));
				++location;
				++i;
			}
			trainer_written = true;
			location = 0x8000;
		}

		//loading prg_rom into memory


		//for prg_rom
		memory[location] = data;
		++location;

		counter++;
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