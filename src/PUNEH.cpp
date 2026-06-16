#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "PUNEH.h"
#include "Utilities.h"

void PUNEH::req_handler()
{
	req = false;
	if (int_enable)
	{
		if (interrupt->read() == sc_logic_1)
		{
			req = true;
		}
	}
}

void PUNEH::initial()
{
	inst_cntr = 0;

	flag.initial();
	PC.initial();
	IR.initial();
	IN_.initial();
	AC.initial();
	OF.initial();

	register_table();
	int_enable = false;
	wait();
}

void PUNEH::reseting()
{
	PC.reset();
	IR.reset();
	IN_.reset();
	AC.reset();
	OF.reset();
	flag.reset();
	int_enable = false;
}

void PUNEH::main_port_reseting()
{
	if (rst->read() == sc_logic_1)
	{
		main_port->reset_started();
		cout << "reset asserted at " << sc_time_stamp() << endl;
	}
	else
	{
		main_port->reset_stopped();
		cout << "normal operation at " << sc_time_stamp() << endl;
	}
}

void PUNEH::operating()
{
	while (1)
	{
		if (rst->read() == sc_logic_1)
			reseting();
		else if (clk->read() == sc_logic_1)
		{
			if (!req)
			{
				fetch();
				wait();

				if (rst->read() == sc_logic_1)
					continue;

				instruction_decode();
				execute();
			}
			else
				Interrupt_routine();
		}
		wait();
	}
}

void PUNEH::fetch()
{
	do_read(IR, mem(PC));
	
	pinf.inst_cntr = inst_cntr;
	pinf.time = sc_time_stamp();
	pinf.PC_val = PC.get_value();
	pinf.IR_val = IR.get_value();
}

void PUNEH::instruction_decode()
{
	sc_lv<16> IR_val;
	IR_val = IR.get_value();

	inst_info.opcode = IR_val.range(15, 12).to_uint();
	inst_info.sub_opcode = IR_val.range(11, 8).to_uint();
	inst_info.inst = IR_val.range(7, 0).to_uint();
	inst_info.immediate = IR_val.range(11, 0).to_int();
	inst_info.flag_en = IR_val.range(7, 4).to_uint();
	inst_info.flag_value = IR_val.range(3, 0).to_uint();

	if (inst_info.opcode == 15)
		inst_info.type = no_address;
	else if (inst_info.opcode < 15)
	{
		inst_info.type = full_address;
		inst_info.mapping = addressed;
		switch (inst_info.opcode)
		{
		case LDn: inst_info.mapping = indirect; break;
		case STn: inst_info.mapping = indirect; break;
		case ADn: inst_info.mapping = indirect; break;
		case JMn: inst_info.mapping = indirect; break;
		}
	}
}

void PUNEH::indirect_operand()
{
	sc_lv<16> address;
	address.range(15,12) = OF.get_value(15,12);
	address.range(0, 11) = IR.get_value(0, 11);
	do_read(IN_, mem(address.to_int()));
}

void PUNEH::do_read(PUNEH_register &dest, int address)
{
	main_port->trylock(PUNEH_priority, read, address, one);
	main_port->get(dest.put_value());
	dest.value_check();
	main_port->unlock();
}

void PUNEH::do_read(PUNEH_register &dest, PUNEH_register address)
{
	do_read(dest, address.get_value());
}

int PUNEH::do_read(int address)
{
	PUNEH_register dummy;
	do_read(dummy, mem(address));
	return dummy.get_value();
}

void PUNEH::do_write(int address, PUNEH_register source)
{
	main_port->trylock(PUNEH_priority, write, address, one);
	main_port->put(source.get_value());
	main_port->unlock();
}

void PUNEH::do_write(PUNEH_register address, PUNEH_register source)
{
	do_write(address.get_value(), source);
}

bool PUNEH::execute()
{
	if (SemiHosting_Enable)
	{
		return intercept_instruction_execution();
	}
	else
	{
		bool ok;
		ok = normal_instruction_execution();
		if (printing_enable)
		{
			printing();
		}
		return ok;
	}
}

bool PUNEH::intercept_instruction_execution()
{
	int row_cnt = table.row_cntr;
	entry temp;

	for (int i = 0; i < row_cnt; i++)
	{
		temp = table.get_row_info(i);

		if (is_intercept_inst(temp, IR, PC))
		{
			temp.fun_ptr(AC, PC);
			return true;
		}
	}
	bool ok;
	ok = normal_instruction_execution();
	if (printing_enable)
		printing();
	return ok;
}

