#include <iostream>
#include "cpu.h"

using namespace std;

int main()
{
	CPU cpu;
	cpu.LoadROM("../TestROMs/cpu_dummy_reads.nes");
	cpu.MainLoop();

	return 0;
}