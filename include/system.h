#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "channel.h"
#include "memory_controller.h"
#include "PUNEH.h"
#include "USART.h"
#include "memory.h"
#include "RF.h"


#ifndef _MY_PUNEH_SYSTEM_
#define _MY_PUNEH_SYSTEM_

class my_system : public sc_module
{
public:

	sc_signal<sc_logic> clk;
	sc_signal<sc_logic> rst;
	sc_signal<sc_logic> interrupt;
	sc_signal<sc_logic> TRx;

	PUNEH *PUNEH_MP;
	main_channel *CH;
	memory_controller *mem_ctrl;
	USART* my_usart;
	memory* mem;
	RF* reg_file;
	
	SC_CTOR(my_system)
	{
		mem = new memory("memory");

		CH = new main_channel;

		PUNEH_MP = new PUNEH("PUNEH_processor");
		PUNEH_MP->clk(clk);
		PUNEH_MP->rst(rst);
		PUNEH_MP->main_port(*CH);
		PUNEH_MP->interrupt(interrupt);

		mem_ctrl = new memory_controller("mem_controller_of_PUNEH");
		mem_ctrl->clk(clk);
		mem_ctrl->rst(rst);
		mem_ctrl->main_port(*CH);
		mem_ctrl->MCU_socket.bind(mem->Memory_socket);

		my_usart = new USART("my_usart");
		my_usart->clk(clk);
		my_usart->rst(rst);
		my_usart->main_port(*CH);
		my_usart->TX(TRx);
		my_usart->RX(TRx);

		reg_file = new RF("RF");
		reg_file->rst(rst);
		reg_file->main_port(*CH);

		SC_THREAD(clocking);
		SC_THREAD(reseting);
		SC_THREAD(interrupting);
	}

	void clocking();
	void reseting();
	void interrupting();
	
};

#endif
