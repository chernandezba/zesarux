// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 *   scmp.c
 *
 *   National Semiconductor SC/MP CPU Emulator
 *
 *****************************************************************************/


#include <stdio.h>

#include "scmp.h"
#include "cpu.h"
#include "operaciones.h"

unsigned char *memory_data;

//TODO
int scmp_m_sensea_func(void)
{
	return 0;
}

//const device_type SCMP = &device_creator<scmp_device>;
//const device_type INS8060 = &device_creator<ins8060_device>;

int scmp_m_icount;








SCMP_UINT_8 scmp_device_read_decrypted_byte(SCMP_UINT_16 a)
{
	return peek_byte_no_time(a);
}
SCMP_UINT_8 scmp_device_read_raw_byte(SCMP_UINT_16 a)
{
	return peek_byte_no_time(a);
}

SCMP_UINT_8 scmp_device_read_byte(SCMP_UINT_32 a)
{
	return peek_byte_no_time(a);
}

void scmp_device_write_byte(SCMP_UINT_32 a, SCMP_UINT_8 v)
{
	//printf ("--Write %04XH valor %02XH\n",a,v);
	poke_byte_no_time(a,v);
}


union SCMP_PAIR    scmp_m_PC;
union SCMP_PAIR    scmp_m_P1;
union SCMP_PAIR    scmp_m_P2;
union SCMP_PAIR    scmp_m_P3;
SCMP_UINT_8   scmp_m_AC;
SCMP_UINT_8   scmp_m_ER;
SCMP_UINT_8   scmp_m_SR;


/*
scmp_device_scmp_device(const machine_config &mconfig, const char *tag, device_t *owner, SCMP_UINT_32 clock)
	: cpu_device(mconfig, SCMP, "INS 8050 SC/MP", tag, owner, clock, "ins8050", __FILE__)
	, scmp_m_prograscmp_m_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
	, scmp_m_flag_out_func(*this)
	, scmp_m_sout_func(*this)
	, scmp_m_sin_func(*this)
	, scmp_m_sensea_func(*this)
	, scmp_m_senseb_func(*this)
	, scmp_m_halt_func(*this)
{
}


scmp_device_scmp_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, SCMP_UINT_32 clock, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, scmp_m_prograscmp_m_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
	, scmp_m_flag_out_func(*this)
	, scmp_m_sout_func(*this)
	, scmp_m_sin_func(*this)
	, scmp_m_sensea_func(*this)
	, scmp_m_senseb_func(*this)
	, scmp_m_halt_func(*this)
{
}


ins8060_device_ins8060_device(const machine_config &mconfig, const char *tag, device_t *owner, SCMP_UINT_32 clock)
	: scmp_device(mconfig, INS8060, "INS 8060 SC/MP II", tag, owner, clock, "ins8060", __FILE__)
{
}

offs_t scmp_device_disasscmp_m_disassemble(char *buffer, offs_t pc, const SCMP_UINT_8 *oprom, const SCMP_UINT_8 *opram, SCMP_UINT_32 options)
{
	extern CPU_DISASSEMBLE( scmp );
	return CPU_DISASSEMBLE_NAME(scmp)(this, buffer, pc, oprom, opram, options);
}
*/

SCMP_UINT_16 scmp_device_ADD12(SCMP_UINT_16 addr, SCMP_INT_8 val)
{
	return ((addr + val) & 0x0fff) | (addr & 0xf000);
}

SCMP_UINT_8 scmp_device_ROP()
{
	SCMP_UINT_16 pc = scmp_m_PC.w.l;
	scmp_m_PC.w.l = scmp_device_ADD12(scmp_m_PC.w.l,1);   //PC=PC+1
	//return scmp_m_direct->read_decrypted_byte( pc);
	return scmp_device_read_decrypted_byte( pc);
}

SCMP_UINT_8 scmp_device_ARG()
{
	SCMP_UINT_16 pc = scmp_m_PC.w.l;
	scmp_m_PC.w.l = scmp_device_ADD12(scmp_m_PC.w.l,1);  //PC=PC+1
	return scmp_device_read_raw_byte(pc);
}

