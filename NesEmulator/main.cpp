#include <iostream>
#include "cpu.h"

using namespace std;

int main()
{
	CPU cpu;
	bool ret = cpu.LoadROM("../TestROMs/super_mario_brothers.nes");
	cpu.MainLoop();

	return 0;
}