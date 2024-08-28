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

#ifndef Z88_H
#define Z88_H


//Para usar PATH_MAX
#include "zesarux.h"

#include "cpu.h"
#include "utils.h"


extern char valor_sonido_3200hz;
extern z80_byte blink_mapped_memory_banks[];
extern z80_int blink_pixel_base[];
extern z80_int blink_sbr;
extern z80_byte blink_tim[];
extern z80_byte blink_tmk;
extern z80_byte blink_tsta;


extern z80_byte blink_sta;
extern z80_byte blink_int;


//temporal solo para debug
//extern z80_byte blink_tack;


struct s_z88_memory_slot {
        //tamanyo-1 (para asi usar este valor como mascara)
        z80_long_int size;


        //antiguo: 0 RAM, 1 ROM, 2 EPROM, 3 FLASH
	//0 RAM, 2 EPROM, 3 FLASH, 4 Hybrida 512kb RAM+512 kb EPROM
        z80_byte type;

	//Identificador del fabricante de la flash, de momento solo soporta Intel I28Fxxxx FlashFile Memory chips (0x89)
	z80_byte flash_manufacturer_code;

	//Modelo de la flash
	z80_byte flash_device_code;
	//I28F004S5 = 0xA7 Device Code for 512Kb memory, 8 x 64K erasable sectors, 32 x 16K banks
	//I28F008S5 = 0xA6 Device Code for 1Mb memory, 16 x 64K erasable sectors, 64 x 16K banks

	//Si la tarjeta flash esta en modo comando
	z80_bit z88_flash_card_in_command_mode;

	//Comando ejecutandose en la flash
	z80_byte executing_command_number;

	//Registro de estado de la flash
	z80_byte statusRegister;


	//si slot vacio, size=0.
	// y tambien tipo=0 (RAM). Sino hay funciones, por ejemplo flush eprom, que aunque size sea 0, si tipo es eprom, intenta hacer flush


	//offset dentro de la memoria de 4 MB donde comienza el slot:
        //        0,
        //        0x40*16384,
        //        0x80*16384,
        //        0xc0*16384

	z80_long_int offset_total;

	//nombre del archivo cargado para eprom y flash
	//en caso de insertar eprom/flash desde menu o linea de comandos, contendra el nombre
	//en caso de cargar un snapshot que contenga un eprom/flash en slots 1 o 2, no viene el nombre en el snapshot y por tanto aqui no habra nada
	char eprom_flash_nombre_archivo[PATH_MAX];

};

typedef struct s_z88_memory_slot z88_memory_slot;

extern z88_memory_slot z88_memory_slots[];

extern z80_long_int z88_internal_rom_size;
extern z80_long_int z88_internal_ram_size;

extern z80_bit z88_eprom_or_flash_persistent_writes;

//Para algunas funciones que usan direcciones del z88
struct s_z88_dir {
	z80_byte bank;
	z80_int dir;
};

typedef struct s_z88_dir z88_dir;


struct s_z88_eprom_flash_file {

/*

Estructura del filesystem en eprom y flash:

1 byte      n           length of filename
1 byte      x           '/' for latest version, $00 for old version (deleted)
n-1 bytes   'xxxx'      filename
4 bytes     m           length of file (least significant byte first)
m bytes                 body of file

archivo borrado, empieza con byte 0 en el nombre, en vez de '/'
en una flash se puede haber borrado desde flashstore
en eprom, si se guarda un archivo con mismo nombre que otro que ya habia, el anterior se marca como borrado
en eprom esto es posible porque el byte '/' se convierte a 00 (en una eprom se puede pasar siempre de bit 1 a 0, cosa como en este caso,
pero no al reves, de bit 0 a 1) entonces una eprom no puede pasar de 00 a '/', esto solo lo hace el menu del emulador,
en la opcion undelete, porque se "salta" las normas de escritura de eprom


En tarjetas flash, el flashstore las inicializa con un archivo nulo:

01   00    00 00 00 00

Borrado, longitud nombre 1, nombre byte 0 (borrado), con 0 longitud. Porque? Respuesta:

Oh yeah, that has a story!
We discovered very early on the development of the Intel flash libraries
that the Flash Chip would go into command mode without us understanding why.

We believe it has to do with what data you put in the bottom bank
(memory) on that chip.

So, we solved it by putting some null data... when formatting a file area.

This is only relevant for real intel flash cards. The null-file is not
displayed by either the Filer nor FlashStore. It's an end-user secret.


*/

	//longitud nombre incluyendo /. si 255, no hay archivo
	z80_byte namelength;

	//nombre incluyendo / y sin 0 del final
	z80_byte name[256];