SCMP_UINT_8 scmp_device_RM(SCMP_UINT_32 a)
{
	//return scmp_m_program->read_byte(a);
	return scmp_device_read_byte(a);
}

void scmp_device_WM(SCMP_UINT_32 a, SCMP_UINT_8 v)
{
	//scmp_m_program->write_byte(a, v);
	scmp_device_write_byte(a, v);
}

void scmp_device_illegal(SCMP_UINT_8 opcode GCC_UNUSED)
{
//	SCMP_UINT_16 pc = scmp_m_PC.w.l;
//	LOG(("SC/MP illegal instruction %04X $%02X\n", pc-1, opcode));
}

union SCMP_PAIR *scmp_device_GET_PTR_REG(int num)
{
	switch(num) {
		case 1: return &scmp_m_P1;
		case 2: return &scmp_m_P2;
		case 3: return &scmp_m_P3;
		default :
				return &scmp_m_PC;
	}
}

void scmp_device_BIN_ADD(SCMP_UINT_8 val)
{
	SCMP_UINT_16 tmp = scmp_m_AC + val + ((scmp_m_SR >> 7) & 1);
	SCMP_UINT_8 ov = (((scmp_m_AC & 0x80)==(val & 0x80)) && ((scmp_m_AC & 0x80)!=(tmp & 0x80))) ? 0x40 : 0x00;

	scmp_m_AC = tmp & 0xff;
	scmp_m_SR &= 0x3f; // clear CY/L and OV flag
	scmp_m_SR |= (tmp & 0x100) ? 0x80 : 0x00; // set CY/L
	scmp_m_SR |= ov;
}

void scmp_device_DEC_ADD(SCMP_UINT_8 val)
{
	SCMP_UINT_16 tmp = scmp_m_AC + val + ((scmp_m_SR >> 7) & 1);
	if ((tmp & 0x0f) > 9) tmp +=6;
	scmp_m_AC = tmp % 0xa0;
	scmp_m_SR &= 0x7f; // clear CY/L flag
	scmp_m_SR |= (tmp > 0x99) ? 0x80 : 0x00;
}

SCMP_UINT_16 scmp_device_GET_ADDR(SCMP_UINT_8 code)
{
	SCMP_UINT_16 addr = 0;
	SCMP_INT_8 offset = 0;
	SCMP_UINT_16 retVal = 0;
	SCMP_UINT_16 ptr = scmp_device_GET_PTR_REG(code & 0x03)->w.l;

	SCMP_UINT_8 arg = scmp_device_ARG();
	if (arg == 0x80) {
		offset = scmp_m_ER;
	} else {
		if (arg & 0x80) {
			offset = (SCMP_INT_8)arg;
		} else {
			offset = arg;
		}
	}

	addr = scmp_device_ADD12(ptr,offset);

	if (code & 0x04) {
		if (code & 0x03) {
			// Auto-indexed
			if (offset < 0) {
				// pre decrement
				scmp_device_GET_PTR_REG(code & 0x03)->w.l = addr;
				retVal = addr;
			} else {
				// post increment
				retVal = ptr;
				scmp_device_GET_PTR_REG(code & 0x03)->w.l = addr;
			}
		} else {
			// Immediate
		}
	} else {
		// Indexed
		retVal = addr;
	}
	return retVal;
}