bool PUNEH::normal_instruction_execution()
{
	sc_lv<16> address;
	int imm;
	sc_lv<4> old, out;

	if (inst_info.type == full_address)
	{
		pinf.add = int2hex(inst_info.immediate, 3);

		if (inst_info.mapping == indirect)
		{
			if (inst_info.opcode != JMn)
			{
				indirect_operand();
				wait();
				if (rst->read() == sc_logic_1)
					return false;
			}
		}

		imm = inst_info.immediate;
		address.range(15, 12) = OF.get_value(15, 12);
		address.range(11, 0) = imm;
		

		switch (inst_info.opcode)
		{
		case LDm:
			pinf.mnom = "LDm";
			AC.put_value(imm);
			update_ZN_flag();
			PC.put_value(PC.get_value() + 1);
			break;
		case LDa:
			pinf.mnom = "LDa";
			do_read(AC, mem(address.to_int()));
			update_ZN_flag();
			PC.put_value(PC.get_value() + 1);
			break;
		case LDn:
			pinf.mnom = "LDn";
			do_read(AC, mem(IN_));
			update_ZN_flag();
			PC.put_value(PC.get_value() + 1);
			break;
		case STa:
			pinf.mnom = "STa";
			do_write(mem(address.to_int()), AC);
			PC.put_value(PC.get_value() + 1);
			break;
		case STn:
			pinf.mnom = "STn";
			do_write(mem(IN_), AC);
			PC.put_value(PC.get_value() + 1);
			break;
		case INa:
			pinf.mnom = "INa";
			do_read(AC, mem(address.to_int()));
			AC.put_value(AC.get_value() + 1);
			alu_info.op1 = AC.get_value();
			update_ZN_flag();
			update_CV_flag(inc);
			wait();
			do_write(mem(address.to_int()), AC);
			PC.put_value(PC.get_value() + 1);
			break;
		case ANm :
			pinf.mnom = "ANm";
			AC.put_value(AC.get_value() & imm);
			update_ZN_flag();
			PC.put_value(PC.get_value() + 1);
			break;
		case ANa:
			pinf.mnom = "ANa";
			imm = do_read(mem(address.to_int()));
			AC.put_value(AC.get_value() & imm);
			update_ZN_flag();
			PC.put_value(PC.get_value() + 1);
			break;
		case ADm:
			pinf.mnom = "ADm";
			alu_info.op1 = AC.get_value();
			alu_info.op2 = imm;
			AC.put_value(AC.get_value() + imm);
			update_ZN_flag();
			update_CV_flag(two_op);
			PC.put_value(PC.get_value() + 1);
			break;
		case ADa:
			pinf.mnom = "ADa";
			imm = do_read(mem(address.to_int()));
			alu_info.op1 = AC.get_value();
			alu_info.op2 = imm;
			AC.put_value(AC.get_value() + imm);
			update_ZN_flag();
			update_CV_flag(two_op);
			PC.put_value(PC.get_value() + 1);
			break;
		case ADn:
			pinf.mnom = "ADn";
			imm = do_read(mem(IN_.get_value()));
			alu_info.op1 = AC.get_value();
			alu_info.op2 = imm;
			AC.put_value(AC.get_value() + imm);
			update_ZN_flag();
			update_CV_flag(two_op);
			PC.put_value(PC.get_value() + 1);
			break;
		case MLa:
			pinf.mnom = "MLa";
			imm = do_read(mem(address.to_int()));
			AC.put_value(AC.get_value(7,0) * (imm%256));
			update_ZN_flag();
			PC.put_value(PC.get_value() + 1);
			break;
		case JMa:
			pinf.mnom = "JMa";
			PC.put_value(address);
			break;
		case JMn:
			pinf.mnom = "JMn";
			do_read(PC, mem(address.to_int()));
			break;
		case JSR:
			pinf.mnom = "JSR";
			PC.put_value(PC.get_value() + 1);
			do_write(mem(address.to_int()), PC);
			PC.put_value(address.to_int() + 1);
			break;
		default:
			PC.put_value(PC.get_value() + 1);
			pinf.mnom = "NON";
			cout << "invalid operation for full-address operations" << endl;
			break;
		}
	}
	else
	{
		switch (inst_info.sub_opcode)
		{
		case LOm: 
			pinf.mnom = "LOm";
			imm = IR.get_value(3,0);
			OF.put_value(imm * 4096);
			pinf.add = inst_info.inst;
			PC.put_value(PC.get_value() + 1);
			break;
		case ALU:
			ALU_op();
			pinf.add = "   ";
			PC.put_value(PC.get_value() + 1);
			break;
		case SRA:
			pinf.mnom = "SRA";
			imm = IR.get_value(7, 0);
			AC.put_value(AC.get_value() >> imm);
			update_ZN_flag();
			update_CV_flag(shift);
			PC.put_value(PC.get_value() + 1);
			break;
		case SLL:
			pinf.mnom = "SLL";
			imm = IR.get_value(7, 0);
			AC.put_value(AC.get_value() << imm);
			update_ZN_flag();
			update_CV_flag(shift);
			PC.put_value(PC.get_value() + 1);
			break;
		case SRL:
			pinf.mnom = "SRL";
			imm = IR.get_value(7, 0);
			AC.put_value(((unsigned)AC.get_value()) >> 2);
			update_ZN_flag();
			update_CV_flag(shift);
			PC.put_value(PC.get_value() + 1);
			break;
		case SKP:
			pinf.mnom = "SKP";
			if (check_flags())
				PC.put_value(PC.get_value() + 2);
			else
				PC.put_value(PC.get_value() + 1);
			pinf.add = int2hex(inst_info.flag_en.to_uint(), 1) + "," + int2hex(inst_info.flag_value.to_uint(), 1);
			break;
		case SET:
			pinf.mnom = "SET";
			flag.get_value(old);
			out = (inst_info.flag_en & inst_info.flag_value) | ((~inst_info.flag_en) & old);
			flag.put_value(out);
			pinf.add = int2hex(inst_info.flag_en.to_uint(), 1) + "," + int2hex(inst_info.flag_value.to_uint(), 1);
			PC.put_value(PC.get_value() + 1);
			break;
		case EIN:
			pinf.mnom = "EIN";
			pinf.add = "   ";
			int_enable = true;
			PC.put_value(PC.get_value() + 1);
			break;
		case SIC:
			pinf.mnom = "SIC";
			pinf.add = "   ";
			PUNEH_register temp_SIC;
			temp_SIC.put_value(flag.get_value());
			do_write(RF_Base + 12, AC);
			wait();
			do_write(RF_Base + 13, OF);
			wait();
			do_write(RF_Base + 14, IN_);
			wait();
			do_write(RF_Base + 15, temp_SIC);
			PC.put_value(PC.get_value() + 1);
			break;
		case RIC:
			pinf.mnom = "RIC";
			pinf.add = "   ";
			PUNEH_register temp_RIC;
			do_read(AC, RF_Base + 12);
			wait();
			do_read(OF, RF_Base + 13);
			wait();
			do_read(IN_, RF_Base + 14);
			wait();
			do_read(temp_RIC, RF_Base + 15);
			flag.put_value(temp_RIC.get_value());
			PC.put_value(PC.get_value() + 1);
			break;
		case RIR:
			pinf.mnom = "RIR";
			pinf.add = "   ";
			do_read(PC, 0XFFF0);
			int_enable = true;
			break;
		case PAC:
			pinf.mnom = "PAC";
			pinf.add = "   ";
			PAC_print();
			PC.put_value(PC.get_value() + 1);
			break;
		case EXT: 
			pinf.mnom = "EXT";
			pinf.add = "   ";
			cout << endl << "\t\t\tEND OF EXECUTION @ " << sc_time_stamp() << endl << endl;
			PC.put_value(PC.get_value() + 1);
			while (true)
			{
				wait();
				if (rst->read() == sc_logic_1)
				{
					break;
				}
			}
			break;
		default:
			PC.put_value(PC.get_value() + 1);
			pinf.mnom = "NON";
			cout << "invalid operation for no-address operations" << endl;
			break;
		}
	}
	return true;
}

