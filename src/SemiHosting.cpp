#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "SemiHosting.h"
#include "Utilities.h"
#include "system.h"

void intercept_table::add_row(entry new_row)
{
	row_cntr++;
	row.push_back(new_row);
}

entry intercept_table::get_row_info(int line)
{
	return row[line];
}

bool semi_host_base::is_intercept_inst(entry temp, PUNEH_register IR, PUNEH_register PC)
{
	sc_lv<16> IR_temp = IR.get_value();

	int H_temp[4];
	H_temp[0] = IR_temp.range(15, 12).to_uint();
	H_temp[1] = IR_temp.range(11, 8).to_uint();
	H_temp[2] = IR_temp.range(7, 4).to_uint();
	H_temp[3] = IR_temp.range(3, 0).to_uint();

	bool ok = true;

	for (int i = 0; i < 4; i++)
	{
		if (temp.opcode_H[i] == -1)
			continue;
		else if (H_temp[i] != temp.opcode_H[i])
			return false;
	}

	if (temp.add != -1)
		if (temp.add != PC.get_value())
			ok = false;
	return ok;
}

void FIL_Intercept(PUNEH_register &AC, PUNEH_register &PC)
{
	cout << "SemiHost is working :D" << endl;
	PC.put_value(PC.get_value() + 1);
}

void XXX_Intercept(PUNEH_register &AC, PUNEH_register &PC)
{
	cout << "second Intercepted function :D" << endl;
	PC.put_value(PC.get_value() + 1);
}

void intercept_library::register_table()
{
	entry temp;
	temp.add = -1;
	temp.opcode_H[0] = 15;
	temp.opcode_H[1] = FLE;
	temp.opcode_H[2] = -1;
	temp.opcode_H[3] = -1;
	temp.fun_ptr = FIL_Intercept;
	table.add_row(temp);

	temp.add = -1;
	temp.opcode_H[0] = 15;
	temp.opcode_H[1] = XXX;
	temp.opcode_H[2] = -1;
	temp.opcode_H[3] = -1;
	temp.fun_ptr = XXX_Intercept;
	table.add_row(temp);
}
