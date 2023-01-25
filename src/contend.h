/*
    ZEsarUX  ZX Second-Emulator And Released for UniX
    Copyright (C) 2013 Cesar Hernandez Bano

    This file is part of ZEsarUX.

    ZEsarUX is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef CONTEND_H
#define CONTEND_H

#include "cpu.h"


extern int contend_patron_65432100[];

extern int contend_patron_76543210[];

//extern z80_byte contend_table[];
//extern z80_byte contend_table_no_mreq[];

extern z80_byte *contend_table;
extern z80_byte *contend_table_no_mreq;


extern z80_byte *contend_pages_128k_p2a;

extern z80_byte contend_pages_128k[];
extern z80_byte contend_pages_p2a[];

extern z80_byte contend_pages_actual[4];


extern void inicializa_tabla_contend(void);
extern void inicializa_tabla_contend_cached_change_cpu_speed(void);
extern void inicializa_tabla_contend_speed_higher(void);


extern void (*contend_read)(z80_int direccion,int time);
extern void (*contend_read_no_mreq)(z80_int direccion,int time);
extern void (*contend_write_no_mreq)(z80_int direccion,int time);

extern void (*ula_contend_port_early)( z80_int port );
extern void (*ula_contend_port_late)( z80_int port );

extern int (*port_from_ula) (z80_int puerto);

extern void contend_read_ace(z80_int direccion,int time);
extern void contend_read_cpc(z80_int direccion,int time);
extern void contend_read_sam(z80_int direccion,int time);
extern void contend_read_zx8081(z80_int direccion,int time);
extern void contend_read_no_mreq_ace(z80_int direccion,int time);
extern void contend_read_no_mreq_cpc(z80_int direccion,int time);
extern void contend_read_no_mreq_sam(z80_int direccion,int time);
extern void contend_read_no_mreq_zx8081(z80_int direccion,int time);
extern void contend_write_no_mreq_ace(z80_int direccion,int time);
extern void contend_write_no_mreq_cpc(z80_int direccion,int time);
extern void contend_write_no_mreq_sam(z80_int direccion,int time);
extern void contend_write_no_mreq_zx8081(z80_int direccion,int time);
extern void contend_read_48k(z80_int direccion,int time);
extern void contend_read_no_mreq_48k(z80_int direccion,int time);
extern void contend_write_no_mreq_48k(z80_int direccion,int time);
extern void contend_read_128k(z80_int direccion,int time);
extern void contend_read_no_mreq_128k(z80_int direccion,int time);
extern void contend_write_no_mreq_128k(z80_int direccion,int time);

extern void ula_contend_port_early_ace( z80_int port );
extern void ula_contend_port_early_cpc( z80_int port );
extern void ula_contend_port_early_sam( z80_int port );
extern void ula_contend_port_early_zx8081( z80_int port );
extern void ula_contend_port_late_ace( z80_int port );
extern void ula_contend_port_late_cpc( z80_int port );
extern void ula_contend_port_late_sam( z80_int port );
extern void ula_contend_port_late_zx8081( z80_int port );
extern void ula_contend_port_early_48k( z80_int port );
extern void ula_contend_port_late_48k( z80_int port );
extern void ula_contend_port_early_128k( z80_int port );
extern void ula_contend_port_late_128k( z80_int port );

extern void contend_read_z88(z80_int direccion,int time);
extern void contend_read_no_mreq_z88(z80_int direccion,int time);
extern void contend_write_no_mreq_z88(z80_int direccion,int time);
extern void ula_contend_port_early_z88( z80_int port );
extern void ula_contend_port_late_z88( z80_int port );

extern void contend_read_chloe(z80_int direccion,int time);
extern void contend_read_no_mreq_chloe(z80_int direccion,int time);
extern void contend_write_no_mreq_chloe(z80_int direccion,int time);
extern void ula_contend_port_early_chloe( z80_int port );
extern void ula_contend_port_late_chloe( z80_int port );

extern z80_byte contend_pages_chloe[];


extern void contend_read_prism(z80_int direccion,int time);
extern void contend_read_no_mreq_prism(z80_int direccion,int time);
extern void contend_write_no_mreq_prism(z80_int direccion,int time);
extern void ula_contend_port_early_prism( z80_int port );
extern void ula_contend_port_late_prism( z80_int port );




extern void contend_read_timex(z80_int direccion,int time);
extern void contend_read_no_mreq_timex(z80_int direccion,int time);
extern void contend_write_no_mreq_timex(z80_int direccion,int time);
extern void ula_contend_port_early_timex( z80_int port );
extern void ula_contend_port_late_timex( z80_int port );


extern void contend_read_chrome(z80_int direccion,int time);
extern void contend_read_no_mreq_chrome(z80_int direccion,int time);
extern void contend_write_no_mreq_chrome(z80_int direccion,int time);
extern void ula_contend_port_early_chrome( z80_int port );
extern void ula_contend_port_late_chrome( z80_int port );
extern z80_byte contend_pages_chrome[];

extern void contend_read_tsconf(z80_int direccion,int time);
extern void contend_read_no_mreq_tsconf(z80_int direccion,int time);
extern void contend_write_no_mreq_tsconf(z80_int direccion,int time);
extern void ula_contend_port_early_tsconf( z80_int port );
extern void ula_contend_port_late_tsconf( z80_int port );
//extern z80_byte contend_pages_tsconf[];

extern void contend_read_baseconf(z80_int direccion,int time);
extern void contend_read_no_mreq_baseconf(z80_int direccion,int time);
extern void contend_write_no_mreq_baseconf(z80_int direccion,int time);
extern void ula_contend_port_early_baseconf( z80_int port );
extern void ula_contend_port_late_baseconf( z80_int port );

extern void contend_read_mk14(z80_int direccion,int time);
extern void contend_read_no_mreq_mk14(z80_int direccion,int time);
extern void contend_write_no_mreq_mk14(z80_int direccion,int time);
extern void ula_contend_port_early_mk14( z80_int port );
extern void ula_contend_port_late_mk14( z80_int port );
extern z80_byte contend_pages_mk14[];


extern void contend_read_msx1(z80_int direccion,int time);
extern void contend_read_no_mreq_msx1(z80_int direccion,int time);
extern void contend_write_no_mreq_msx1(z80_int direccion,int time);
extern void ula_contend_port_early_msx1( z80_int port );
extern void ula_contend_port_late_msx1( z80_int port );

extern void contend_read_svi(z80_int direccion,int time);
extern void contend_read_no_mreq_svi(z80_int direccion,int time);
extern void contend_write_no_mreq_svi(z80_int direccion,int time);
extern void ula_contend_port_early_svi( z80_int port );
extern void ula_contend_port_late_svi( z80_int port );


extern void contend_read_coleco(z80_int direccion,int time);
extern void contend_read_no_mreq_coleco(z80_int direccion,int time);
extern void contend_write_no_mreq_coleco(z80_int direccion,int time);
extern void ula_contend_port_early_coleco( z80_int port );
extern void ula_contend_port_late_coleco( z80_int port );


extern void contend_read_sg1000(z80_int direccion,int time);
extern void contend_read_no_mreq_sg1000(z80_int direccion,int time);
extern void contend_write_no_mreq_sg1000(z80_int direccion,int time);
extern void ula_contend_port_early_sg1000( z80_int port );
extern void ula_contend_port_late_sg1000( z80_int port );

extern void contend_read_sms(z80_int direccion,int time);
extern void contend_read_no_mreq_sms(z80_int direccion,int time);
extern void contend_write_no_mreq_sms(z80_int direccion,int time);
extern void ula_contend_port_early_sms( z80_int port );
extern void ula_contend_port_late_sms( z80_int port );


extern void contend_read_pcw(z80_int direccion GCC_UNUSED,int time);
extern void contend_read_no_mreq_pcw(z80_int direccion GCC_UNUSED,int time);
extern void contend_write_no_mreq_pcw(z80_int direccion GCC_UNUSED,int time);
extern void ula_contend_port_early_pcw( z80_int port GCC_UNUSED );
extern void ula_contend_port_late_pcw( z80_int port GCC_UNUSED );


extern int port_from_ula_48k (z80_int puerto);
extern int port_from_ula_p2a (z80_int puerto);

extern z80_bit contend_enabled;

//Seria 72000 mas o menos, pero le damos mas para poder jugar con los settings en advanced hardware settings
//Y multiplicamos por el maximo de cpu turbo
#define CONTEND_TABLE_SIZE_ONE_SPEED 100000

#define MAX_CONTEND_TABLE (CONTEND_TABLE_SIZE_ONE_SPEED*MAX_CPU_TURBO_SPEED)


#endif