bool PUNEH::check_flags()
{
	bool ok(true);
	sc_lv<4> cfval;

	flag.get_value(cfval);

	for (int i = 0; i < 4; i++)
	{
		if (inst_info.flag_en[i] == '0')
			continue;
		if (inst_info.flag_value[i] != cfval[i])
			ok = false;
	}
	return ok;
}

void PUNEH::update_ZN_flag()
{
	if (AC.get_value() == 0)
		flag.Z = '1';
	else
		flag.Z = '0';

	if (AC.get_value() < 0)
		flag.N = '1';
	else
		flag.N = '0';
}

void PUNEH::update_CV_flag(update_CV_flag_type type)
{
	if (type == two_op)
	{
		if (((alu_info.op1 < 0) && (alu_info.op2 < 0) && (AC.get_value() >= 0)) || ((alu_info.op1 >= 0) && (alu_info.op2 >= 0) && (AC.get_value() < 0)))
			flag.V = '1';
		else
			flag.V = '0';

		if (abs(AC.get_value()) < abs(alu_info.op1 + alu_info.op2))
			flag.C = '1';
		else
			flag.C = '0';
	}
	else if (type == inc)
	{
		if (alu_info.op1 > 0 && AC.get_value() < 0)
		{
			flag.C = '0';
			flag.V = '1';
		}
		else if ((alu_info.op1 < 0 && AC.get_value() >= 0))
		{
			flag.C = '1';
			flag.V = '1';
		}
		else
		{
			flag.C = '0';
			flag.V = '0';
		}
	}
	else if (type == shift)
	{
		if ((alu_info.op1 >= 0 && AC.get_value() < 0) || (alu_info.op1 < 0 && AC.get_value() >= 0))
			flag.V = '1';
		else
			flag.V = '0';

		if (alu_info.op1 < 0)
			flag.C = '1';
		else
			flag.C = '0';
	}
	else
		cout << "error while doing flags" << endl;
}

