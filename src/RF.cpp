#include "RF.h"

void RF::initiate()
{
	for (int i = 0; i < RF_offset; i++)
	{
		val[i] = 0XDEAD;
	}
}

void RF::reseting()
{
	for (int i = 0; i < RF_offset; i++)
	{
		val[i] = 0;
	}
}

void RF::responding()
{
	transaction_info info;
	while (true)
	{
		if (rst->read() == sc_logic_1)
		{
			wait(rst->default_event());
		}
		else
		{
			main_port->wait_for_call(RF_Base, RF_Base + RF_offset - 1);
			main_port->get_info(info);

			if (rst->read() == sc_logic_1) continue;

			if (info.op == read)
				main_port->put(val[info.address.to_uint() - RF_Base]);
			else
				main_port->get(val[info.address.to_uint() - RF_Base]);
		}
	}
}
