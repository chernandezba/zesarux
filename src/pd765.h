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

#ifndef PD765_H
#define PD765_H

#include "cpu.h"

#define PD765_MAIN_STATUS_REGISTER_D0B_MASK 0x01
#define PD765_MAIN_STATUS_REGISTER_D1B_MASK 0x02
#define PD765_MAIN_STATUS_REGISTER_D2B_MASK 0x04
#define PD765_MAIN_STATUS_REGISTER_D3B_MASK 0x08
#define PD765_MAIN_STATUS_REGISTER_CB_MASK  0x10
#define PD765_MAIN_STATUS_REGISTER_EXM_MASK 0x20
#define PD765_MAIN_STATUS_REGISTER_DIO_MASK 0x40
#define PD765_MAIN_STATUS_REGISTER_RQM_MASK 0x80


#define PD765_STATUS_REGISTER_ZERO_NR_MASK 0x08

//Abnormal termination
#define PD765_STATUS_REGISTER_ZERO_AT 0x40

#define PD765_STATUS_REGISTER_ONE_MA_MASK 0x01
#define PD765_STATUS_REGISTER_ONE_NW_MASK 0x02
#define PD765_STATUS_REGISTER_ONE_ND_MASK 0x04
#define PD765_STATUS_REGISTER_ONE_OR_MASK 0x10
#define PD765_STATUS_REGISTER_ONE_DE_MASK 0x20
#define PD765_STATUS_REGISTER_ONE_EN_MASK 0x80

#define PD765_STATUS_REGISTER_TWO_MD_MASK 0x01
#define PD765_STATUS_REGISTER_TWO_BC_MASK 0x02
#define PD765_STATUS_REGISTER_TWO_SN_MASK 0x04
#define PD765_STATUS_REGISTER_TWO_SH_MASK 0x08
#define PD765_STATUS_REGISTER_TWO_WC_MASK 0x10
#define PD765_STATUS_REGISTER_TWO_DD_MASK 0x20
#define PD765_STATUS_REGISTER_TWO_CM_MASK 0x40

#define PD765_STATUS_REGISTER_THREE_FT_MASK 0x80
#define PD765_STATUS_REGISTER_THREE_WP_MASK 0x40
#define PD765_STATUS_REGISTER_THREE_RD_MASK 0x20
#define PD765_STATUS_REGISTER_THREE_T0_MASK 0x10
#define PD765_STATUS_REGISTER_THREE_TS_MASK 0x08
#define PD765_STATUS_REGISTER_THREE_D2_MASK 0x04
#define PD765_STATUS_REGISTER_THREE_D1_MASK 0x02
#define PD765_STATUS_REGISTER_THREE_D0_MASK 0x01

extern z80_byte pd765_main_status_register;

extern z80_bit pd765_enabled;
extern void pd765_enable(void);
extern void pd765_disable(void);

//Para poder hacer debug_printf con la clase PD765 adecuada
#define DBG_PRINT_PD765 debug_printf(VERBOSE_CLASS_PD765|

extern z80_byte pd765_read(void);
extern z80_byte pd765_read_status_register(void);
extern void pd765_out_port_data_register(z80_byte value);
extern z80_bit pd765_terminal_count_signal;

//extern void pd765_out_port_1ffd(z80_byte value);

extern void pd765_set_terminal_count_signal(void);
extern void pd765_reset_terminal_count_signal(void);


extern void pd765_reset(void);

extern int pd765_interrupt_pending;
extern int pd765_pcn;
extern int pd765_motor_status;

//Estructura para tratamiento de senyales con contador
struct s_pd765_signal_counter {
    int current_counter; //inicializar a 0
    int value;  //valor actual de la señal. inicializar a 0
    int running; //indica contador ejecutandose. inicializar a 0
    int max;     //valor maximo a partir del cual se pasa a 1, inicializar con valor deseado

    //Funcion que se llama al activar valor
    void (*function_triggered)(void);
    
};

typedef struct s_pd765_signal_counter pd765_signal_counter;

extern pd765_signal_counter signal_se;

enum pd765_command_list {
    PD765_COMMAND_SPECIFY,
    PD765_COMMAND_SENSE_DRIVE_STATUS,
    PD765_COMMAND_RECALIBRATE,
    PD765_COMMAND_SENSE_INTERRUPT_STATUS,
    PD765_COMMAND_SEEK,
    PD765_COMMAND_READ_ID,
    PD765_COMMAND_READ_DATA,
    PD765_COMMAND_READ_DELETED_DATA,
    PD765_COMMAND_READ_TRACK,
    PD765_COMMAND_WRITE_DATA,
    PD765_COMMAND_FORMAT_TRACK,
    PD765_COMMAND_INVALID
};

extern const char *pd765_last_command_name(void);

extern enum pd765_command_list pd765_command_received;

extern void pd765_next_event_from_core(void);

extern int pd765_debug_last_sector_read;

extern int pd765_read_stats_bytes_sec;
extern void pd765_read_stats_update(void);

extern int pd765_write_stats_bytes_sec;
extern void pd765_write_stats_update(void);

extern z80_byte pd765_debug_last_sector_id_c_read;
extern z80_byte pd765_debug_last_sector_id_h_read;
extern z80_byte pd765_debug_last_sector_id_r_read;
extern z80_byte pd765_debug_last_sector_id_n_read;

extern int pd765_motor_speed;
extern void pd765_handle_speed_motor(void);

extern void pd765_motor_on(void);
extern void pd765_motor_off(void);

extern void cf2_floppy_icon_activity(void);

extern int cf2_floppy_icon_is_saving;

#endif
