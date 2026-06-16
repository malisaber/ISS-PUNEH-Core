#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "system.h"

int sc_main(int argc, char **argv)
{	
	my_system UUT("my_system");

	sc_start(5000,SC_NS);
	return 0;
}