	//Tamanyo en mismo orden que vienen en la eprom (byte menos significativo primero)
	z80_byte size[4];

	//Puntero a donde estan los datos
	z88_dir datos;

	//Mismo puntero a datos pero direccion de memoria, para funciones _new_ptr
	z80_byte *datos_ptr;

};

typedef struct s_z88_eprom_flash_file z88_eprom_flash_file;

extern void z88_set_default_memory_pages(void);

//extern z80_byte *z88_puntero_internal_rom;
//extern z80_byte *z88_puntero_internal_ram;

extern z80_byte *z88_puntero_memoria;

extern z80_byte *z88_return_memory_pointer(z80_int direccion);

extern z80_byte z88_retorna_bank_low(z80_byte bank,z80_int *direccion);

extern z80_byte z88_port_b0;

extern void poke_byte_z88(z80_int dir,z80_byte valor);
extern void poke_byte_no_time_z88(z80_int dir,z80_byte valor);
extern z80_byte peek_byte_z88(z80_int dir);
extern z80_byte peek_byte_no_time_z88(z80_int dir);


extern void poke_byte_no_time_z88_bank(z80_int dir,z80_byte bank,z80_byte valor);
extern z80_byte peek_byte_no_time_z88_bank(z80_int dir,z80_byte bank);

extern z80_byte peek_byte_no_time_z88_bank_no_check_low(z80_int dir,z80_byte bank);


extern z80_byte lee_puerto_z88(z80_byte h,z80_byte l);
extern void out_port_z88(z80_int puerto,z80_byte value);


extern z80_bit z88_snooze;
extern z80_bit z88_coma;

extern void z88_notificar_tecla(void);

extern void z88_debug_mostrar_sbf(void);

extern void hard_reset_cpu_z88(void);

extern z80_byte blink_kbd_a15;
extern z80_byte blink_kbd_a14;
extern z80_byte blink_kbd_a13;
extern z80_byte blink_kbd_a12;
extern z80_byte blink_kbd_a11;
extern z80_byte blink_kbd_a10;
extern z80_byte blink_kbd_a9;
extern z80_byte blink_kbd_a8;

extern z80_byte blink_com;

extern z80_byte blink_epr;

extern z80_byte blink_rxd;

extern z80_byte blink_rxe;

extern z80_byte blink_rxc;

extern z80_byte blink_txd;

extern z80_byte blink_txc;

extern z80_byte blink_umk;

extern z80_byte blink_uit;

extern void init_z88_memory_slots(void);

//extern void z88_load_rom_card(char *archivo, int slot);
extern void z88_load_eprom_card(char *archivo, int slot);
extern void z88_load_flash_intel_card(char *archivo, int slot);
extern void z88_load_hybrid_eprom_card(char *archivo, int slot);
extern void z88_insert_ram_card(int size,int slot);

extern char *z88_memory_types[];

extern void z88_erase_eprom_flash(void);

extern void z88_flush_eprom_or_flash_to_disk(void);

extern int z88_eprom_or_flash_must_flush_to_disk;

extern int z88_footer_timer_slot3_activity_indicator;

extern int z88_slot3_activity_indicator;

extern void z88_reset_slot3_activity_indicator(void);

extern int z88_create_blank_eprom_flash_file(char *nombre,int size);

extern void z88_change_internal_ram(int size);

extern void z88_increment_pointer(z88_dir *dir);
extern void z88_add_pointer (z88_dir *dir,z80_long_int size);

extern void z88_eprom_flash_find_init(z88_dir *dir,int slot);
extern int z88_eprom_flash_find_next(z88_dir *dir,z88_eprom_flash_file *file);
extern void z88_debug_print_eprom_flash_file(z88_eprom_flash_file *file);
extern void z88_eprom_flash_get_file_name(z88_eprom_flash_file *file,char *nombre);



extern void z88_return_eprom_flash_file (z88_dir *dir,z88_eprom_flash_file *file);
extern int z88_write_eprom_flash_file(z88_dir *dir,z88_eprom_flash_file *file,z80_byte *datos);

//extern void z88_find_eprom_flash_free_space (z88_dir *dir);
extern void z88_find_eprom_flash_free_space (z88_dir *dir,int slot);

extern void z88_find_eprom_flash_file (z88_dir *dir,z88_eprom_flash_file *file,char *nombre, int slot);
extern int z88_eprom_flash_fwrite(char *nombre,z80_byte *datos,z80_long_int longitud);

//extern void z88_eprom_flash_free(z80_long_int *total_eprom,z80_long_int *used_eprom, z80_long_int *free_eprom);
extern void z88_eprom_flash_free(z80_long_int *total_eprom,z80_long_int *used_eprom, z80_long_int *free_eprom,int slot);

