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

uint16_t CPU::GetIndirectAddress()
{
	uint8_t lowByte = GetOperand();
	uint8_t highByte = GetOperand();

	uint16_t tempAddress = highByte;
	tempAddress <<= 8;
	tempAddress |= lowByte;

	uint8_t lowByteIndirect = memory[tempAddress];
	uint8_t highByteIndirect = memory[tempAddress + 1];

	uint16_t finalAddress = highByteIndirect;
	finalAddress <<= 8;
	finalAddress |= lowByteIndirect;

	pc = finalAddress;

	return finalAddress;
}

uint16_t CPU::GetXIndexedAbsoluteOperand()
{
	uint16_t absoluteAddress = GetAbsOperand();
	absoluteAddress += X;

	return absoluteAddress;
}

uint16_t CPU::GetYIndexedAbsoluteOperand()
{
	uint16_t absoluteAddress = GetAbsOperand();
	absoluteAddress += Y;

	return absoluteAddress;
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

uint8_t CPU::GetImmidiateOperand()
{
	return GetOperand();
}

uint16_t CPU::GetImmidiateOperand16()
{
	uint8_t lowByte = GetOperand();
	uint8_t highByte = GetOperand();

	uint16_t result = highByte;
	result <<= 8;
	result |= lowByte;

	return result;
}

void CPU::PushToStack(uint8_t value)
{
	memory[sp] = value;
	--sp;

	if (sp < 0x0100) // wrap around when there is overflow
		sp = 0x01FF;
}

uint8_t CPU::PopFromStack()
{
	return memory[sp++];
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

/* BCC  Branch on carry bit clear */
void CPU::BCC(uint8_t operand)
{
	++pc;
	if (C == 0)
		pc += operand;
}

/* BCS  Branch on carry bit set */
void CPU::BCS(uint8_t operand)
{
	++pc;
	if (C == 1)
		pc += operand;
}

/* BEQ  Branch on result zero */
void CPU::BEQ(uint8_t operand)
{
	++pc;
	if (Z == 1)
		pc += operand;
}

/* BMI  Branch on result minus */
void CPU::BMI(uint8_t operand)
{
	++pc;
	if (N == 1)
		pc += operand;
}

/* BNE  Branch on result not zero */
void CPU::BNE(uint8_t operand)
{
	++pc;
	if (Z == 0)
		pc += operand;
}

/* BPL  Branch on rezult plus */
void CPU::BPL(uint8_t operand)
{
	++pc;
	if (N == 0)
		pc += operand;
}

/* BVC  Branch on overflow bit clear */
void CPU::BVC(uint8_t operand)
{
	++pc;
	if (V == 0)
		pc += operand;
}

/* BVS  Branch on overflow bit set */
void CPU::BVS(uint8_t operand)
{
	++pc;
	if (V == 1)
		pc += operand;
}

/* BRK  Force Break */
void CPU::BRK()
{
	I = true;
	pc += 2;
	PushToStack(pc);
	PushToStack(CreateStatusRegister());
}

/* RTI  Return from Interrupt */
void CPU::RTI()
{
	// @TODO: FINISH THIS
	uint8_t statusReg = PopFromStack();
	pc = PopFromStack();
}

/* RTS  Return from Subroutine */
void CPU::RTS()
{
	pc = PopFromStack() + 1;
}

/* PHP  Push Processor Status on Stack */
void CPU::PHP()
{
	PushToStack(P);
}

/* CLC  Clear Carry Flag */
void CPU::CLC()
{
	C = false;
}

/* PLP  Pull Processor Status from Stack */
void CPU::PLP() 
{ 
	sp = PopFromStack(); 
}

/* SEC  Set Carry Flag */
void CPU::SEC() 
{ 
	C = true; 
}

/* PHA  Push Accumulator on Stack */
void CPU::PHA() 
{ 
	PushToStack(A); 
}

/* CLI  Clear Interrupt Disable Bit */
void CPU::CLI() 
{ 
	I = false; 
}

/* PLA  Pull Accumulator from Stack */
void CPU::PLA()
{
	A = PopFromStack();
	SetNegativeFlag(A);
	SetZeroFlag(A);
}

/* SEI  Set Interrupt Disable Status */
void CPU::SEI() 
{ 
	I = true; 
}

/* DEY  Decrement Index Y by One */
void CPU::DEY()
{
	--Y;
	SetNegativeFlag(Y);
	SetZeroFlag(Y);
}

/* TYA  Transfer Index Y to Accumulator */
void CPU::TYA()
{
	A = Y;
	SetNegativeFlag(A);
	SetZeroFlag(A);
}

/* TAY  Transfer Accumulator to Index Y */
void CPU::TAY()
{
	Y = A;
	SetNegativeFlag(Y);
	SetZeroFlag(Y);
}

/* CLV  Clear Overflow Flag */
void CPU::CLV()
{
	V = false;
}

/* INY  Increment Index Y by One */
void CPU::INY()
{
	++Y;
	SetNegativeFlag(Y);
	SetZeroFlag(Y);
}

/* CLD  Clear Decimal Mode */
void CPU::CLD()
{
	D = false;
}

/* INX  Increment Index X by One */
void CPU::INX()
{
	++X;
	SetNegativeFlag(X);
	SetZeroFlag(X);
}

/* SED  Set Decimal Flag */
void CPU::SED()
{
	D = true;
}

/* TXA  Transfer Index X to Accumulator */
void CPU::TXA()
{
	A = X;
	SetNegativeFlag(A);
	SetZeroFlag(A);
}

/* TXS  Transfer Index X to Stack Register */
void CPU::TXS()
{
	sp = X;
}

/* TAX  Transfer Accumulator to Index X */
void CPU::TAX()
{
	X = A;
	SetNegativeFlag(X);
	SetZeroFlag(X);
}

/* TSX  Transfer Stack Pointer to Index X */
void CPU::TSX()
{
	X = sp;
	SetNegativeFlag(X);
	SetZeroFlag(X);
}

/* DEX  Decrement Index X by One */
void CPU::DEX()
{
	--X;
	SetNegativeFlag(X);
	SetZeroFlag(X);
}

/* NOP  No Operation */
void CPU::NOP()
{
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

	/* X-indexed absolute addressing mode */

	case 0x7D: ADC(memory[GetXIndexedAbsoluteOperand()]);	break;
	case 0x3D: AND(memory[GetXIndexedAbsoluteOperand()]);	break;
	case 0x1E: ASL(&memory[GetXIndexedAbsoluteOperand()]);	break;
	case 0xDD: CMP(memory[GetXIndexedAbsoluteOperand()]);	break;
	case 0xDE: DEC(GetXIndexedAbsoluteOperand());			break;
	case 0x5D: EOR(memory[GetXIndexedAbsoluteOperand()]);	break;
	case 0xFE: INC(GetXIndexedAbsoluteOperand());			break;
	case 0xBD: LDA(GetXIndexedAbsoluteOperand());			break;
	case 0xBC: LDY(GetXIndexedAbsoluteOperand());			break;
	case 0x5E: LSR(&memory[GetXIndexedAbsoluteOperand()]);	break;
	case 0x1D: ORA(memory[GetXIndexedAbsoluteOperand()]);	break;
	case 0x3E: ROL(&memory[GetXIndexedAbsoluteOperand()]);	break;
	case 0x7E: ROR(&memory[GetXIndexedAbsoluteOperand()]);	break;
	case 0xFD: SBC(memory[GetXIndexedAbsoluteOperand()]);	break;
	case 0x9D: STA(GetXIndexedAbsoluteOperand());			break;

	/* Y-indexed absolute addressing mode */

	case 0x79: ADC(memory[GetYIndexedAbsoluteOperand()]);	break;
	case 0x39: AND(memory[GetYIndexedAbsoluteOperand()]);	break;
	case 0xD9: CMP(memory[GetYIndexedAbsoluteOperand()]);	break;
	case 0x59: EOR(memory[GetYIndexedAbsoluteOperand()]);	break;
	case 0xB9: LDA(GetYIndexedAbsoluteOperand());			break;
	case 0xBE: LDX(GetYIndexedAbsoluteOperand());			break;
	case 0x19: ORA(memory[GetYIndexedAbsoluteOperand()]);	break;
	case 0xF9: SBC(memory[GetYIndexedAbsoluteOperand()]);	break;
	case 0x99: STA(GetYIndexedAbsoluteOperand());			break;

	/* Indirect addressing mode */

	case 0x6C: JMP(GetIndirectAddress());	break;

	/* Immidiate addressing mode */

	case 0x69: ADC(GetImmidiateOperand());	break;
	case 0x29: AND(GetImmidiateOperand());	break;
	case 0xC9: CMP(GetImmidiateOperand());	break;
	case 0xE0: CPX(GetImmidiateOperand());	break;
	case 0xC0: CPY(GetImmidiateOperand());	break;
	case 0x49: EOR(GetImmidiateOperand());	break;
	case 0xA9: LDA(GetImmidiateOperand16());	break;
	case 0xA2: LDX(GetImmidiateOperand16());	break;
	case 0xA0: LDY(GetImmidiateOperand16());	break;
	case 0x09: ORA(GetImmidiateOperand());	break;
	case 0xE9: SBC(GetImmidiateOperand());	break;

	/* Relative addressing mode */

	case 0x90: BCC(GetOperand());	break;
	case 0xB0: BCS(GetOperand());	break;
	case 0xF0: BEQ(GetOperand());	break;
	case 0x30: BMI(GetOperand());	break;
	case 0xD0: BNE(GetOperand());	break;
	case 0x10: BPL(GetOperand());	break;
	case 0x50: BVC(GetOperand());	break;
	case 0x70: BVS(GetOperand());	break;

	/* Implied addressing mode */
	case 0x00:  BRK();  break;
	case 0x40:  RTI();  break;
	case 0x60:  RTS();  break;
	case 0x08:  PHP();  break;
	case 0x18:  CLC();  break;
	case 0x28:  PLP();  break;
	case 0x38:  SEC();  break;
	case 0x48:  PHA();  break;
	case 0x58:  CLI();  break;
	case 0x68:  PLA();  break;
	case 0x78:  SEI();  break;
	case 0x88:  DEY();  break;
	case 0x98:  TYA();  break;
	case 0xA8:  TAY();  break;
	case 0xB8:  CLV();  break;
	case 0xC8:  INY();  break;
	case 0xD8:  CLD();  break;
	case 0xE8:  INX();  break;
	case 0xF8:  SED();  break;
	case 0x8A:  TXA();  break;
	case 0x9A:  TXS();  break;
	case 0xAA:  TAX();  break;
	case 0xBA:  TSX();  break;
	case 0xCA:  DEX();  break;
	case 0xEA:  NOP();  break;

	default:
		std::cout << "Invalid opcode " << std::hex << currentOpcode << std::endl;
	}

	UpdatePC();
}