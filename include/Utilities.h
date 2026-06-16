#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc.h>
#include <string>

#ifndef _MY_UTILITIES_H_
#define _MY_UTILITIES_H_

/************************************************************************/
/*							MAPPING INFO								*/
/************************************************************************/
#define max_val 65536

#define USART_Base		0XFFE0
#define USART_offset	4
#define RF_Base			0XFFF0
#define RF_offset		16


/************************************************************************/
/*								PROCESSOR								*/
/************************************************************************/
enum InstType {full_address, no_address};
enum MapType {immediate, addressed, indirect};

struct PUNEH_inst
{
	InstType type;
	MapType	 mapping;
	unsigned char opcode;
	unsigned char sub_opcode;
	char inst;
	int immediate;
	sc_lv<4> flag_en;
	sc_lv<4> flag_value;
};

struct Flags
{
	sc_lv<1> Z, N, C, V;
	void get_value(sc_lv<4> &fval);
	int get_value();
	void put_value(sc_lv<4> fval);
	void reset();
	void initial();
};

struct PUNEH_register
{
	int val;
	void reset();
	void put_value(sc_lv<16> data);
	void put_value(int data);
	int& put_value();
	void get_value(sc_lv<16> &data);
	void get_value(int &data);
	int get_value();
	int get_value(int a, int b); // (min , max) or (max, min)
	void initial();
	void value_check();
};

struct ALU_info
{
	int op1,op2;
};

#define mem(x) x

#define LDm 0
#define LDa 1 
#define LDn 2
#define STa 3
#define STn 4
#define INa 5
#define ANm 6
#define ANa 7
#define ADm 8
#define ADa 9 
#define ADn 10
#define MLa 11
#define JMa 12
#define JMn 13
#define JSR 14

#define LOm 0
#define ALU 1
#define SRA 2
#define SLL 3
#define SRL 4
#define SKP 5
#define SET 6
#define EIN 7
#define SIC 8
#define RIC 9
#define RIR 10
#define PAC 12	//print AC
#define FLE 13
#define XXX 14
#define EXT 15	// EXIT

// senihost : file op
#define OPN 0
#define CLS 1
#define RED 2
#define WRT 3


#define LPO 0
#define LOP 1
#define ACZ 2
#define ACN 3
#define ACI 4

/************************************************************************/
/************************************************************************/
/*								  channel								*/
/************************************************************************/
#define access_cnt 10

enum operation {read, write};
enum burst {one, two, four, eight};

struct transaction_info
{
	operation op;
	sc_lv<16> address;
	burst burst_cnt;
};
/************************************************************************/
/*								  assembler								*/
/************************************************************************/
enum update_CV_flag_type
{two_op, inc, shift};

int FA(int opcode, int imm);
int NA(int sub_opcode);
int NA(int sub_opcode, int imm);
int NA(int sub_opcode, int en, int val);
std::string int2hex(int val, int len);

struct printing_info
{
	int inst_cntr;
	sc_time time;
	sc_lv<16> PC_val;
	int IR_val;
	int AC_val;
	int OF_val;
	InstType type;
	std::string mnom;
	std::string add;
	int flag;
};

#define int_val 0
#define uint_val 1
#define char_val 2
#define hex_val 3
#define no_header 0
#define add_header 8
#define endl_befor 16
#define endl_after 64
/************************************************************************/





#endif