void scmp_device_execute_one(int opcode)
{
	SCMP_UINT_8 tmp;
	SCMP_UINT_8 ptr = opcode & 3;
	if (opcode&128) {
		// two bytes instructions
		switch (opcode)
		{
			// Memory Reference Instructions
			case 0xc0 : case 0xc1 : case 0xc2 : case 0xc3 :
			case 0xc5 : case 0xc6 : case 0xc7 :
						//LD
						t_estados+= 18;
						scmp_m_AC = scmp_device_RM(scmp_device_GET_ADDR(opcode));
						break;
			case 0xc8 : case 0xc9 : case 0xca : case 0xcb :
			case 0xcd : case 0xce : case 0xcf :
						// ST
						t_estados+= 18;
						scmp_device_WM(scmp_device_GET_ADDR(opcode),scmp_m_AC);
						break;
			case 0xd0 : case 0xd1 : case 0xd2 : case 0xd3 :
						case 0xd5 : case 0xd6 : case 0xd7 :
						// AND
						t_estados+= 18;
						scmp_m_AC &= scmp_device_RM(scmp_device_GET_ADDR(opcode));
						break;
			case 0xd8 : case 0xd9 : case 0xda : case 0xdb :
						case 0xdd : case 0xde : case 0xdf :
						//OR
						t_estados+= 18;
						scmp_m_AC |= scmp_device_RM(scmp_device_GET_ADDR(opcode));
						break;
			case 0xe0 : case 0xe1 : case 0xe2 : case 0xe3 :
						case 0xe5 : case 0xe6 : case 0xe7 :
						// XOR
						t_estados+= 18;
						scmp_m_AC ^= scmp_device_RM(scmp_device_GET_ADDR(opcode));
						break;
			case 0xe8 : case 0xe9 : case 0xea : case 0xeb :
						case 0xed : case 0xee : case 0xef :
						// DAD
						t_estados+= 23;
						scmp_device_DEC_ADD(scmp_device_RM(scmp_device_GET_ADDR(opcode)));
						break;
			case 0xf0 : case 0xf1 : case 0xf2 : case 0xf3 :
						case 0xf5 : case 0xf6 : case 0xf7 :
						// ADD
						t_estados+= 19;
						scmp_device_BIN_ADD(scmp_device_RM(scmp_device_GET_ADDR(opcode)));
						break;
			case 0xf8 : case 0xf9 : case 0xfa : case 0xfb :
						case 0xfd : case 0xfe : case 0xff :
						// CAD
						t_estados+= 20;
						scmp_device_BIN_ADD(~scmp_device_RM(scmp_device_GET_ADDR(opcode)));
						break;
			// Memory Increment/Decrement Instructions
			case 0xa8 : case 0xa9 : case 0xaa : case 0xab :
						// IDL
						{
							SCMP_UINT_16 addr = scmp_device_GET_ADDR(opcode);
							t_estados+= 22;
							scmp_m_AC = scmp_device_RM(addr) + 1;
							scmp_device_WM(addr,scmp_m_AC);
						}
						break;
			case 0xb8 : case 0xb9 : case 0xba : case 0xbb :
						// DLD
						{
							SCMP_UINT_16 addr = scmp_device_GET_ADDR(opcode);
							t_estados+= 22;
							scmp_m_AC = scmp_device_RM(addr) - 1;
							scmp_device_WM(addr,scmp_m_AC);
						}
						break;
			// Immediate Instructions
			case 0xc4 : // LDI
						t_estados+= 10;
						scmp_m_AC = scmp_device_ARG();
						break;
			case 0xd4 : // ANI
						t_estados+= 10;
						scmp_m_AC &= scmp_device_ARG();
						break;
			case 0xdc : // ORI
						t_estados+= 10;
						scmp_m_AC |= scmp_device_ARG();
						break;
			case 0xe4 : // XRI
						t_estados+= 10;
						scmp_m_AC ^= scmp_device_ARG();
						break;
			case 0xec : // DAI
						t_estados+= 15;
						scmp_device_DEC_ADD(scmp_device_ARG());
						break;
			case 0xf4 : // ADI
						t_estados+= 11;
						scmp_device_BIN_ADD(scmp_device_ARG());
						break;
			case 0xfc : // CAI
						t_estados+= 12;
						scmp_device_BIN_ADD(~scmp_device_ARG());
						break;
			// Transfer Instructions
			case 0x90 : case 0x91 : case 0x92 : case 0x93 :// JMP
						t_estados+= 11;
						tmp = scmp_device_ARG();
						scmp_m_PC.w.l = scmp_device_ADD12(scmp_device_GET_PTR_REG(ptr)->w.l,(SCMP_INT_8)tmp);

						//temp. Parece que falta esto! Sino, en el primer jmp de la rom, se queda registro PC por detras
						//Luego se comprueba que siguientes JMP saltan a donde deben
						//scmp_m_PC.w.l = scmp_device_ADD12(scmp_m_PC.w.l,1);   //PC=PC+1


						break;
			case 0x94 : case 0x95 : case 0x96 : case 0x97 :
						// JP
						t_estados+= 9;
						tmp = scmp_device_ARG();
						if (!(scmp_m_AC & 0x80)) {
							scmp_m_PC.w.l = scmp_device_ADD12(scmp_device_GET_PTR_REG(ptr)->w.l,(SCMP_INT_8)tmp);
							t_estados+= 2;
						}
						break;
			case 0x98 : case 0x99 : case 0x9a : case 0x9b :
						// JZ
						t_estados+= 9;
						tmp = scmp_device_ARG();
						if (!scmp_m_AC) {
							scmp_m_PC.w.l = scmp_device_ADD12(scmp_device_GET_PTR_REG(ptr)->w.l,(SCMP_INT_8)tmp);
							t_estados+= 2;
						}
						break;
			case 0x9c : case 0x9d : case 0x9e : case 0x9f :
						// JNZ
						t_estados+= 9;
						tmp = scmp_device_ARG();
						if (scmp_m_AC) {
							scmp_m_PC.w.l = scmp_device_ADD12(scmp_device_GET_PTR_REG(ptr)->w.l,(SCMP_INT_8)tmp);
							t_estados+= 2;
						}
						break;
			// Double-Byte Miscellaneous Instructions
			case 0x8f:  // DLY
						tmp = scmp_device_ARG();
						t_estados+= 13 + (scmp_m_AC * 2) + (((SCMP_UINT_32)tmp) << 1) + (((SCMP_UINT_32)tmp) << 9);
						scmp_m_AC = 0xff;
						break;
			// Others are illegal
			default :   t_estados+= 1;
						scmp_device_illegal (opcode);
						break;
		}

	} else {
		// one byte instructions
		switch (opcode)
		{
			// Extension Register Instructions
			case 0x40:  // LDE
						t_estados+= 6;
						scmp_m_AC = scmp_m_ER;
						break;
			case 0x01:  // XAE
						t_estados+= 7;
						tmp = scmp_m_AC;
						scmp_m_AC = scmp_m_ER;
						scmp_m_ER = tmp;
						break;
			case 0x50:  // ANE
						t_estados+= 6;
						scmp_m_AC &= scmp_m_ER;
						break;
			case 0x58:  // ORE
						t_estados+= 6;
						scmp_m_AC |= scmp_m_ER;
						break;
			case 0x60:  // XRE
						t_estados+= 6;
						scmp_m_AC ^= scmp_m_ER;
						break;
			case 0x68:  // DAE
						t_estados+= 11;
						scmp_device_DEC_ADD(scmp_m_ER);
						break;
			case 0x70:  // ADE
						t_estados+= 7;
						scmp_device_BIN_ADD(scmp_m_ER);
						break;
			case 0x78:  // CAE
						t_estados+= 8;
						scmp_device_BIN_ADD(~scmp_m_ER);
						break;
			// Pointer Register Move Instructions
			case 0x30: case 0x31: case 0x32: case 0x33: // XPAL
						t_estados+= 8;
						tmp = scmp_m_AC;
						scmp_m_AC = scmp_device_GET_PTR_REG(ptr)->b.l;
						scmp_device_GET_PTR_REG(ptr)->b.l = tmp;
						break;
			case 0x34:  case 0x35 :case 0x36: case 0x37:
						// XPAH
						t_estados+= 8;
						tmp = scmp_m_AC;
						scmp_m_AC = scmp_device_GET_PTR_REG(ptr)->b.h;
						scmp_device_GET_PTR_REG(ptr)->b.h = tmp;
						break;
			case 0x3c:  case 0x3d :case 0x3e: case 0x3f:
						// XPPC
						{
							SCMP_UINT_16 tmp16 = scmp_device_ADD12(scmp_m_PC.w.l,-1); // Since PC is incremented we need to fix it
							t_estados+= 7;
							scmp_m_PC.w.l = scmp_device_GET_PTR_REG(ptr)->w.l;
							scmp_device_GET_PTR_REG(ptr)->w.l = tmp16;
							// After exchange CPU increment PC
							scmp_m_PC.w.l = scmp_device_ADD12(scmp_m_PC.w.l,1);
						}
						break;
			// Shift, Rotate, Serial I/O Instructions
			case 0x19:  // SIO
						t_estados+= 5;
						//TODO scmp_m_sout_func(scmp_m_ER & 0x01);
						scmp_m_ER >>= 1;
						//TODO scmp_m_ER |= scmp_m_sin_func() ? 0x80 : 0x00;
						break;
			case 0x1c:  // SR
						t_estados+= 5;
						scmp_m_AC >>= 1;
						break;
			case 0x1d:  // SRL
						t_estados+= 5;
						scmp_m_AC >>= 1;
						scmp_m_AC |= scmp_m_SR & 0x80; // add C/L flag
						break;
			case 0x1e:  // RR
						t_estados+= 5;
						scmp_m_AC =  (scmp_m_AC >> 1) | ((scmp_m_AC & 0x01) << 7);
						break;
			case 0x1f:  // RRL
						t_estados+= 5;
						tmp = (scmp_m_AC & 0x01) << 7;
						scmp_m_AC =  (scmp_m_AC >> 1) | (scmp_m_SR & 0x80);
						scmp_m_SR = (scmp_m_SR & 0x7f) | tmp;
						break;
			// Single Byte Miscellaneous Instructions
			case 0x00:  // HALT
						t_estados+= 8;
						//TODO scmp_m_halt_func(1);
						//TODO scmp_m_halt_func(0);
						break;
			case 0x02:  // CCL
						t_estados+= 5;
						scmp_m_SR &= 0x7f;
						break;
			case 0x03:  // SCL
						t_estados+= 5;
						scmp_m_SR |= 0x80;
						break;
			case 0x04:  // DINT
						t_estados+= 6;
						scmp_m_SR &= 0xf7;
						break;
			case 0x05:  // IEN
						t_estados+= 6;
						scmp_m_SR |= 0x08;
						break;
			case 0x06:  // CSA
						t_estados+= 5;
						scmp_m_SR &= 0xcf; // clear SA and SB flags
						//TODO scmp_m_SR |= scmp_m_sensea_func() ? 0x10 : 0x00;
						//TODO scmp_m_SR |= scmp_m_senseb_func() ? 0x20 : 0x00;
						scmp_m_AC = scmp_m_SR;
						break;
			case 0x07:  // CAS
						t_estados+= 6;
						scmp_m_SR = scmp_m_AC;
						//TODO scmp_m_flag_out_func(scmp_m_SR & 0x07);
						break;
			case 0x08:  // NOP
						t_estados+= 5;
						break;
			// Others are illegal
			default :   t_estados+= 1;
						scmp_device_illegal (opcode);
						break;
		}
	}
}



