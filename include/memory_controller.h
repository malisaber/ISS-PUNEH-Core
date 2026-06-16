#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "channel.h"
#include "CommonHeader.h"

#ifndef _MY_MEMORY_CONTROLLER_H_
#define _MY_MEMORY_CONTROLLER_H_

class memory_controller : public sc_module
{
public:
	sc_port< sc_signal_in_if< sc_logic >, 1 > clk, rst;
	sc_port< slave_port > main_port;
	tlm_utils::simple_initiator_socket<memory_controller> MCU_socket;

	transaction_info info;
	tlm::tlm_generic_payload* access_request;
	unsigned char data[2];

	SC_CTOR(memory_controller) : MCU_socket("MCU_socket"), access_request(0)
	{
		access_request = new tlm::tlm_generic_payload;
		SC_THREAD(responding);
	}

	void responding();
};

#endif
