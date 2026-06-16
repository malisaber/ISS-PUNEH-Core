#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "system.h"

void my_system::clocking()
{
	clk.write(sc_logic_0);

	while (1)
	{
		wait(1, SC_NS);
		if (clk.read() == sc_logic_0)
			clk.write(sc_logic_1);
		else
			clk.write(sc_logic_0);
	}
}

void my_system::reseting()
{
	rst.write(sc_logic_1);
	wait(10, SC_NS);
	rst.write(sc_logic_0);
	wait();

}

void my_system::interrupting()
{
	interrupt.write(sc_logic_0);
	while (true)
	{
		wait(111, SC_NS);
		interrupt.write(sc_logic_1);
		wait(10, SC_NS);
		interrupt.write(sc_logic_0);
	}

}