/***************************************************************************
    COMMON EXECUTION
***************************************************************************/
void scmp_device_take_interrupt()
{
	SCMP_UINT_16 tmp = scmp_device_ADD12(scmp_m_PC.w.l,-1); // We fix PC so at return it goes to current location
	scmp_m_SR &= 0xf7; // clear IE flag

	t_estados+= 8; // assumption
	// do XPPC 3
	scmp_m_PC.w.l = scmp_device_GET_PTR_REG(3)->w.l;
	scmp_device_GET_PTR_REG(3)->w.l = tmp;
	// After exchange CPU increment PC
	scmp_m_PC.w.l = scmp_device_ADD12(scmp_m_PC.w.l,1);
}

/*
void scmp_device_execute_run()
{
	do
	{
		if ((scmp_m_SR & 0x08) && (scmp_m_sensea_func())) {
			scmp_device_take_interrupt();
		}
		//TODO debugger_instruction_hook(this, scmp_m_PC.d);
		scmp_device_execute_one(scmp_device_ROP());

	} while (t_estados> 0);
}*/

/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

void scmp_device_device_start()
{
	/* set up the state table */
	{
	/*
		state_add(SCMP_PC,     "PC",    scmp_m_PC.w.l);
		state_add(STATE_GENPC, "GENPC", scmp_m_PC.w.l).noshow();
		state_add(STATE_GENFLAGS, "GENFLAGS", scmp_m_SR).noshow().formatstr("%8s");
		state_add(SCMP_P1,     "P1",    scmp_m_P1.w.l);
		state_add(SCMP_P2,     "P2",    scmp_m_P2.w.l);
		state_add(SCMP_P3,     "P3",    scmp_m_P3.w.l);
		state_add(SCMP_AC,     "AC",    scmp_m_AC);
		state_add(SCMP_ER,     "ER",    scmp_m_ER);
		state_add(SCMP_SR,     "SR",    scmp_m_SR);
	*/
	}

	/*scmp_m_program = &space(AS_PROGRAM);
	scmp_m_direct = &scmp_m_program->direct();*/

	/* resolve callbacks */
	/*
	scmp_m_flag_out_func.resolve_safe();
	scmp_m_sout_func.resolve_safe();
	scmp_m_sin_func.resolve_safe(0);
	scmp_m_sensea_func.resolve_safe(0);
	scmp_m_senseb_func.resolve_safe(0);
	scmp_m_halt_func.resolve_safe();

	save_item(NAME(scmp_m_PC));
	save_item(NAME(scmp_m_P1));
	save_item(NAME(scmp_m_P2));
	save_item(NAME(scmp_m_P3));
	save_item(NAME(scmp_m_AC));
	save_item(NAME(scmp_m_ER));
	save_item(NAME(scmp_m_SR));

	scmp_m_icountptr = &scmp_m_icount;*/
}



