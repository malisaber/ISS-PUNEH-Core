#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include <vector>
#include "Utilities.h"

#ifndef _MY_SEMIHOST_H_
#define _MY_SEMIHOST_H_

class entry 
{ 
public:
	int opcode_H[4];
	int add; 
	void(*fun_ptr)(PUNEH_register &AC, PUNEH_register &PC);
	entry(){ for (int i = 0; i < 4; i++){ opcode_H[i] = -1; } };
};

class intercept_table
{
public:
	std::vector<entry> row;
	int row_cntr;
	intercept_table(){ row_cntr = 0; };
	void add_row(entry new_row);
	entry get_row_info(int line);
};

class semi_host_base
{
public:
	intercept_table table;

	bool is_intercept_inst(entry temp, PUNEH_register IR, PUNEH_register PC);
};

class intercept_library : public semi_host_base
{
public:
	void register_table();
};

#endif // !_MY_SEMIHOST_H_
