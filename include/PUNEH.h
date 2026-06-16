#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "Utilities.h"
#include "channel.h"
#include "SemiHosting.h"


#ifndef _MY_PUNEH_H_
#define _MY_PUNEH_H_

#define printing_enable false
#define SemiHosting_Enable true

class PUNEH : public sc_module, public intercept_library
{
public:
	sc_port< sc_signal_in_if< sc_logic >, 1 > clk, rst, interrupt;
	sc_port< master_port > main_port;

	PUNEH_register PC, IR, IN_, AC, OF;
	Flags flag;
	bool int_enable;
	bool req;

	int inst_cntr;
	const int PUNEH_priority = 0;
	PUNEH_inst inst_info;
	ALU_info alu_info;
	printing_info pinf;
	
	sc_event go;

	SC_CTOR(PUNEH)
	{
		SC_METHOD(req_handler); sensitive << interrupt;
		SC_THREAD(initial);
		SC_THREAD(operating); sensitive << go;
		SC_METHOD(main_port_reseting); sensitive << rst;
		SC_METHOD(go_generator); sensitive << rst << clk;
	}
	
	void initial();
	void main_port_reseting();
	void operating();
	void go_generator();
	void req_handler();


	void fetch();
	void instruction_decode();
	bool execute();
	void reseting();
	void indirect_operand();
	void Interrupt_routine();
	bool normal_instruction_execution();
	bool intercept_instruction_execution();
	void do_read(PUNEH_register &dest, int address);				//	do_read(destination_reg, mem_address)
	void do_read(PUNEH_register &dest, PUNEH_register address);		//	do_read(destination_reg, address_reg)
	int  do_read(int address);										//	just read the address
	void do_write(int address, PUNEH_register source);				//	do_write(mem_address, source_register)
	void do_write(PUNEH_register address, PUNEH_register source);	//	do_write(address_reg, source_register)
	void ALU_op();

	void init_semihost();

	bool check_flags();

	void update_ZN_flag();
	void update_CV_flag(update_CV_flag_type type);

	void printing();
	void PAC_print();
};

#endif