/***************************************************************************
    COMMON RESET
***************************************************************************/

void scmp_reset()
{
	scmp_m_PC.d = 0;
	scmp_m_P1.d = 0;
	scmp_m_P2.d = 0;
	scmp_m_P3.d = 0;
	scmp_m_AC = 0;
	scmp_m_ER = 0;
	scmp_m_SR = 0;
}


void scmp_get_flags_letters(unsigned char f,char *buffer)
{
	sprintf(buffer,"%c%c%c%c%c%c%c%c",
	  (f & 0x80) ? 'C' : '-',
	  (f & 0x40) ? 'V' : '-',
	  (f & 0x20) ? 'B' : '-',
	  (f & 0x10) ? 'A' : '-',
	  (f & 0x08) ? 'I' : '-',
	  (f & 0x04) ? '2' : '-',
	  (f & 0x02) ? '1' : '-',
	  (f & 0x01) ? '0' : '-');
}

/***************************************************************************
    COMMON STATE IMPORT/EXPORT
***************************************************************************/

/*
void scmp_device_state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c%c%c%c%c",
				(scmp_m_SR & 0x80) ? 'C' : '.',
				(scmp_m_SR & 0x40) ? 'V' : '.',
				(scmp_m_SR & 0x20) ? 'B' : '.',
				(scmp_m_SR & 0x10) ? 'A' : '.',
				(scmp_m_SR & 0x08) ? 'I' : '.',
				(scmp_m_SR & 0x04) ? '2' : '.',
				(scmp_m_SR & 0x02) ? '1' : '.',
				(scmp_m_SR & 0x01) ? '0' : '.');
			break;
	}
}
*/




