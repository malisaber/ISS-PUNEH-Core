#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "memory.h"
#include "Utilities.h"

void memory::b_transport(tlm::tlm_generic_payload& incoming, sc_time& t)
{
	tlm::tlm_command cmd = incoming.get_command();
	uint64 address = incoming.get_address();
	unsigned char* ptr = incoming.get_data_ptr();
	unsigned int length = incoming.get_data_length();
	unsigned char* byt = incoming.get_byte_enable_ptr();
	unsigned int wid = incoming.get_streaming_width();

	if (address >= uint64(4096) || byt != 0 || length > 2 || wid < length)
	{
		SC_REPORT_ERROR("TLM-2.0: ", "\t\tInconsistant Generic Payload");
	}

	if (cmd == tlm::TLM_READ_COMMAND)
	{
		*(ptr + 0) = (unsigned char)mem[address].range(7, 0).to_uint();
		*(ptr + 1) = (unsigned char)mem[address].range(15,8).to_uint();
	}
	else if (cmd == tlm::TLM_WRITE_COMMAND)
	{
		mem[address].range(7, 0) = (sc_lv<8>) *(ptr + 0);
		mem[address].range(15,8) = (sc_lv<8>) *(ptr + 1);
	}

	incoming.set_response_status(tlm::TLM_OK_RESPONSE);

	if (t != sc_time(0, SC_NS))
	{
		wait(t);
	}
}



#define n 15
#define a0 45
#define a1 46
#define t 47
#define cntr 48
#define pointer 49

void memory::initiate()
{
	mem[0] = FA(LDm, -n + 1);	// number of terms : n
	mem[1] = FA(STa, cntr);
	mem[2] = FA(LDm, 100);		// results will store in mem[64] to mem[64+abs(n)]
	mem[3] = FA(STa, pointer);
	mem[4] = FA(LDm, 0);
	mem[5] = FA(STa, a0);
	mem[6] = FA(STn, pointer);
	mem[7] = FA(INa, pointer);
	mem[8] = FA(LDm, 1);
	mem[9] = NA(PAC, 0X49);
	mem[10] = FA(STa, a1);
	mem[11] = FA(STn, pointer);
	mem[12] = FA(INa, pointer);
	// loop:
	mem[13] = FA(LDa, a0);
	mem[14] = FA(ADa, a1);
	mem[15] = FA(STa, t);
	mem[16] = FA(STn, pointer);
	mem[17] = NA(PAC, 0X49);
	mem[18] = FA(LDa, a1);
	mem[19] = FA(STa, a0);
	mem[20] = FA(LDa, t);
	mem[21] = FA(STa, a1);
	mem[22] = FA(INa, pointer);
	mem[23] = FA(INa, cntr);
	mem[24] = NA(SKP, 0X8, 0X8); // sKP if z == 1
	mem[25] = FA(JMa, 13);
	mem[26] = NA(EXT);
}
