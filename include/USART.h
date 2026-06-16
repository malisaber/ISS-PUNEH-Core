#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include "Utilities.h"
#include "channel.h"

#ifndef _MY_USART_H_
#define _MY_USART_H_

#define UTDR_ofst 0
#define URDR_ofst 1
#define USCR_ofst 2
#define UBRR_ofst 3

#define TXEn 0
#define RXEn 1
#define RXC 2
#define TXC 3

class USART : public sc_module
{
public:
	sc_port< sc_signal_in_if< sc_logic >, 1 > clk, rst;
	sc_port< slave_port > main_port;
	sc_port<sc_signal_in_if<sc_logic>, 1> RX;
	sc_port<sc_signal_out_if<sc_logic>, 1> TX;


	PUNEH_register UTDR, URDR, USCR, UBRR;
	transaction_info info;

	sc_logic new_data;
	sc_lv<10> inp_frame, out_frame;
	sc_event TRx_event;

	SC_CTOR(USART)
	{
		SC_THREAD(accessing);
		SC_THREAD(Transferring); sensitive << TRx_event;
		SC_THREAD(Receiving); sensitive << TRx_event;
		SC_THREAD(BoudRateGen); sensitive << clk;
	}

	void accessing();
	void BoudRateGen();
	void Transferring();
	void Receiving();
};

#endif
