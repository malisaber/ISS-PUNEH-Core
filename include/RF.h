#include <systemc.h>
#include "Utilities.h"
#include "channel.h"

#ifndef _PUNEH_REGISTER_FILE_
#define _PUNEH_REGISTER_FILE_

class RF: public sc_module
{
	int val[RF_offset];
public:
	sc_port< slave_port > main_port;
	sc_port< sc_signal_in_if< sc_logic >, 1 > rst;

	SC_CTOR(RF)
	{
		SC_METHOD(initiate);
		SC_METHOD(reseting); sensitive << rst;
		SC_THREAD(responding);
	}
	void responding();
	void reseting();
	void initiate();
};


#endif // !_PUNEH_REGISTER_FILE_