extern z80_long_int z88_eprom_flash_reclaim_free_space(void);

extern z80_long_int z88_eprom_flash_recover_deleted(void);

extern char z88_get_beeper_sound(void);

extern z80_byte z88_return_keyboard_port_value(z80_byte puerto_h);

extern void z88_awake_from_coma(void);
extern void z88_enable_coma(void);
extern void z88_awake_from_snooze(void);
extern void z88_enable_snooze(void);

#define Z88_T_ESTADOS_COMA_SNOOZE 200

extern unsigned int z88_contador_para_flap;

extern int z88_flap_is_open(void);

extern z80_bit estado_parpadeo_cursor;


extern z80_byte z88_get_bank_slot(int slot);

extern void notificar_tecla_interrupcion_si_z88(void);

extern int z88_return_card_type (int slot);

extern int z88_eprom_new_ptr_flash_find_next(z80_byte **ptr_dir,z88_eprom_flash_file *file);

extern int z88_get_total_ram(void);

extern int do_not_run_init_z88_memory_slots;

extern int z88_pendiente_cerrar_tapa_timer;

extern void z88_close_flap_ahora(void);

extern void z88_open_flap(void);

extern void z88_set_system_clock_to_z88(void);


#define BM_COMSRUN      0x80
// Bit 7, SRUN        Speaker source (0=SBIT, 1=TxD or 3200hz)

#define BM_COMSBIT      0x40
//Bit 6, SBIT        SRUN=0: 0=low, 1=high; SRUN=1: 0=3200 hz, 1=TxD

#define BM_COMOVERP     0x20
//Bit 5, OVERP       Set to overprogram EPROM s

#define BM_COMRESTIM    0x10
//Bit 4, RESTIM      Set to reset the RTC, clear to continue

#define BM_COMPROGRAM   0x08
//Bit 3, PROGRAM     Set to enable EPROM  programming

#define BM_COMRAMS      0x04
//Bit 2, RAMS        Binding of lower 8K of segment 0: 0=bank 0, 1=bank 20

#define BM_COMVPPON     0x02
//Bit 1, VPPON       Set to turn programming voltage ON

#define BM_COMLCDON     0x01
//Bit 0, LCDON       Set to turn LCD ON, clear to turn LCD OFF



#define BM_INTKWAIT 0x80
// Bit 7, If set, reading the keyboard will Snooze

#define BM_INTA19 0x40
// Bit 6, If set, an active high on A19 will exit Coma

#define BM_INTFLAP 0x20
// Bit 5, If set, flap interrupts are enabled

#define BM_INTUART 0x10
// Bit 4, If set, UART interrupts are enabled

#define BM_INTBTL 0x08
// Bit 3, If set, battery low interrupts are enabled

#define BM_INTKEY 0x04
// Bit 2, If set, keyboard interrupts (Snooze or Coma) are enabl.

#define BM_INTTIME 0x02
// Bit 1, If set, RTC interrupts are enabled

#define BM_INTGINT 0x01
// Bit 0, If clear, no interrupts get out of blink


#define BM_STAFLAPOPEN 0x80
// Bit 7, If set, flap open, else flap closed

#define BM_STAA19 0x40
// Bit 6, If set, high level on A19 occurred during coma

#define BM_STAFLAP 0x20
// Bit 5, If set, positive edge has occurred on FLAPOPEN

#define BM_STAUART 0x10
// Bit 4, If set, an enabled UART interrupt is active

#define BM_STABTL 0x08
// Bit 3, If set, battery low pin is active

#define BM_STAKEY 0x04
// Bit 2, If set, a column has gone low in snooze (or coma)

#define BM_STATIME 0x01
// Bit 0, If set, an enabled TSTA interrupt is active

#define BM_TMKTICK 0x01

#define BM_TSTATICK 0x01



        // Set to enable minute interrupt
#define BM_TMKMIN 0x04

        // Set to enable second interrupt
#define BM_TMKSEC 0x02
        // Set to enable tick interrupt
#define BM_TMKTICK 0x01
        // Set to acknowledge minute interrupt
#define BM_TACKMIN 0x04
        // Set to acknowledge second interrupt
#define BM_TACKSEC 0x02
        // Set to acknowledge tick interrupt
#define BM_TACKTICK 0x01


        // Set if minute interrupt has occurred
#define BM_TSTAMIN 0x04
        // Set if second interrupt has occurred
#define BM_TSTASEC 0x02
        // Set if tick interrupt has occurred
#define BM_TSTATICK 0x01

#define Z88_MAX_CARD_FILENAME 16


#endif
