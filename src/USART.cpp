#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "USART.h"

void USART::accessing()
{
	wait(0, SC_NS);
	UTDR.put_value(0);
	URDR.put_value(0);
	USCR.put_value(1<<TXC);
	UBRR.put_value(0);
	new_data = 0;

	while (1)
	{
		if (rst->read() == sc_logic_1)
		{
			wait(rst->default_event());
		}
		else
		{
			main_port->wait_for_call(USART_Base, USART_Base + USART_offset - 1);
			main_port->get_info(info);

			if (rst->read() == sc_logic_1) continue;

			switch (info.address.to_uint() - USART_Base)
			{
			case UTDR_ofst:
				if (info.op == read)
					main_port->put(UTDR.get_value());
				else if (info.op == write)
				{
					main_port->get(UTDR.put_value());
					new_data = 1;
					USCR.put_value(USCR.get_value() & (~(1 << TXC)));
				}
				break;
			case URDR_ofst:
				if (info.op == read)
				{
					main_port->put(URDR.get_value());
					USCR.put_value(USCR.get_value() & (~(1 << RXC)));
				}
				else if (info.op == write)
					main_port->get(URDR.put_value());
				break;
			case USCR_ofst:
				if (info.op == read)
					main_port->put(USCR.get_value());
				else if (info.op == write)
				{
					sc_lv<16> temp;
					int tmp_TXC;
					int tmp_RXC;

					main_port->get(temp);
					tmp_TXC = USCR.get_value(TXC, TXC);
					tmp_RXC = USCR.get_value(RXC, RXC);


					if (tmp_TXC == 0)
						temp.range(TXC, TXC) = 0;
					else
						if (temp.range(TXC, TXC) == 1)
							temp.range(TXC, TXC) = 0;
						else
							temp.range(TXC, TXC) = 1;


					if (tmp_RXC == 0)
						temp.range(RXC, RXC) = 0;
					else
						if (temp.range(RXC, RXC) == 1)
							temp.range(RXC, RXC) = 0;
						else
							temp.range(RXC, RXC) = 1;

					USCR.put_value(temp);
				}
					
				break;
			case UBRR_ofst:
				if (info.op == read)
					main_port->put(UBRR.get_value());
				else if (info.op == write)
					main_port->get(UBRR.put_value());
				break;
			}
		}
	}
}

void USART::BoudRateGen()
{
	int cntr = 0;
	while (1)
	{
		if ((clk->read() == sc_logic_1) && clk->event())
		{
			if (cntr == 0)
			{
				cntr = -1 * UBRR.get_value();
				TRx_event.notify();
			}
			else
				cntr++;
		}
		wait();
	}
}

void USART::Transferring()
{
	out_frame = -1;
	TX->write(sc_logic_1);
	int shift_cntr = 0;
	while (1)
	{
		if (USCR.get_value(TXEn, TXEn) != 0)
		{
			if (shift_cntr != 0)
			{
				if (out_frame.to_uint() % 2)
					TX->write(sc_logic_1);
				else
					TX->write(sc_logic_0);

				out_frame = out_frame.to_uint() / 2;
				shift_cntr++;
				if (shift_cntr == 0)
				{
					USCR.put_value(USCR.get_value() | (1 << TXC));
				}
			}
			else
			{
				if (new_data == sc_logic_1)
				{
					out_frame = 512 + 2 * UTDR.get_value(7, 0);
					new_data = sc_logic_0;
					shift_cntr = -10;
				}
			}
		}
		wait();
	}
}

void USART::Receiving()
{
	inp_frame = 0;
	int shift_cntr = 0;
	while (1)
	{
		if (USCR.get_value(RXEn, RXEn) != 0)
		{
			if (shift_cntr != 0)
			{
				inp_frame = inp_frame.to_uint() / 2 + RX->read().to_bool() * 512;
				shift_cntr++;
				if (shift_cntr == 0)
				{
					USCR.put_value(USCR.get_value() | (1 << RXC));
					URDR.put_value(inp_frame.range(8, 1));
				}
			}
			else
			{
				if (RX->read() == sc_logic_0)
				{
					shift_cntr = -10;
					inp_frame = inp_frame.to_uint() / 2 + RX->read().to_bool() * 512;
					shift_cntr++;
				}
			}
		}
		wait();
	}
}