/*
prueba_main()
{

	scmp_device_device_reset();

	//scmp_m_PC.w.l=0x1000;
	//scmp_m_PC.w.h=0;

        //Prueba desensamblado rom
        char buffer[80];
        int i=0;

	memory_data=malloc(65536);

	//memcpy(&memory_data[0x1000],roscmp_m_data,512); //Cargar rom
	memcpy(&memory_data[0x0000],roscmp_m_data,512);

	//scmp_m_PC.w.l=0x52;

        while (1) {

                printf ("%04XH ",scmp_m_PC.w.l);
                int longitud=CPU_DISASSEMBLE( scmp_m_PC.w.l , memory_data[scmp_m_PC.w.l], memory_data[scmp_m_PC.w.l+1], buffer);
		printf ("%02X ",memory_data[scmp_m_PC.w.l]);
		if (longitud>1) printf ("%02X ",memory_data[scmp_m_PC.w.l+1]);
		else printf ("   ");

                 //printf ("longitud: %d %s\n",longitud,buffer);
                 printf ("%s\n",buffer);



	         i ++;

		char temp[100];
		scanf("%s",temp);

		scmp_device_execute_one(scmp_device_ROP());

        }
}
*/




//typedef unsigned char SCMP_UINT_8;
//typedef unsigned int SCMP_UINT_16;

