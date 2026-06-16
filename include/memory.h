#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "CommonHeader.h"

#ifndef _MY_MEMORY_H_
#define _MY_MEMORY_H_

SC_MODULE(memory)
{
	tlm_utils::simple_target_socket<memory> Memory_socket;
	sc_lv<16> mem[4095];

	SC_CTOR(memory)
	{
		Memory_socket.register_b_transport(this, &memory::b_transport); 

		SC_METHOD(initiate);
	}

public:
	virtual void b_transport(tlm::tlm_generic_payload&, sc_time&);
	void initiate();
};

#endif
