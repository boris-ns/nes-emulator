#include "cpu.h"

#include <fstream>
#include <iostream>

CPU::CPU()
{
	currentOpcode = 0;

	/*PRG start*/
	pc = 0x8000;

	prg_rom_size = 1;
	prg_ram_size = 1;
	chr_size = 1;

	sp = 0x01FF;
	A = 0;  
	X = 0;  
	Y = 0;  
	P = 0;

	N = false;
	V = false;
	B = false;
	D = false;
	I = false;
	Z = false;
	C = false;

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

uint8_t CPU::GetOperand()
{
	UpdatePC();
	return FetchOpcode();
}

uint16_t CPU::GetAbsOperand()
{
	uint8_t lowByte  = GetOperand();
	uint8_t highByte = GetOperand();

	uint16_t result = highByte;
	result <<= 8;
	result |= lowByte;

	return result;
}

uint16_t CPU::GetXIndexedZeroPageOperand()
{
	uint8_t lowByte = GetOperand();
	uint16_t completeAddress = 0x0000 | lowByte;

	completeAddress += X;

	/*Wraparound*/
	completeAddress &= 0x00FF;

	return completeAddress;
}

uint16_t CPU::GetYIndexedZeroPageOperand()
{
	uint8_t lowByte = GetOperand();
	uint16_t completeAddress = 0x0000 | lowByte;

	completeAddress += Y;

	/*Wraparound*/
	completeAddress &= 0x00FF;

	return completeAddress;
}

void CPU::PushToStack(uint8_t value)
{
	memory[sp] = value;
	--sp;

	if (sp < 0x0100) // wrap around when there is overflow
		sp = 0x01FF;
}

void CPU::SetCarryFlag(uint8_t result)
{
	// @TODO: implement this
}

void CPU::SetZeroFlag(uint8_t result)
{
	Z = (result == 0);
}

void CPU::SetNegativeFlag(uint8_t result)
{
	N = (result & 0b10000000);
}

void CPU::SetOverflowFlag(uint8_t result)
{
	// @TODO: implement this
}

/* ASL  Shift Left One Bit (Memory or Accumulator) */
void CPU::ASL(uint8_t* operand)
{
	C = *operand & 0x80; // set carry flag
	*operand <<= 1;
	SetZeroFlag(*operand);
	SetNegativeFlag(*operand);
}

/* ROL  Rotate One Bit Left (Memory or Accumulator) */
void CPU::ROL(uint8_t* operand)
{
	uint8_t mask = (*operand & 0x80) >> 7;
	*operand = (*operand << 1) | mask;
	C = mask;
	SetZeroFlag(*operand);
	SetNegativeFlag(*operand);
}

/* ROR  Rotate One Bit Right (Memory or Accumulator) */
void CPU::ROR(uint8_t* operand)
{
	uint8_t mask = (*operand & 0x01) << 7;
	*operand = (*operand >> 1) | mask;
	C = mask;
	SetZeroFlag(*operand);
	SetNegativeFlag(*operand);
}

/* LSR  Shift One Bit Right (Memory or Accumulator) */
void CPU::LSR(uint8_t* operand)
{
	C = *operand & 0x01;
	*operand >>= 1;
	SetZeroFlag(*operand);
}

/* STA  Store Accumulator in Memory */
void CPU::STA(uint16_t address)
{
	memory[address] = A;
}

/* STX  Store Index X in Memory */
void CPU::STX(uint16_t address)
{
	memory[address] = X;
}

/* STY  Store Index Y in Memory */
void CPU::STY(uint16_t address)
{
	memory[address] = Y;
}

/* LDA  Load Accumulator with Memory */
void CPU::LDA(uint16_t address)
{
	A = memory[address];
	SetNegativeFlag(A);
	SetZeroFlag(A);
}

/* LDX  Load Index X with Memory */
void CPU::LDX(uint16_t address)
{
	X = memory[address];
	SetNegativeFlag(X);
	SetZeroFlag(X);
}

/* LDY  Load Index Y with Memory */
void CPU::LDY(uint16_t address)
{
	Y = memory[address];
	SetNegativeFlag(X);
	SetZeroFlag(X);
}

/* CMP  Compare Memory with Accumulator */
void CPU::CMP(uint8_t operand)
{
	uint8_t result = A - operand;
	SetNegativeFlag(result);
	SetZeroFlag(result);
	SetCarryFlag(result);
}

/* CPX  Compare Memory and Index X */
void CPU::CPX(uint8_t operand)
{
	uint8_t result = X - operand;
	SetNegativeFlag(result);
	SetZeroFlag(result);
	SetCarryFlag(result);
}

/* CPY  Compare Memory and Index Y */
void CPU::CPY(uint8_t operand)
{
	uint8_t result = Y - operand;
	SetNegativeFlag(result);
	SetZeroFlag(result);
	SetCarryFlag(result);
}

/* ORA  OR Memory with Accumulator */
void CPU::ORA(uint8_t operand)
{
	A |= operand;
	SetNegativeFlag(A);
	SetZeroFlag(A);
}

/* AND  AND Memory with Accumulator */
void CPU::AND(uint8_t operand)
{
	A &= operand;
	SetNegativeFlag(A);
	SetZeroFlag(A);
}

/* EOR  Exclusive-OR Memory with Accumulator */
void CPU::EOR(uint8_t operand)
{
	A ^= operand;
	SetNegativeFlag(A);
	SetZeroFlag(A);
}

/* ADC  Add Memory to Accumulator with Carry */
// @TODO: check if correct ?
void CPU::ADC(uint8_t operand)
{
	A += operand + C;
	SetNegativeFlag(A);
	SetZeroFlag(A);
	SetCarryFlag(A);
	SetOverflowFlag(A);
}

/* SBC  Subtract Memory from Accumulator with Borrow */
void CPU::SBC(uint8_t operand)
{
	A -= operand - C;
	SetNegativeFlag(A);
	SetZeroFlag(A);
	SetCarryFlag(A);
	SetOverflowFlag(A);
}

/* BIT  Test Bits in Memory with Accumulator */
void CPU::BIT(uint8_t operand)
{
	uint8_t result = A & operand;
	N = operand & 0b10000000;
	V = operand & 0b01000000;
	SetZeroFlag(result);
}

/* DEC  Decrement Memory by One */
void CPU::DEC(uint16_t address)
{
	memory[address] -= 1;
	SetNegativeFlag(memory[address]);
	SetZeroFlag(memory[address]);
}

/* INC  Increment Memory by One */
void CPU::INC(uint16_t address)
{
	memory[address] += 1;
	SetNegativeFlag(memory[address]);
	SetZeroFlag(memory[address]);
}

/* JMP  Jump to New Location */
void CPU::JMP(uint16_t address)
{
	pc = address;
}

/* JSR  Jump to New Location Saving Return Address */
void CPU::JSR(uint16_t address)
{
	PushToStack(pc);
	pc = address;
}

void CPU::DecodeExecuteOpcode()
{
	switch (currentOpcode)
	{
	/* Accumulator addressing mode */
	case 0x0A:  ASL(&A);  break;
	case 0x2A:  ROL(&A);  break;
	case 0x4A:  LSR(&A);  break;
	case 0x6A:  ROR(&A);  break;

	/* Zero page addressing mode */
	case 0x05:  ORA(memory[GetOperand()]);   break;
	case 0x06:  ASL(&memory[GetOperand()]);  break;
	case 0x24:  BIT(memory[GetOperand()]);   break;
	case 0x25:  AND(memory[GetOperand()]);   break;
	case 0x26:  ROL(&memory[GetOperand()]);  break;
	case 0x45:  EOR(memory[GetOperand()]);   break;
	case 0x46:  LSR(&memory[GetOperand()]);  break;
	case 0x65:  ADC(memory[GetOperand()]);   break;
	case 0x66:  ROR(&memory[GetOperand()]);  break;
	case 0x84:  STY(GetOperand());           break;
	case 0x85:  STA(GetOperand());           break;
	case 0x86:  STX(GetOperand());           break;
	case 0xA4:  LDY(GetOperand());           break;
	case 0xA5:  LDA(GetOperand());           break;
	case 0xA6:  LDX(GetOperand());           break;
	case 0xC4:  CPY(memory[GetOperand()]);   break;
	case 0xC5:  CMP(memory[GetOperand()]);   break;
	case 0xC6:  DEC(GetOperand());           break;
	case 0xE4:  CPX(memory[GetOperand()]);   break;
	case 0xE5:  SBC(memory[GetOperand()]);   break;
	case 0xE6:  INC(GetOperand());           break;

	/* X-indexed zero page addressing mode */

	case 0x75: ADC(memory[GetXIndexedZeroPageOperand()]);	break;
	case 0x35: AND(memory[GetXIndexedZeroPageOperand()]);	break;
	case 0x16: ASL(&memory[GetXIndexedZeroPageOperand()]);	break;
	case 0xD5: CMP(memory[GetXIndexedZeroPageOperand()]);	break;
	case 0xD6: DEC(GetXIndexedZeroPageOperand());			break;
	case 0x55: EOR(memory[GetXIndexedZeroPageOperand()]);	break;
	case 0xF6: INC(GetXIndexedZeroPageOperand());			break;
	case 0xB5: LDA(GetXIndexedZeroPageOperand());			break;
	case 0xB4: LDY(GetXIndexedZeroPageOperand());			break;
	case 0x56: LSR(&memory[GetXIndexedZeroPageOperand()]);	break;
	case 0x15: ORA(memory[GetXIndexedZeroPageOperand()]);	break;
	case 0x36: ROL(&memory[GetXIndexedZeroPageOperand()]);	break;
	case 0x76: ROR(&memory[GetXIndexedZeroPageOperand()]);	break;
	case 0xF5: SBC(memory[GetXIndexedZeroPageOperand()]);	break;
	case 0x95: STA(GetXIndexedZeroPageOperand());			break;
	case 0x94: STY(GetXIndexedZeroPageOperand());			break;

	/* Y-indexed zero page addressing mode */

	case 0xB6: LDX(GetYIndexedZeroPageOperand());	break;
	case 0x96: STX(GetYIndexedZeroPageOperand());	break;

	/* Absolute addressing mode */
	case 0x2C:  BIT(memory[GetAbsOperand()]);   break;
	case 0x8C:  STY(GetAbsOperand());           break;
	case 0xAC:  LDY(GetAbsOperand());           break;
	case 0xCC:  CPY(memory[GetAbsOperand()]);   break;
	case 0xEC:  CPX(memory[GetAbsOperand()]);   break;
	case 0x0D:  ORA(memory[GetAbsOperand()]);   break;
	case 0x2D:  AND(memory[GetAbsOperand()]);   break;
	case 0x4D:  EOR(memory[GetAbsOperand()]);   break;
	case 0x6D:  ADC(memory[GetAbsOperand()]);   break;
	case 0x8D:  STA(GetAbsOperand());           break;
	case 0xAD:  LDA(GetAbsOperand());           break;
	case 0xCD:  CMP(memory[GetAbsOperand()]);   break;
	case 0xED:  SBC(memory[GetAbsOperand()]);   break;
	case 0x0E:  ASL(&memory[GetAbsOperand()]);  break;
	case 0x2E:  ROL(&memory[GetAbsOperand()]);  break;
	case 0x4E:  LSR(&memory[GetAbsOperand()]);  break;
	case 0x6E:  ROR(&memory[GetAbsOperand()]);  break;
	case 0x8E:  STX(GetAbsOperand());           break;
	case 0xAE:  LDX(GetAbsOperand());           break;
	case 0xCE:  DEC(GetAbsOperand());           break;
	case 0xEE:  INC(GetAbsOperand());           break;
	case 0x4C:  JMP(GetAbsOperand());           break;
	case 0x20:  JSR(GetAbsOperand());           break;

	default:
		std::cout << "Invalid opcode " << std::hex << currentOpcode << std::endl;
	}

	UpdatePC();
}