int scmp_CPU_DISASSEMBLE( int pc , unsigned char op, unsigned char arg, char *buffer)
{
	unsigned int PC = pc;
	//SCMP_UINT_8 op = OP(pc++);
	SCMP_UINT_8 ptr = op & 3;

	pc++;

	if (op & 128) {
		// two bytes instructions
		char as[10];
		char aspr[10];
		//SCMP_UINT_8 arg = ARG(pc);
		pc++;
		if (arg==0x80) {
			sprintf(as,"E");
		} else {
			if (arg & 0x80) {
				sprintf(as,"-$%02x",0x100-arg);
			} else {
				sprintf(as,"+$%02x",arg);
			}
		}
		sprintf(aspr,"%s(%d)",as,ptr);

		switch (op)
		{
			// Memory Reference Instructions
			case 0xc0 : sprintf (buffer,"ld %s",as); break;
			case 0xc1 : case 0xc2 : case 0xc3 :
						sprintf (buffer,"ld %s",aspr);break;
			case 0xc5 : case 0xc6 : case 0xc7 :
						sprintf (buffer,"ld @%s",aspr); break;
			case 0xc8 : sprintf (buffer,"st %s",as); break;
			case 0xc9 : case 0xca : case 0xcb :
						sprintf (buffer,"st %s",aspr);break;
			case 0xcd : case 0xce : case 0xcf :
						sprintf (buffer,"st @%s",aspr); break;
			case 0xd0 : sprintf (buffer,"and %s",as); break;
			case 0xd1 : case 0xd2 : case 0xd3 :
						sprintf (buffer,"and %s",aspr);break;
			case 0xd5 : case 0xd6 : case 0xd7 :
						sprintf (buffer,"and @%s",aspr); break;
			case 0xd8 : sprintf (buffer,"or %s",as); break;
			case 0xd9 : case 0xda : case 0xdb :
						sprintf (buffer,"or %s",aspr);break;
			case 0xdd : case 0xde : case 0xdf :
						sprintf (buffer,"or @%s",aspr); break;
			case 0xe0 : sprintf (buffer,"xor %s",as); break;
			case 0xe1 : case 0xe2 : case 0xe3 :
						sprintf (buffer,"xor %s",aspr);break;
			case 0xe5 : case 0xe6 : case 0xe7 :
						sprintf (buffer,"xor @%s",aspr); break;
			case 0xe8 : sprintf (buffer,"dad %s",as); break;
			case 0xe9 : case 0xea : case 0xeb :
						sprintf (buffer,"dad %s",aspr);break;
			case 0xed : case 0xee : case 0xef :
						sprintf (buffer,"dad @%s",aspr); break;
			case 0xf0 : sprintf (buffer,"add %s",as); break;
			case 0xf1 : case 0xf2 : case 0xf3 :
						sprintf (buffer,"add %s",aspr);break;
			case 0xf5 : case 0xf6 : case 0xf7 :
						sprintf (buffer,"add @%s",aspr); break;
			case 0xf8 : sprintf (buffer,"cad %s",as); break;
			case 0xf9 : case 0xfa : case 0xfb :
						sprintf (buffer,"cad %s",aspr);break;
			case 0xfd : case 0xfe : case 0xff :
						sprintf (buffer,"cad @%s",aspr); break;
			// Memory Increment/Decrement Instructions
			case 0xa8 : case 0xa9 : case 0xaa : case 0xab :
						sprintf (buffer,"ild %s",aspr); break;
			case 0xb8 : case 0xb9 : case 0xba : case 0xbb :
						sprintf (buffer,"dld %s",aspr); break;
			// Immediate Instructions
			case 0xc4 : sprintf (buffer,"ldi $%02x",arg); break;
			case 0xd4 : sprintf (buffer,"ani $%02x",arg); break;
			case 0xdc : sprintf (buffer,"ori $%02x",arg); break;
			case 0xe4 : sprintf (buffer,"xri $%02x",arg); break;
			case 0xec : sprintf (buffer,"dai $%02x",arg); break;
			case 0xf4 : sprintf (buffer,"adi $%02x",arg); break;
			case 0xfc : sprintf (buffer,"cai $%02x",arg); break;
			// Transfer Instructions
			case 0x90 : sprintf (buffer,"jmp %s",as);break;
			case 0x91 : case 0x92 : case 0x93 :
						sprintf (buffer,"jmp %s",aspr);break;
			case 0x94 : sprintf (buffer,"jp %s",as); break;
			case 0x95 : case 0x96 : case 0x97 :
						sprintf (buffer,"jp %s",aspr); break;
			case 0x98 : sprintf (buffer,"jz %s",as); break;
			case 0x99 : case 0x9a : case 0x9b :
						sprintf (buffer,"jz %s",aspr); break;
			case 0x9c : sprintf (buffer,"jnz %s",as); break;
			case 0x9d : case 0x9e : case 0x9f :
						sprintf (buffer,"jnz %s",aspr); break;
			// Double-Byte Miscellaneous Instructions
			case 0x8f:  sprintf (buffer,"dly $%02x",arg); break;
			// Others are illegal
			default : sprintf (buffer,"illegal"); pc--; break; // Illegal we consider without param
		}
	} else {
		// one byte instructions
		switch (op)
		{
			// Extension Register Instructions
			case 0x40:  sprintf (buffer,"lde"); break;
			case 0x01:  sprintf (buffer,"xae"); break;
			case 0x50:  sprintf (buffer,"ane"); break;
			case 0x58:  sprintf (buffer,"ore"); break;
			case 0x60:  sprintf (buffer,"xre"); break;
			case 0x68:  sprintf (buffer,"dae"); break;
			case 0x70:  sprintf (buffer,"ade"); break;
			case 0x78:  sprintf (buffer,"cae"); break;
			// Pointer Register Move Instructions
			case 0x30:  case 0x31 :case 0x32: case 0x33:
						sprintf (buffer,"xpal %d",ptr); break;
			case 0x34:  case 0x35 :case 0x36: case 0x37:
						sprintf (buffer,"xpah %d",ptr); break;
			case 0x3c:  case 0x3d :case 0x3e: case 0x3f:
						sprintf (buffer,"xppc %d",ptr); break;
			// Shift, Rotate, Serial I/O Instructions
			case 0x19:  sprintf (buffer,"sio"); break;
			case 0x1c:  sprintf (buffer,"sr"); break;
			case 0x1d:  sprintf (buffer,"srl"); break;
			case 0x1e:  sprintf (buffer,"rr"); break;
			case 0x1f:  sprintf (buffer,"rrl"); break;
			// Single Byte Miscellaneous Instructions
			case 0x00:  sprintf (buffer,"halt"); break;
			case 0x02:  sprintf (buffer,"ccl"); break;
			case 0x03:  sprintf (buffer,"scl"); break;
			case 0x04:  sprintf (buffer,"dint"); break;
			case 0x05:  sprintf (buffer,"ien"); break;
			case 0x06:  sprintf (buffer,"csa"); break;
			case 0x07:  sprintf (buffer,"cas"); break;
			case 0x08:  sprintf (buffer,"nop"); break;
			// Others are illegal
			default : sprintf (buffer,"illegal"); break;
		}
	}

	return (pc - PC);
}

/*
dis_main()
{

	//Prueba desensamblado rom
	char buffer[80];
	int i=0;

	while (i<512) {
		printf ("%04XH ",0x100+i);
		int longitud=CPU_DISASSEMBLE( 0x100+i , dis_romdata[i], dis_romdata[i+1], buffer);
		 //printf ("longitud: %d %s\n",longitud,buffer);
		 printf ("%s\n",buffer);
	  i +=longitud;
	}
}
*/

void scmp_run_opcode(void)
{
	scmp_device_execute_one(scmp_device_ROP());
}
