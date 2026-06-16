#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "memory_controller.h"
#include "Utilities.h"

void memory_controller::responding()
{
	sc_time blocked_time = sc_time(0, SC_NS);
	tlm::tlm_command cmnd;
	sc_lv<16> ID;

	while (1)
	{
		if (rst->read() == sc_logic_1)
		{
			wait(rst->default_event());
		}
		else
		{
			main_port->wait_for_call(0, 4095);
			main_port->get_info(info);

			if (rst->read() == sc_logic_1) continue;


			if (info.op == read)
			{
				cmnd = tlm::TLM_READ_COMMAND;
				access_request->set_command(cmnd);
				access_request->set_address(info.address.to_uint64());
				access_request->set_data_ptr((unsigned char*)data);
				access_request->set_data_length(2);
				access_request->set_streaming_width(2);
				access_request->set_byte_enable_ptr(0);
				access_request->set_dmi_allowed(false);
				access_request->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
				MCU_socket->b_transport(*access_request, blocked_time);
				if (access_request->get_response_status() == tlm::TLM_OK_RESPONSE)
				{
					ID.range(7, 0) = data[0];
					ID.range(15, 8) = data[1];
					main_port->put(ID);
				}
				else
				{
					SC_REPORT_ERROR("TLM2", "Error in memory handling of MCU request\n");
				}
			}
			else
			{
				cmnd = tlm::TLM_WRITE_COMMAND;
				access_request->set_command(cmnd);
				access_request->set_address(info.address.to_uint64());
				access_request->set_data_ptr((unsigned char*)data);
				access_request->set_data_length(2);
				access_request->set_streaming_width(2);
				access_request->set_byte_enable_ptr(0);
				access_request->set_dmi_allowed(false);
				access_request->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
				main_port->get(ID);
				data[0] = ID.range(7, 0).to_uint();
				data[1] = ID.range(15, 8).to_uint();
				MCU_socket->b_transport(*access_request, blocked_time);
				if (access_request->get_response_status() != tlm::TLM_OK_RESPONSE)
				{
					SC_REPORT_ERROR("TLM2", "Error in memory handling of MCU request\n");
				}
			}
		}
	}
}