void PUNEH::ALU_op()
{
	alu_info.op1 = AC.get_value();
	alu_info.op2 = 0;

	switch (inst_info.inst)
	{
	case LPO:
		pinf.mnom = "LPO";
		OF.put_value(PC.get_value());
		break;
	case LOP:
		pinf.mnom = "LOP";
		PC.put_value(OF.get_value());
		break;
	case ACZ:
		pinf.mnom = "ACZ";
		AC.put_value(0);
		update_ZN_flag();
		break;
	case ACN:
		pinf.mnom = "ACN";
		AC.put_value(-1 - AC.get_value());
		update_ZN_flag();
		break;
	case ACI:
		pinf.mnom = "ACI";
		AC.put_value(AC.get_value() + 1);
		update_ZN_flag();
		update_CV_flag(inc);
		break;
	default:
		pinf.mnom = "NON";
		break;
	}
}

void PUNEH::go_generator()
{
	if (rst->read() == sc_logic_1)
	{
		go.notify();
	}
	else if ((clk->read() == sc_logic_1) && clk->event())
	{
		go.notify();
	}
}

void PUNEH::printing()
{
	sc_lv<4> bin_f;
	

	pinf.AC_val = AC.get_value();
	pinf.OF_val = OF.get_value();
	pinf.flag = flag.get_value();

	pinf.type = inst_info.type;

	bin_f = pinf.flag;

	cout << "\tInst." << pinf.inst_cntr << "\t@ " << sc_time_stamp() <<
		"\t=>\tPC:" << int2hex(pinf.PC_val.to_uint(),4) << ",\tIR:" << int2hex(pinf.IR_val, 4) <<
		"\t=>\t" << pinf.mnom << " " << pinf.add <<
		";\tAC: " << pinf.AC_val << ",\tOF: " << pinf.OF_val <<
		",\tZNCV: " << bin_f << 
		"\tinterrupt: " << interrupt->read() <<
		"\tint_en: " << int_enable << 
		"\treq: " << req;
	cout << endl;
	
	inst_cntr++;
}

void PUNEH::PAC_print()
{
	sc_lv<16> IR_temp;
	IR_temp = IR.get_value();
	unsigned int type, header, befor, after;
	type = IR_temp.range(2, 0).to_uint();
	header = IR_temp.range(3, 3).to_uint();
	befor = IR_temp.range(5, 4).to_uint();
	after = IR_temp.range(7, 6).to_uint();

	for (unsigned int i = 0; i < befor; i++)
		cout << endl;

	if (header != 0)
		cout << "\t\t\tAC Value: ";

	switch (type)
	{
	case int_val:
		cout << AC.get_value();
		break;
	case uint_val:
		cout << (unsigned int)AC.get_value();
		break;
	case char_val:
		cout << (unsigned char)AC.get_value(7, 0);
		break;
	case hex_val:
		cout << int2hex(AC.get_value(), 4);
		break;
	default:
		cout << AC.get_value();
		break;
	}
	

	for (unsigned int i = 0; i < after; i++)
		cout << endl;
}

void PUNEH::Interrupt_routine()
{
	int_enable = false;
	req = false;
	do_write(RF_Base, PC);
	PC.put_value(0XFFF1);
}
