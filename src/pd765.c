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

/*

Hello there... I'm glad you are curious and want to know more about my code. 

First what you have to know is that this pd765.c code is not optimized, but created
in order to be an understandable code. In fact, this is the second version of this code, there
was a first version, that you may find on ZEsarUX versions before 10.3, that had the first version of this code,
which didn't work at all. 

That first version of the code was started on ZEsarUX 4.1 (16 July 2016), it didn't worked at all,
and was left untouched until version 7.0 (25 May 2018) where I added +3DOS Traps. 
That was a quick way to have it working, actually it was trapping
all +3DOS calls and simulate to use disk. That was far from being perfect and only 10% of games were working.
Also, the pd765 layer was not fixed. That code was left untouched again during 4 years...

Then, when I started ZEsarUX 10.3 (27 October 2022), I wanted to fix pd765 emulation, so I had to rewrite it from scratch.
I erased the old pd765.c and created a new blank one. Started again to read the chip specifications and slowly created, this time,
a new pd765 emulation layer that really worked.

Fun fact: The old code didn't work because I was interpreting the READ DATA command in a bad way: instead of returning first the sector data, 
and then the ST0,1,2,CHRN values, I was returning them inverted: first ST0,1,2,CHRN values and then the sector data... Funny, isn't it?

Anyway, as I said, don't expect it to be an optimized code, it was created to be understandable, and also
as a way to know how the PD765 chip worked, as I knew almost nothing before.
Also, there are some chip commands that are not implemented yet. There's a reason: I don't have any program or game that uses them.
So, instead of adding them and not trying with a real program (I could create a test program but it would work according to what
I understand from the chip, that could be wrong), I prefer to have them unemulated. 
So, if you have a real test program, which works on a real +3 Spectrum machine, and there in ZEsarUX shows an error like 
"Invalid command", please tell me, so I could add that command and test it.

So, the list of the unemulated commands are:

WRITE DELETED DATA
SCAN EQUAL
SCAN LOW OR EQUAL
SCAN HIGH OR EQUAL

Also, there could be (probably) some features or behaviour of some commands that are not emulated the right way.
I have tested my code with lots of disks (943 exactly) and tried to have it working the best way. 
There are still known bugs and disks that can't be read right, I have collected my results in the following list:

Spectrum +3 Disks:
-Unprotected games:              98.3 % working
-Paul Owens protected games:     85.7 % working
-Speedlock protected games:      54.2 % working
-Alkatraz protected games:          0 % working
-Unknown method protected games:    0 % working (a total of 15 disks that seem to be protected but don't know the method)

Enjoy reading!

*/

/*

Some useful documentation:

EL CONTROLADOR DE DISQUETES NEC 765
http://galia.fc.uaslp.mx/~cantocar/ARQUI_COM_II/LIBRO%20IBM-PC/1206.html


Format:DSK disk image file format - CPCWiki
https://www.cpcwiki.eu/index.php/Format:DSK_disk_image_file_format

More information
https://worldofspectrum.org/faq/reference/diskreference.htm#ZX

Operating Systems Development Series
http://www.brokenthorn.com/Resources/OSDev20.html


http://dunfield.classiccmp.org/r/765.pdf


*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>


#include "pd765.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "dsk.h"
#include "menu_items.h"
#include "screen.h"
#include "settings.h"
#include "pcw.h"

z80_bit pd765_enabled={0};



/*
Main status register

Bit Number  Name                Symbol      Description
----------  -----------         ------      --------------
DB0         FDD 0 Busy          D0B         FDD Number 0 is in seek mode. If any of the bits is set FDC will not accept read or write command
DB1         FDD 1 Busy          D1B         FDD Number 1 is in seek mode. If any of the bits is set FDC will not accept read or write command
DB2         FDD 2 Busy          D2B         FDD Number 2 is in seek mode. If any of the bits is set FDC will not accept read or write command
DB3         FDD 3 Busy          D3B         FDD Number 3 is in seek mode. If any of the bits is set FDC will not accept read or write command
DB4         FDC Busy            CB          A read or write command is in process. FDC will not accept any other command
DB5         Execution Mode      EXM         This bit is set only during execution phase in non-DMA mode. When DB5 goes low, execution phase has ended,
                                            and result phase was started. It operates only during NON-DMA mode of operation
DB6         Data Input/Output   DIO         Indicates direction of data transfer between FDC and Data Register. If DIO = "1" then transfer is from
                                            Data Register to the Processor. If DIO = "0", then transfer is from the Processor to Data Register
DB7         Request for Master  RQM         Indicates Data Register is ready to send or receive data to or from the Processor. Both bits DIO and RQM
                                            should be used to perform the hand-shaking functions of "ready" and "direction" to the processor

*/

//TODO: Tengo mis dudas con el significado del bit:
//DB4         FDC Busy            CB          A read or write command is in process. FDC will not accept any other command
//Mickey mouse usa este bit
//Se activa este bit solo con comandos que implican lectura o escritura? Como read data o read id?
//O tambien con comandos como sense interrupt... En este caso "A read or write command is in process" querria decir:
//"cuando este en curso una lectura o escritura de parametros", cosa que no pareceria logica, porque se activaria
//siempre que esta en medio de un comando


#define PD765_MAIN_STATUS_REGISTER_ON_BOOT PD765_MAIN_STATUS_REGISTER_RQM_MASK

z80_byte pd765_main_status_register=PD765_MAIN_STATUS_REGISTER_ON_BOOT;



#define PD765_PHASE_COMMAND     0
#define PD765_PHASE_EXECUTION   1
#define PD765_PHASE_RESULT      2

#define PD765_PHASE_ON_BOOT PD765_PHASE_COMMAND

//Fase en la que está la controladora
int pd765_phase=PD765_PHASE_ON_BOOT;


//Indice en el retorno de resultados de un comando
int pd765_output_parameters_index=0;

//Indice en la recepción de parámetros de un comando
int pd765_input_parameters_index=0;



enum pd765_command_list pd765_command_received;

//Parámetros recibidos en un comando
z80_byte pd765_input_parameter_hd;
z80_byte pd765_input_parameter_us0;
z80_byte pd765_input_parameter_us1;
z80_byte pd765_input_parameter_c;
z80_byte pd765_input_parameter_h;
z80_byte pd765_input_parameter_r;
z80_byte pd765_input_parameter_n;
z80_byte pd765_input_parameter_n_format;
z80_byte pd765_input_parameter_eot;
z80_byte pd765_input_parameter_gpl;
z80_byte pd765_input_parameter_dtl;
z80_byte pd765_input_parameter_srt;
z80_byte pd765_input_parameter_hut;
z80_byte pd765_input_parameter_hlt;
z80_byte pd765_input_parameter_nd;
z80_byte pd765_input_parameter_ncn;
z80_byte pd765_input_parameter_sc;
z80_byte pd765_input_parameter_d;


z80_byte pd765_input_parameter_mt;
z80_byte pd765_input_parameter_mf;
z80_byte pd765_input_parameter_sk;


//Signal TS0 de ST3
z80_bit pd765_signal_ts0={0};

//Interrupcion pendiente de la controladora
int pd765_interrupt_pending=0;

//Terminal Count Signal. Usado por PCW
//Que yo sepa, +3 o CPC no tienen manera de acceder a esta señal
//z80_bit pd765_terminal_count_signal={0};

//Cilindro actual
int pd765_pcn=0;

//Ultimo sector fisico leido. En principio esto solo se usa para debug
int pd765_debug_last_sector_read=0;

//Ultimo sector fisico escrito. En principio esto solo se usa para debug
int pd765_debug_last_sector_write=0;


//Ultimos id de sector leidos. En principio esto solo se usa para debug
z80_byte pd765_debug_last_sector_id_c_read=0;
z80_byte pd765_debug_last_sector_id_h_read=0;
z80_byte pd765_debug_last_sector_id_r_read=0;
z80_byte pd765_debug_last_sector_id_n_read=0;

//Ultimo sector fisico leido por comandos read data y read_id
int pd765_ultimo_sector_fisico_read=-1;

//Ultimo sector fisico leido por comandos write data
int pd765_ultimo_sector_fisico_write=-1;


//int tempp_estados=0;

//Estado motor. 0 apagado, 1 activado
int pd765_motor_status=0;

//Velocidad relativa del motor: 0%: detenido del todo, 100%: iniciado del todo
//De momento solo usado en visual floppy
int pd765_motor_speed=0;



//En cuanto se acelera o frena el motor por cada frame de pantalla (o sea cada 20 ms)
//Al parecer al hacer un cat a:, desde un motor on, hasta el primer comando (recalibrate) pasa 1 segundo
#define PD765_INCREMENT_PERCENTAGE_MOTOR 2

//Gestion de velocidad del motor.
void pd765_handle_speed_motor(void)
{
    //DBG_PRINT_PD765 VERBOSE_DEBUG,"estados: %d speed: %d",tempp_estados++,pd765_motor_speed);

    if (pd765_motor_status) {
        //Iniciado. Llevar hasta 100% velocidad
        if (pd765_motor_speed<=100) {
            pd765_motor_speed +=PD765_INCREMENT_PERCENTAGE_MOTOR;
            if (pd765_motor_speed>100) pd765_motor_speed=100;
        }
    }
    else {
        //Detenido. Llevar velocidad hasta 0%
        if (pd765_motor_speed>0) {
            pd765_motor_speed -=PD765_INCREMENT_PERCENTAGE_MOTOR;
            if (pd765_motor_speed<0) pd765_motor_speed=0;
        }
    }
}

//ultimo valor de bytes/segundo
int pd765_read_stats_bytes_sec=0;
//Acumulado hasta ahora
int pd765_read_stats_bytes_sec_acumulated=0;

//Estadisticas de lectura de pd765
void pd765_read_stats_update(void)
{
    pd765_read_stats_bytes_sec=pd765_read_stats_bytes_sec_acumulated;

    pd765_read_stats_bytes_sec_acumulated=0;
}


//ultimo valor de bytes/segundo
int pd765_write_stats_bytes_sec=0;
//Acumulado hasta ahora
int pd765_write_stats_bytes_sec_acumulated=0;

//Estadisticas de lectura de pd765
void pd765_write_stats_update(void)
{
    pd765_write_stats_bytes_sec=pd765_write_stats_bytes_sec_acumulated;

    pd765_write_stats_bytes_sec_acumulated=0;
}

//
//Gestion de tratamiento de senyales con contador
//


//tratamiento de senyales de contador (sc=signal counter)
//establecer a 0
void pd765_sc_reset(pd765_signal_counter *s)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: reset signal");
    s->current_counter=0;
    s->running=0;
    s->value=0;
}

//establecer a 1
void pd765_sc_set(pd765_signal_counter *s)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: set signal");
    s->current_counter=0;
    s->running=0;
    s->value=1;
}

//Incrementar si esta running y cambiar a 1 si llega al limite
void pd765_sc_handle_running(pd765_signal_counter *s,int incremento)
{
    if (s->running) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: handle signal running. Current counter: %d max: %d",s->current_counter,s->max);
        (s->current_counter)+=incremento;
        if ((s->current_counter)>=(s->max)) {
            DBG_PRINT_PD765 VERBOSE_DEBUG," PD765: Activar senyal");
            pd765_sc_set(s);

            s->function_triggered();
        }
    }
}

//Activar contador
void pd765_sc_set_running(pd765_signal_counter *s)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: set signal running");
    s->running=1;
}

//Activar contador, reiniciando desde 0 y con valor 0
void pd765_sc_initialize_running(pd765_signal_counter *s)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: set initialize signal running");
    pd765_sc_reset(s);
    pd765_sc_set_running(s);
}

//obtener valor
int pd765_sc_get(pd765_signal_counter *s)
{
    return s->value;
}


//
//FIN Gestion de tratamiento de senyales con contador
//


void pd765_set_interrupt_pending(void)
{

    //if (pd765_interrupt_pending) return;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Set Interrupt pending");

    pd765_interrupt_pending=1;

    if (MACHINE_IS_PCW) {
        pcw_interrupt_from_pd765();
    }
}

//Decir que el seek que se estaba ejecutando era un recalibrate
z80_bit pd765_seek_was_recalibrating={0};


void pd765_signal_se_function_triggered(void)
{
    
    pd765_pcn=pd765_input_parameter_ncn;

    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: seek has finished. Changing PCN from NCN: %d",pd765_pcn);

    //Controlar limite seek.
    //TODO: realmente hay que controlar esto en el seek? quiza no, quiza
    //se controla luego que al hacer un read data no estemos leyendo mas alla del total de pistas...
    if (pd765_pcn>=dsk_get_total_tracks()) {
        //TODO: ni deberia empezar el seek con esto
        debug_printf(VERBOSE_ERR,"PD765: seek BEYOND limit: %d",pd765_pcn);
    }


    //E indicar fase ejecucion ha finalizado
    pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);

    //Decir RQM
    //pd765_main_status_register |= PD765_MAIN_STATUS_REGISTER_RQM_MASK;

    //TODO: No tengo claro porque de esto. la ROM necesita esto para salir del bucle cerrado
    //pd765_main_status_register &= (0xFF - PD765_MAIN_STATUS_REGISTER_DIO_MASK);

    //TODO: correcto esto aqui?
    pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_D0B_MASK - PD765_MAIN_STATUS_REGISTER_D1B_MASK - PD765_MAIN_STATUS_REGISTER_D2B_MASK - PD765_MAIN_STATUS_REGISTER_D3B_MASK);                

    pd765_phase=PD765_PHASE_COMMAND;

    //Avisar interrupcion pendiente de la controladora
    pd765_set_interrupt_pending();

    //Y si era un recalibrate, disparar senyal
    if (pd765_seek_was_recalibrating.v) pd765_signal_ts0.v=1;
    else pd765_signal_ts0.v=0;


    //Decir anterior sector leido de esa pista
    pd765_ultimo_sector_fisico_read=-1;

}

//Signal SE de ST0. Cambio a valor 1 cuando se consulta 5 veces
pd765_signal_counter signal_se={
    0,0,0,
    //5,pd765_signal_se_function_triggered

    //Norte y sur: 97 t-estados para mover cabezal de la pista 0 a la 1
    80,pd765_signal_se_function_triggered
};


//
//Gestion de buffer de respuesta
//

//TODO: todos los comandos que devuelvan resultado deberian usar este buffer

//Maximo sector es de 8kb, esto hay mas que suficiente
#define PD765_MAX_RESULT_BUFFER 9000
//El buffer de resultado
z80_byte pd765_result_buffer[PD765_MAX_RESULT_BUFFER];
//Y cuantos datos hay en el buffer para retornarlos
int pd765_result_buffer_length=0;

int pd765_result_bufer_read_pointer=0;

//int pd765_result_bufer_write_pointer=0;

//Por si acaso funciones para escribir y leer del buffer y que no nos salgamos
void pd765_reset_buffer(void)
{
    pd765_result_buffer_length=0;
    pd765_result_bufer_read_pointer=0;
}

int pd765_buffer_read_is_final(void)
{
    if (pd765_result_bufer_read_pointer<0 || pd765_result_bufer_read_pointer>=pd765_result_buffer_length) return 1;
    else return 0;
}

z80_byte pd765_get_buffer(void)
{
    if (pd765_result_bufer_read_pointer<0 || pd765_result_bufer_read_pointer>=pd765_result_buffer_length) {
        debug_printf(VERBOSE_ERR,"Error getting PD765 buffer beyond limit: %d",pd765_result_bufer_read_pointer);
        return 0;
    }
    else {
        return pd765_result_buffer[pd765_result_bufer_read_pointer++];
    }
}



void pd765_put_buffer(z80_byte value)
{
    
    if (pd765_result_buffer_length>=PD765_MAX_RESULT_BUFFER) {
        debug_printf(VERBOSE_ERR,"Error putting PD765 buffer beyond limit: %d",pd765_result_buffer_length);
        return;
    }
    pd765_result_buffer[pd765_result_buffer_length++]=value;
}



void pd765_reset(void)
{
    pd765_main_status_register=PD765_MAIN_STATUS_REGISTER_ON_BOOT;
    pd765_phase=PD765_PHASE_ON_BOOT;
    pd765_input_parameters_index=0;
    pd765_output_parameters_index=0;
    pd765_signal_ts0.v=0;
    pd765_pcn=0;
    pd765_interrupt_pending=0;
    pd765_motor_status=0;
    pd765_ultimo_sector_fisico_read=-1;
    pd765_ultimo_sector_fisico_write=-1;

    pd765_sc_reset(&signal_se);
    pd765_seek_was_recalibrating.v=0;
    //pd765_terminal_count_signal.v=0;
    pd765_reset_buffer();
}

z80_bit pd765_enabled;
void pd765_enable(void)
{
    if (pd765_enabled.v) return;

    debug_printf (VERBOSE_INFO,"Enabling PD765");
    pd765_enabled.v=1;

}

void pd765_disable(void)
{
    if (pd765_enabled.v==0) return;

    debug_printf (VERBOSE_INFO,"Disabling PD765");
    pd765_enabled.v=0;
}

void pd765_motor_on(void)
{
    if (!pd765_motor_status) {
        pd765_motor_status=1;
        DBG_PRINT_PD765 VERBOSE_INFO,"PD765: Motor on PC=%04XH",reg_pc);
    }
}
void pd765_motor_off(void)
{
    if (pd765_motor_status) {
        pd765_motor_status=0;
        DBG_PRINT_PD765 VERBOSE_INFO,"PD765: Motor off PC=%04XH",reg_pc);
    }
}

int pd765_ultimo_t_estados=-1;

void pd765_next_event_from_core(void)
{

    //TODO De momento solo hacer eventos de seek
    if (pd765_enabled.v) {
        //Vamos a saber cuantos t-estados han pasado desde el anterior

        //Estado inicial. no sabemos cuanto ha transcurrido
        if (pd765_ultimo_t_estados<0) pd765_ultimo_t_estados=t_estados;

        int diferencia=t_estados-pd765_ultimo_t_estados;

        //Si t_estados ha dado la vuelta, la resta sera <0
        if (diferencia<0) {

            //del anterior hasta fin de frame
            int diferencia_hasta_fin_frame=screen_testados_total-pd765_ultimo_t_estados;

            //y sumar lo de ahora
            diferencia=diferencia_hasta_fin_frame+t_estados;

            //Si sale negativo (que no deberia) dejarlo tal cual
            if (diferencia<0) diferencia=0;

            if (signal_se.running) DBG_PRINT_PD765 VERBOSE_DEBUG,"STATES: end of frame");
        }

        if (signal_se.running) DBG_PRINT_PD765 VERBOSE_DEBUG,"STATES: diference: %d last: %d now: %d",diferencia,pd765_ultimo_t_estados,t_estados);

        pd765_sc_handle_running(&signal_se,diferencia);

        pd765_ultimo_t_estados=t_estados;
    }
}

z80_byte pd765_get_st0(void)
{

    //TODO completar BIEN esto


    z80_byte return_value=(pd765_sc_get(&signal_se) * 32) | (pd765_input_parameter_hd<<2) | (pd765_input_parameter_us1<<1) | pd765_input_parameter_us0;


    return return_value;
}


z80_byte pd765_get_st1(void)
{
    //TODO

    return 0;
}


z80_byte pd765_get_st2(void)
{
    //TODO

    return 0;
}




z80_byte pd765_get_st3(void)
{
    /*
    Bit Name                Symbol  Description
    D7  Fault               FT      This bit is used to indicate the status of the Fault signal from the FDD
    D6  Write Protected     WP      This bit is used to indicate the status of the Write Protected signal from the FDD
    D5  Ready               RY      This bit is used to indicate the status of the Ready signal from the FDD
    D4  Track 0             T0      This bit is used to indicate the status of the Track 0 signal from the FDD
    D3  Two Side            TS      This bit is used to indicate the status ot the Two Side signal from the FDD
    D2  Head Address        HD      This bit is used to indicate the status of Side Select signal to the FDD
    D1  Unit Select 1       US1     This bit is used to indicate the status of the Unit Select 1 signal to the FDD
    D0  Unit Select 0       US0     This bit is used to indicate the status of the Unit Select 0 signal to the FDD
    */

   //TODO: posible WP (si protegemos para escritura desde menu) y FT (en que casos?)

   

   return (PD765_STATUS_REGISTER_THREE_RD_MASK) | (pd765_signal_ts0.v * PD765_STATUS_REGISTER_THREE_T0_MASK) 
        | (pd765_input_parameter_hd<<2) | (pd765_input_parameter_us1<<1) | pd765_input_parameter_us0 ;

    //Two side??
//        | PD765_STATUS_REGISTER_THREE_TS_MASK;
}

//
// Gestion de escrituras de puerto
//

void pd765_handle_command_specify(void)
{
    //TODO: de momento no hacer nada
}

void pd765_read_parameters_specify(z80_byte value)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Receiving command parameters for SPECIFY");
    if (pd765_input_parameters_index==1) {
        pd765_input_parameter_srt=(value>>4) & 0x0F;
        pd765_input_parameter_hut=value & 0x0F;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: SRT=%XH HUT=%XH",pd765_input_parameter_srt,pd765_input_parameter_hut);
        pd765_input_parameters_index++;
    }
    else if (pd765_input_parameters_index==2) {
        pd765_input_parameter_hlt=(value>>4) & 0x0F;
        pd765_input_parameter_nd=value & 0x0F;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: HLT=%XH ND=%XH",pd765_input_parameter_hlt,pd765_input_parameter_nd);

        //Fin de comando
        pd765_input_parameters_index=0;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: End command parameters for SPECIFY");

        pd765_handle_command_specify();
    }       
}

void pd765_handle_command_sense_interrupt_status(void)
{
    //Cambiamos a fase de resultado
    pd765_phase=PD765_PHASE_RESULT;

    //E indicar que hay que leer datos
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

    //E indice a 0
    pd765_output_parameters_index=0;

    //Estos bits se resetean con un sense interrupt
    //if (pd765_sc_get(&signal_se)) {
    //    //TODO: dudoso hacer esto aqui
    //    pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_D0B_MASK - PD765_MAIN_STATUS_REGISTER_D1B_MASK - PD765_MAIN_STATUS_REGISTER_D2B_MASK - PD765_MAIN_STATUS_REGISTER_D3B_MASK);                
    //}    

    //Mientras dura, indicar que FDC esta busy
    //TODO: aunque creo que esto iria en la fase de ejecucion y no en la de resultado
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_CB_MASK;

    //Quitar flags de seek siempre que seek esté finalizado
    /*
    if (pd765_sc_get(&signal_se)) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Reset DB0 etc");
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_D0B_MASK - PD765_MAIN_STATUS_REGISTER_D1B_MASK - PD765_MAIN_STATUS_REGISTER_D2B_MASK - PD765_MAIN_STATUS_REGISTER_D3B_MASK);    

        pd765_sc_reset(&signal_se);
    }
    */

        //DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Reset DB0 etc");
        //pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_D0B_MASK - PD765_MAIN_STATUS_REGISTER_D1B_MASK - PD765_MAIN_STATUS_REGISTER_D2B_MASK - PD765_MAIN_STATUS_REGISTER_D3B_MASK);    


}

//R de sector a buscar en siguiente comando de read 
z80_byte pd765_read_command_searching_parameter_r;

//R de sector a buscar en siguiente comando de write
z80_byte pd765_write_command_searching_parameter_r;


void pd765_read_put_chrn_in_bus(void)
{
    //Segun la tabla pagina 9
    //TODO: diferentes valores de HD, MT. de momento solo considero cuando ambos sean 0

    z80_byte return_value;

    if (pd765_read_command_searching_parameter_r<pd765_input_parameter_eot) {

        return_value=pd765_input_parameter_c;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning C: %02XH",return_value);
        pd765_put_buffer(return_value);


        return_value=pd765_input_parameter_h;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning H: %02XH",return_value);
        pd765_put_buffer(return_value);


        return_value=pd765_input_parameter_r+1;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning R: %02XH",return_value);
        pd765_put_buffer(return_value);


        return_value=pd765_input_parameter_n;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning N: %02XH",return_value);
        pd765_put_buffer(return_value);    
    }      

    //else if (pd765_read_command_searching_parameter_r==pd765_input_parameter_eot) {
    //Cuando ambos son iguales. TODO: puede suceder que pd765_read_command_searching_parameter_r sea mayor que eot?
    else {

        return_value=pd765_input_parameter_c+1;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning C: %02XH",return_value);
        pd765_put_buffer(return_value);


        return_value=pd765_input_parameter_h;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning H: %02XH",return_value);
        pd765_put_buffer(return_value);


        return_value=1;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning R: %02XH",return_value);
        pd765_put_buffer(return_value);


        return_value=pd765_input_parameter_n;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning N: %02XH",return_value);
        pd765_put_buffer(return_value);    
    }

  
}

//Esto se puso para intentar cargar Alien\ Storm\ \(Erbe\).dsk
//quiza no es necesario?? o gestionar de otra manera
//Esto tiene que ver con lecturas de sectores de 8kb, que afecta supuestamente tambien a speedlock
//int anormal_termination=0;

void pd765_read_chrn_put_return_in_bus(z80_byte leido_st0,z80_byte leido_id_st1,z80_byte leido_id_st2)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST0: %02XH (%s)",leido_st0,(leido_st0 & 32 ? "SE" : ""));
    pd765_put_buffer(leido_st0);    

    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST1: %02XH",leido_id_st1);
    pd765_put_buffer(leido_id_st1);    

    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST2: %02XH",leido_id_st2);
    pd765_put_buffer(leido_id_st2);

    pd765_read_put_chrn_in_bus();
}





int pd765_common_dsk_not_inserted_readwrite(void)
{
    //Si DSK no insertado
    if (dskplusthree_emulation.v==0) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: DSK not inserted");

        //E indicar fase ejecucion ha finalizado
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);


        //Cambiamos a fase de resultado
        pd765_phase=PD765_PHASE_RESULT;

        pd765_set_interrupt_pending();

        //E indicar que hay que leer datos
        pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

        z80_byte return_value=pd765_get_st0();

        return_value |=PD765_STATUS_REGISTER_ZERO_NR_MASK;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST0: %02XH (%s)",return_value,(return_value & 32 ? "SE" : ""));
        pd765_put_buffer(return_value);


        return_value=PD765_STATUS_REGISTER_ONE_ND_MASK;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST1: %02XH",return_value);
        pd765_put_buffer(return_value);

        return_value=0;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST2: %02XH",return_value);

        pd765_put_buffer(return_value);

        //TODO: realmente importan los valores de chrn en este caso?

        pd765_read_put_chrn_in_bus();



        return 1;
    }

    return 0;

}

int pd765_common_if_track_unformatted(int pista,int cara)
{
    
    if (!dsk_is_track_formatted(pista,cara)) {
        /*
        The READ ID Command is used to give the present position of the recording head. 
        The FDC stores the values from the first ID Field it is able to read. 
        If no proper ID Address Mark is found on the diskette, before the INDEX HOLE is encountered 
        for the second time then the MA (Missing Address Mark) flag in Status Register 1 is set to a 1 (high), 
        and if no data is found then the ND (No Data) flag is also set in Status Register 1 to a 1 (high). 
        The command is then terminated with Bits 7 and 6 in Status Register O set to 0 and 1 respectively. 
        During this command there is no data transfer between FDC and the CPU except during the result phase.
        */
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Track %02XH Side %d unformatted!!!!!!!!!",pista,cara);
        
        
        pd765_set_interrupt_pending();    


        //E indicar fase ejecucion ha finalizado
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);


        //Cambiamos a fase de resultado
        pd765_phase=PD765_PHASE_RESULT;

        //E indicar que hay que leer datos
        pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

        //Mientras dura, indicar que FDC esta busy. ATF por ejemplo necesita esto
        pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_CB_MASK;

        z80_byte return_value=pd765_get_st0();

        return_value |=0x40;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST0: %02XH (%s)",return_value,(return_value & 32 ? "SE" : ""));
        pd765_put_buffer(return_value);


        return_value=PD765_STATUS_REGISTER_ONE_ND_MASK | PD765_STATUS_REGISTER_ONE_MA_MASK;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST1: %02XH",return_value);
        pd765_put_buffer(return_value);

        return_value=0;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST2: %02XH",return_value);

        pd765_put_buffer(return_value);

        //TODO: realmente importan los valores de chrn en este caso?
        pd765_read_put_chrn_in_bus();

        /*

        return_value=0;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning C: %02XH",return_value);
        pd765_put_buffer(return_value);


        return_value=0;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning H: %02XH",return_value);
        pd765_put_buffer(return_value);


        return_value=0;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning R: %02XH",return_value);
        pd765_put_buffer(return_value);


        return_value=0;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning N: %02XH",return_value);
        pd765_put_buffer(return_value);

        */

        sleep(1);

        return 1;
    }

    return 0;

}


void pd765_siguiente_sector(void)
{
    pd765_ultimo_sector_fisico_read++;

    //TODO de momento solo cara 0
    int total_sectores=dsk_get_total_sectors_track(pd765_pcn,0);


    if (total_sectores!=0) {
        pd765_ultimo_sector_fisico_read=pd765_ultimo_sector_fisico_read % total_sectores;
    }
    else {
        pd765_ultimo_sector_fisico_read=0;
    }  

    //Por si acaso, aunque esto no deberia pasar
    if (pd765_ultimo_sector_fisico_read<0) pd765_ultimo_sector_fisico_read=0;  
}

void pd765_handle_command_read_id(void)
{

   //Inicializar buffer retorno
   pd765_reset_buffer();

    //Si DSK no insertado
    if (pd765_common_dsk_not_inserted_readwrite()) {
        return;
    }

    //Si pista no formateada
    //TODO: de momento solo cara 0
    if (pd765_common_if_track_unformatted(pd765_pcn,0)) {
        return;
    }


    pd765_set_interrupt_pending();    

    //Cambiamos a fase de resultado
    pd765_phase=PD765_PHASE_RESULT;

    //E indicar que hay que leer datos
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

    //E indice a 0
    pd765_output_parameters_index=0;

    //Mientras dura, indicar que FDC esta busy
    //TODO: aunque creo que esto iria en la fase de ejecucion y no en la de resultado
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_CB_MASK;



    //Metemos resultado de leer en buffer de salida

  
    /*
    
    ST0
    ST1
    ST2
    C
    H
    R
    N

    READ ID
    The READ ID Command is used to give the present position of the recording head. 
    The FDC stores the values from the first ID Field it is able to read. 
    If no proper ID Address Mark is found on the diskette, 
    before the INDEX HOLE is encountered for the second time then the MA (Missing Address Mark) flag in Status Register 1 is set to a 1 (high), 
    and if no data is found then the ND (No Data) flag is also set in Status Register 1 to a 1 (high). 
    The command is then terminated with Bits 7 and 6 in Status Register O set to 0 and 1 respectively. 
    During this command there is no data transfer between FDC and the CPU except during the result phase.    

    */



   //Devolver CHRN siguiente
   z80_byte leido_id_c,leido_id_h,leido_id_r,leido_id_n;

  
    //Devolvemos el siguiente lector al anterior leido por read id
    pd765_siguiente_sector();


    int sector=pd765_ultimo_sector_fisico_read;


    
    //TODO de momento solo cara 0
    //TODO: retornar error si no hay sectores en esta pista
    DBG_PRINT_PD765 VERBOSE_DEBUG,"Obtener ID de Read id para sector %d de pista %02XH",sector,pd765_pcn);
   dsk_get_chrn(pd765_pcn,0,sector,&leido_id_c,&leido_id_h,&leido_id_r,&leido_id_n);

    //Guardarlo para debug
    pd765_debug_last_sector_id_c_read=leido_id_c;
    pd765_debug_last_sector_id_h_read=leido_id_h;
    pd765_debug_last_sector_id_r_read=leido_id_r;
    pd765_debug_last_sector_id_n_read=leido_id_n;  

    DBG_PRINT_PD765 VERBOSE_DEBUG,"##read_id: last_r: %d",pd765_debug_last_sector_id_r_read);



    z80_byte leido_st0=pd765_get_st0();
    z80_byte leido_st1=pd765_get_st1();
    z80_byte leido_st2=pd765_get_st2();

    //TODO: no estoy seguro si tengo que usar alguno de los bits de st1 o st2 del sector, quiza
    //de ST1: b2 ND (No Data)
    //de ST2: b0 MD (Missing address Mark in Data field)
    //dsk_get_st12(pd765_pcn,0,sector,&leido_st1,&leido_st2); 

    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST0: %02XH (%s)",leido_st0,(leido_st0 & 32 ? "SE" : ""));
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST1: %02XH",leido_st1);
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST2: %02XH",leido_st2);  



    /*if (leido_st2 & PD765_STATUS_REGISTER_TWO_CM_MASK) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"Sector with deleted mark");
        //sleep(1);
    }
    else {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"Sector with address mark");
    } */   
    
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning C: %02XH",leido_id_c);
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning H: %02XH",leido_id_h);
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning R: %02XH",leido_id_r);
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning N: %02XH",leido_id_n);

    pd765_put_buffer(leido_st0);    
    pd765_put_buffer(leido_st1);
    pd765_put_buffer(leido_st2);
    
    pd765_put_buffer(leido_id_c);
    pd765_put_buffer(leido_id_h);
    pd765_put_buffer(leido_id_r);
    pd765_put_buffer(leido_id_n);

}



void pd765_handle_command_invalid(void)
{
    //Cambiamos a fase de resultado
    pd765_phase=PD765_PHASE_RESULT;

    pd765_set_interrupt_pending();

    //E indicar que hay que leer datos
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

    //E indice a 0
    pd765_output_parameters_index=0;


}

void pd765_handle_command_sense_drive_status(void)
{
    //Cambiamos a fase de resultado
    pd765_phase=PD765_PHASE_RESULT;

    //E indicar que hay que leer datos
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

    //E indice a 0
    pd765_output_parameters_index=0;

    pd765_set_interrupt_pending();
}

void pd765_read_parameters_sense_drive_status(z80_byte value)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Receiving command parameters for SENSE DRIVE STATUS");
    if (pd765_input_parameters_index==1) {
        pd765_input_parameter_hd=(value>>2) & 0x01;
        pd765_input_parameter_us1=(value>>1) & 0x01;
        pd765_input_parameter_us0=value  & 0x01;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: HD=%XH US1=%XH US0=%XH",pd765_input_parameter_hd,pd765_input_parameter_us1,pd765_input_parameter_us0);

        //Fin de comando
        pd765_input_parameters_index=0;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: End command parameters for SENSE DRIVE STATUS");

        pd765_handle_command_sense_drive_status();
    }       
}

void pd765_read_parameters_read_id(z80_byte value)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Receiving command parameters for READ ID");
    if (pd765_input_parameters_index==1) {
        pd765_input_parameter_hd=(value>>2) & 0x01;
        pd765_input_parameter_us1=(value>>1) & 0x01;
        pd765_input_parameter_us0=value  & 0x01;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: HD=%XH US1=%XH US0=%XH",pd765_input_parameter_hd,pd765_input_parameter_us1,pd765_input_parameter_us0);

        //Fin de comando
        pd765_input_parameters_index=0;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: End command parameters for READ ID");

        pd765_handle_command_read_id();
    }       
}

//Indicar si cuando se va a hacer seek o recalibrate, ya se esta en pista indicada, y por tanto no empezar de nuevo
void pd765_if_seek_already_end(void)
{
    if (pd765_input_parameter_ncn==pd765_pcn) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Already seeked where asked");

        pd765_sc_set(&signal_se);

        signal_se.function_triggered();

    }
}

void pd765_handle_command_recalibrate(void)
{
    /*
    RECALIBRATE
    The function of this command is to retract the read/write head within the FDD to the Track 0 position.
    The FDC clears the contents of the PCN counter, and checks the status of the Track 0 signal from the FDD. 
    As long as the Track O signal is low, the Direction signal remains O (low) and Step Pulses are issued.
    When the Track 0 signal goes high, the SE (SEEK END) flag in Status Register O is set to a 1 (high) and 
    the command is terminated. 
    
    If the Track O signal is still low after 77 Step Pulse have been issued, 
    the FDC sets the SE (SEEK END) and EC (EQUIPMENT CHECK) flags of Status Register 0 to both 1s (highs), 
    and terminates the command after bits 7 and 6 of Status Register 0 is set to 0 and 1 respectively.
    The ability to do overlap RECALIBRATE Commands to multiple FDDs and the loss of the READY signal, 
    as described in the SEEK Command, also applies to the RECALIBRATE Command.    
    */
   
    //esto no bien en +3 aunque bien en PCW. 
    //TODO: tengo la duda de si un recalibrate/seek sin disco, deberia retornar error o no
    /*if (pd765_common_dsk_not_inserted_readwrite()) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: DSK not inserted on recalibrate");
        return;
    }*/
   //Inicialmente decimos no senyal TS0
   pd765_signal_ts0.v=0;

   pd765_sc_initialize_running(&signal_se);
   pd765_seek_was_recalibrating.v=1;
   pd765_input_parameter_ncn=0;

    //E indicar fase ejecucion ha empezado
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_EXM_MASK;

    //Decir datos no libres
    //pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_RQM_MASK);

    //Indicar seek unidad 0. Seguro?
    //pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_D0B_MASK;

    //pd765_phase=PD765_PHASE_EXECUTION;

    //En fase de ejecucion se activa interrupt
    //TODO: no tengo claro que en un seek o recalibrate se haga tambien
    //pd765_set_interrupt_pending();

    //pd765_interrupt_pending=0;

    pd765_if_seek_already_end();
   
}



void pd765_read_parameters_recalibrate(z80_byte value)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Receiving command parameters for RECALIBRATE");
    if (pd765_input_parameters_index==1) {
        pd765_input_parameter_us1=(value>>1) & 0x01;
        pd765_input_parameter_us0=value  & 0x01;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: US1=%XH US0=%XH",pd765_input_parameter_us1,pd765_input_parameter_us0);

        //Fin de comando
        pd765_input_parameters_index=0;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: End command parameters for RECALIBRATE");

        pd765_handle_command_recalibrate();
    }       
}

void pd765_handle_command_seek(void)
{
    /*
    Parecido a recalibrate pero vamos al track indicado
    */

    //Si DSK no insertado
    //esto no bien en +3 aunque bien en PCW. 
    //TODO: tengo la duda de si un recalibrate/seek sin disco, deberia retornar error o no    
    /*if (pd765_common_dsk_not_inserted_readwrite()) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: DSK not inserted on seek");
        return;
    }*/

   pd765_sc_initialize_running(&signal_se);
   pd765_seek_was_recalibrating.v=0;

    //E indicar fase ejecucion ha empezado
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_EXM_MASK;

    //Decir datos no libres
    //pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_RQM_MASK);    

    //Indicar seek unidad 0
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_D0B_MASK;


    //pd765_phase=PD765_PHASE_EXECUTION;

    //En fase de ejecucion se activa interrupt
    //TODO: siempre hay que activarlo cuando se esta en este estado?
    //TODO2: por que no estoy haciendo pd765_phase=PD765_PHASE_EXECUTION ?
    //TODO: no tengo claro que en un seek o recalibrate se haga tambien
    //pd765_set_interrupt_pending();    

    //pd765_interrupt_pending=0;

    pd765_if_seek_already_end();

   
}

//Indica que el comando read data se debe detener (no leer sectores siguientes) despues de este
int pd765_read_command_must_stop_anormal_termination=0;
//Lo mismo pero para write
int pd765_write_command_must_stop_anormal_termination=0;

void pd765_handle_command_read_data_put_sector_data_in_bus(int sector_size, int iniciosector)
{
    int indice;

    for (indice=0;indice<sector_size;indice++) {
        //DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Inicio sector de C: %d R: %d : %XH",pd765_input_parameter_c,pd765_input_parameter_r,iniciosector);
    
        z80_byte return_value=plus3dsk_get_byte_disk(iniciosector+indice);
        pd765_put_buffer(return_value);

    }    
}


void old_pd765_read_chrn_put_return_in_bus(z80_byte leido_st0,z80_byte leido_id_st1,z80_byte leido_id_st2)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST0: %02XH (%s)",leido_st0,(leido_st0 & 32 ? "SE" : ""));
    pd765_put_buffer(leido_st0);    

    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST1: %02XH",leido_id_st1);
    pd765_put_buffer(leido_id_st1);    

    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST2: %02XH",leido_id_st2);
    pd765_put_buffer(leido_id_st2);


    z80_byte return_value=pd765_input_parameter_c;
    //return_value=leido_id_c;
    //if (pd765_input_parameter_r==pd765_input_parameter_eot) return_value++;   
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning C: %02XH",return_value);
    pd765_put_buffer(return_value);


    return_value=pd765_input_parameter_h;
    //return_value=leido_id_h;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning H: %02XH",return_value);
    pd765_put_buffer(return_value);


    return_value=pd765_input_parameter_r;
    //return_value=leido_id_r;
    //return_value++;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning R: %02XH",return_value);
    pd765_put_buffer(return_value);


    return_value=pd765_input_parameter_n;
    //return_value=leido_id_n;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning N: %02XH",return_value);
    pd765_put_buffer(return_value);      
}
void pd765_handle_command_read_data_read_chrn_etc(int sector_fisico,int put_values_in_bus)
{



    //Leer chrn para debug
    z80_byte leido_id_c,leido_id_h,leido_id_r,leido_id_n;

    //TODO: de momento solo cara 0
    dsk_get_chrn(pd765_pcn,0,sector_fisico,&leido_id_c,&leido_id_h,&leido_id_r,&leido_id_n);

    
    pd765_debug_last_sector_id_c_read=leido_id_c;
    pd765_debug_last_sector_id_h_read=leido_id_h;
    pd765_debug_last_sector_id_r_read=leido_id_r;
    pd765_debug_last_sector_id_n_read=leido_id_n;


    z80_byte leido_st0=pd765_get_st0();
    z80_byte leido_id_st1 ,leido_id_st2;
    //TODO: de momento solo cara 0
    dsk_get_st12(pd765_pcn,0,sector_fisico,&leido_id_st1,&leido_id_st2);    

    /*if (anormal_termination) {
        leido_st0 |= PD765_STATUS_REGISTER_ZERO_AT;
        pd765_read_command_must_stop_anormal_termination=1;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"Anormal termination por anormal_termination");
        sleep(2);

    }*/



    //TODO: otros bits de st1 y st2 en este caso y en read id, por ejemplo missing address mark(d0) en st1

    
    if (leido_id_c!=pd765_input_parameter_c) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"#####Cylinder read from disk (%02XH) is not what asked (%02XH)",leido_id_c,pd765_input_parameter_c);
        //Wrong cylinder
        leido_id_st2 |= PD765_STATUS_REGISTER_TWO_WC_MASK;

        if (leido_id_c==0xFF) {
            //Bad cylinder
            leido_id_st2 |= PD765_STATUS_REGISTER_TWO_BC_MASK;
        }
    }



    //sleep(5);

    //Detectamos que el sector tiene marca de borrado con: leido_id_st2 & PD765_STATUS_REGISTER_TWO_CM_MASK

    if (leido_id_st2 & PD765_STATUS_REGISTER_TWO_CM_MASK) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"Sector with deleted mark");
    }
    else {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"Sector with address mark");
    }


    if (pd765_command_received==PD765_COMMAND_READ_DELETED_DATA) {

        ///Wec Le Mans (Erbe).dsk le mans espera 40h, 80h y 00h en st0, st1 y st2
        /*
        Otros juegos que funcionan al retornar bien estos valores:
        Carrier command
        Comando Quatro
        Gremlins 2
        Ice Breaker
        Mortadelo y Filemon 2
        Narco Police
        */


        /*
        Read deleted data:
        This command is the same as the Read Data Command except that when the FDC detects a Data Address
        Mark at the beginning of a Data Field (and SK = 0 (low), it will read all the data in the sector and set the
        CM flag in Status Register 2 to a 1 (high), and then terminate the command. If SK = 1, then the FDC skips
        the sector with the Data Address Mark and reads the next sector.

        */

        //TODO: como afecta bit MD??
        //bubble bobble y black lamp 

        //Leido un sector normal, sin marca de borrado
        if ((leido_id_st2 & PD765_STATUS_REGISTER_TWO_CM_MASK)==0) {
            if (pd765_input_parameter_sk) {
                //el skip ya se ha gestionado desde al llamar a dsk_get_sector 
                    DBG_PRINT_PD765 VERBOSE_DEBUG,"Sector not deleted and SK=1");
                    sleep(5);
            }
            else {
                //SK=0
                //leer tal cual

                leido_st0 |=PD765_STATUS_REGISTER_ZERO_AT; //Abnormal termination of command (NT)

                //poner el bit de CM
                leido_id_st2 |= PD765_STATUS_REGISTER_TWO_CM_MASK;

                pd765_read_command_must_stop_anormal_termination=1;

                DBG_PRINT_PD765 VERBOSE_DEBUG,"Anormal termination porque read deleted, sector normal y sk=0");
                //sleep(2);


                //Paris Dakar carga algo mejor si retorno 40h, 80h, 00
                //leido_st0=0x40; //Abnormal termination of command (NT)
                //leido_id_st1=PD765_STATUS_REGISTER_ONE_EN_MASK; 
         
            }
        }

        else {
            //Leido un sector con marca de borrado
            if (pd765_input_parameter_sk) {
                    DBG_PRINT_PD765 VERBOSE_DEBUG,"Sector deleted and SK=1");
            }
            else {
                leido_st0 |=PD765_STATUS_REGISTER_ZERO_AT; //Abnormal termination of command (NT)

                //TODO: cargas con speedlock no requieren que no se detenga la carga de multiples sectores, ejemplo Pang.dsk
                //Creo que esto deberia estar activado para todos discos pero para speed lock no...
                //pd765_read_command_must_stop_anormal_termination=1;
                //DBG_PRINT_PD765 VERBOSE_DEBUG,"Anormal termination porque read deleted, sector borrado y sk=0");
                //sleep(2);


                //End of Cylinder. When the FDC tries to access a Sector beyond the final Sector of a Cylinder, this flag is set
                //TODO: a saber por qué hay que activar este flag, yo solo se que Wec Le Mans (Erbe).dsk espera exactamente
                //este valor. Si no, no carga
                leido_id_st1=PD765_STATUS_REGISTER_ONE_EN_MASK; 

                //quitar el bit de CM
                leido_id_st2 &= (255-PD765_STATUS_REGISTER_TWO_CM_MASK);     
            }
        }


        //TODO: quitar el bit de CM de manera general, tiene sentido??
        //leido_id_st2 &= (255-PD765_STATUS_REGISTER_TWO_CM_MASK);     


    }
    

    
    if (pd765_command_received==PD765_COMMAND_READ_DATA) {
        /*
        read data
        If the FDC reads a Deleted Data Address Mark off the diskette, and the SK bit (bit D5 in the first Command
        Word) is not set (SK = 0), then the FDC sets the CM (Control Mark) flag in Status Register 2 to a 1 (high),
        and terminates the Read Data Command, after reading all the data in the Sector. If SK = 1, the FDC skips
        the sector with the Deleted Data Address Mark and reads the next sector. The CRC bits in the deleted data
        field are not checked when SK = 1.
        */

       //TODO: como afecta bit MD??

        //leido un sector borrado
        if (leido_id_st2 & PD765_STATUS_REGISTER_TWO_CM_MASK) {
            if (pd765_input_parameter_sk) {
                    //el skip ya se ha gestionado desde al llamar a dsk_get_sector 
                    DBG_PRINT_PD765 VERBOSE_DEBUG,"next sector when sector deleted and sk=1");
                    sleep(5);
            }
            else {
                    
                    /*
                    If the FDC reads a Deleted Data Address Mark off the diskette, and the SK bit (bit D5 in the first Command
                    Word) is not set (SK = 0), then the FDC sets the CM (Control Mark) flag in Status Register 2 to a 1 (high),
                    and terminates the Read Data Command, after reading all the data in the Sector
                    */
                    //Devolver tal cual st1 y st2 que vengan del sector, con el error apropiado

                    leido_st0 |=PD765_STATUS_REGISTER_ZERO_AT; //Abnormal termination of command (NT)

                    pd765_read_command_must_stop_anormal_termination=1;

                    DBG_PRINT_PD765 VERBOSE_DEBUG,"Anormal termination por read data, sector borrado y sk=0");
                    //sleep(2);



                    //DBG_PRINT_PD765 VERBOSE_DEBUG,"TODO. Sector deleted and SK=0");
                    //sleep(5);                          
            }
        }
        else {
            //leido un sector normal
            if (pd765_input_parameter_sk) {
                    DBG_PRINT_PD765 VERBOSE_DEBUG,"next sector when sector not deleted and sk=1");
            }
            else {
                    //leer tal cual
                    //DBG_PRINT_PD765 VERBOSE_DEBUG,"TODO. Sector not deleted and SK=0");
                    //sleep(5);                          
            }            
        }

    }        

    // Comprobar CRC
    /*
    After reading the ID and Data Fields in each sector, the FDC checks the CRC bytes.
    If a read error is detected (incorrect CRC in ID field), the FDC sets the DE (Data Error)
    flag in Status Register 1 to a 1 (high), and if a CRC error occurs in the Data Field the
    FDC also sets the DD (Data Error in Data Field) flag in Status Register 2 to a 1 (high),
    and terminates the Read Data Command. (Status Register 0 also has bits 7 and o set to 0 and 1 respectively.)
    */

    if ((leido_id_st1 & PD765_STATUS_REGISTER_ONE_DE_MASK) || (leido_id_st2 & PD765_STATUS_REGISTER_TWO_DD_MASK)) {
        leido_st0 |=PD765_STATUS_REGISTER_ZERO_AT; //Abnormal termination of command (NT)
        pd765_read_command_must_stop_anormal_termination=1;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"Abnormal termination por CRC error");
        //sleep(1);
    }

    if (put_values_in_bus) {
        pd765_read_chrn_put_return_in_bus(leido_st0,leido_id_st1,leido_id_st2);
    }
    
 
}

int pd765_total_sectors_read_track=0;

int pd765_total_sectors_write_track=0;

int pd765_last_sector_size_read_data=0;

int pd765_last_sector_size_write_data=0;


//Los dos posibles estados al leer datos
//Devolviendo valores sector
#define PD765_READ_COMMAND_STATE_READING_DATA 1
//Devolviendo ST0, ST1, ST2..
#define PD765_READ_COMMAND_STATE_ENDING_READING_DATA 2
int pd765_read_command_state=PD765_READ_COMMAND_STATE_READING_DATA;

//Los dos posibles estados al escribir datos
//Leyendo valores sector
#define PD765_WRITE_COMMAND_STATE_WRITING_DATA 1
//Devolviendo ST0, ST1, ST2..
#define PD765_WRITE_COMMAND_STATE_ENDING_WRITING_DATA 2
int pd765_write_command_state=PD765_WRITE_COMMAND_STATE_WRITING_DATA;



void pd765_handle_command_read_data(void)
{
/*
Read data:

If the FDC reads a Deleted Data Address Mark off the diskette, and the SK bit (bit D5 in the first Command
Word) is not set (SK = 0), then the FDC sets the CM (Control Mark) flag in Status Register 2 to a 1 (high),
and terminates the Read Data Command, after reading all the data in the Sector. If SK = 1, the FDC skips
the sector with the Deleted Data Address Mark and reads the next sector. The CRC bits in the deleted data
field are not checked when SK = 1.

*/

    pd765_read_command_must_stop_anormal_termination=0;

    //Inicializar buffer retorno
    pd765_reset_buffer();    


    //de momento esto a 0 por si el comando no lee (por dsk no insertado, seek beyond limit, etc)
    pd765_last_sector_size_read_data=0;

    if (pd765_common_dsk_not_inserted_readwrite()) {

        pd765_read_command_state=PD765_READ_COMMAND_STATE_ENDING_READING_DATA; 

        pd765_read_command_must_stop_anormal_termination=1;

        DBG_PRINT_PD765 VERBOSE_DEBUG,"Anormal termination dsk no insertado");


        return;
    }

    //Si pista no formateada
    //TODO: de momento solo cara 0
    if (pd765_common_if_track_unformatted(pd765_pcn,0)) {

        pd765_read_command_state=PD765_READ_COMMAND_STATE_ENDING_READING_DATA; 

        pd765_read_command_must_stop_anormal_termination=1;

        DBG_PRINT_PD765 VERBOSE_DEBUG,"Anormal termination porque pista no formateada");


        return;
    }    


    //En fase de ejecucion se activa
    pd765_set_interrupt_pending();    

    //Cambiamos a fase de resultado
    //TODO: realmente la fase seria ejecucion, arreglar esto
    pd765_phase=PD765_PHASE_RESULT;

    pd765_read_command_state=PD765_READ_COMMAND_STATE_READING_DATA;

    //E indicar que hay que leer datos
    //pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

    //E indice a 0
    pd765_output_parameters_index=0;

    //E indicar fase ejecucion ha empezado
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_EXM_MASK;    

    //Mientras dura, indicar que FDC esta busy
    //TODO: aunque creo que esto iria en la fase de ejecucion y no en la de resultado
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_CB_MASK;    


    //E indicar que hay que leer datos
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

    //Metemos resultado de leer en buffer de salida

    /*
    ST0
    ST1
    ST2
    C
    H
    R
    N

    */


   //TODO: esto solo es asi cuando N es 0
   int sector_size=pd765_input_parameter_dtl;



   sector_size=dsk_get_sector_size_track(pd765_pcn,0); //TODO: de momento una cara solamente; 

 
    z80_byte sector_fisico;


    //primero intentar obtener sector siguiente dentro de la pista
    int iniciosector;

    int search_deleted=0;

    if (pd765_command_received==PD765_COMMAND_READ_DELETED_DATA) search_deleted=1;

    if (search_deleted) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"search deleted");
        //sleep(5);
    }


    if (pd765_command_received==PD765_COMMAND_READ_TRACK) {
        //Obtener el siguiente sector sin comparar R
        iniciosector=dsk_get_sector(pd765_pcn,pd765_read_command_searching_parameter_r,&sector_fisico,pd765_ultimo_sector_fisico_read,search_deleted,pd765_input_parameter_sk,0);
    }

    else {


        DBG_PRINT_PD765 VERBOSE_DEBUG,"Trying to seek next sector after physical %d on track %d with id %02XH",pd765_ultimo_sector_fisico_read,pd765_pcn,pd765_read_command_searching_parameter_r);
        iniciosector=dsk_get_sector(pd765_pcn,pd765_read_command_searching_parameter_r,&sector_fisico,pd765_ultimo_sector_fisico_read,search_deleted,pd765_input_parameter_sk,1);

        
        if (iniciosector<0) {
            //no hay siguiente, volver a girar la pista
            DBG_PRINT_PD765 VERBOSE_DEBUG,"Next sector with asked ID not found. Starting from the beginning of track");
            iniciosector=dsk_get_sector(pd765_pcn,pd765_read_command_searching_parameter_r,&sector_fisico,-1,search_deleted,pd765_input_parameter_sk,1);
        }

    }

    //gestionar error si sector no encontrado
    //Megaphoenix esta dando este error: 
    //NOT Found sector ID 02H on track 4
    //Rainbow islands tambien, intenta leer de pista 39, que no esta formateada
    //Tambien abadia del crimen
    if (iniciosector<0) {
        /*
        If the FDC detects the Index Hole twice without finding the right sector, (indicated in "R"), 
        then the FDC sets the ND (No Data) flag in Status Register 1 to a 1 (high), and terminates the Read Data Command.
        (Status Register 0 also has bits 7 and 6 set to 0 and 1 respectively.)
        */

       //TODO: valores de retorno de CRHN son correctos? Que se devuelve en este caso?
       //Creo que no aplicaria la tabla de la pagina 9, porque siempre habla de "ultimo sector transferido", y si 
       //por ejemplo no hemos ni transferido un sector, que habria que poner entonces?

        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: sector not found");

        //E indicar fase ejecucion ha finalizado
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);


        //Cambiamos a fase de resultado
        pd765_phase=PD765_PHASE_RESULT;

        pd765_set_interrupt_pending();

        //E indicar que hay que leer datos
        pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;
           

        z80_byte return_value=pd765_get_st0();

        //abnormal termination
        return_value |=PD765_STATUS_REGISTER_ZERO_AT;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST0: %02XH (%s)",return_value,(return_value & 32 ? "SE" : ""));
        pd765_put_buffer(return_value);


        return_value=PD765_STATUS_REGISTER_ONE_ND_MASK|PD765_STATUS_REGISTER_ONE_MA_MASK;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST1: %02XH",return_value);
        pd765_put_buffer(return_value);

        return_value=PD765_STATUS_REGISTER_TWO_MD_MASK;        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST2: %02XH",return_value);
        pd765_put_buffer(return_value);     


        pd765_read_put_chrn_in_bus();

        /*

        return_value=pd765_input_parameter_c;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning C: %02XH",return_value);
        pd765_put_buffer(return_value);


        return_value=pd765_input_parameter_h;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning H: %02XH",return_value);
        pd765_put_buffer(return_value);


        return_value=pd765_input_parameter_r;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning R: %02XH",return_value);
        pd765_put_buffer(return_value);


        return_value=pd765_input_parameter_n;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning N: %02XH",return_value);
        pd765_put_buffer(return_value);       
        */  



        pd765_read_command_state=PD765_READ_COMMAND_STATE_ENDING_READING_DATA;  

        return;

    }

    pd765_total_sectors_read_track++;

    //Indicar ultimo sector leido para debug
    pd765_debug_last_sector_read=sector_fisico;

    //Ultimo sector leido 
    pd765_ultimo_sector_fisico_read=sector_fisico;


    //anormal_termination=0;

    //Tamanyo real para caso discos extendidos
    if (dsk_file_type_extended) {
        //DBG_PRINT_PD765 VERBOSE_DEBUG,"sector size before: %d",sector_size);
        //Tamanyo que dice el sector realmente
        int real_sector_size=dsk_get_real_sector_size_extended(pd765_pcn,0,sector_fisico); //TODO de momento solo cara 0

        //sector_size es el tamaño que decia del sector en la info de pista

        //TODO: esto tambien pasa cuando es mayor?
        //cuando es mayor lo que sucede es que es un sector escrito varias veces con diferentes datos,
        //en el disco real esta escrito una vez pero con datos "debiles" lo cual aporta datos cambiantes cada vez que se lea,
        //de ahi que haya que simularlo escogiendo una de las copias ¿al azar?
        if (real_sector_size<sector_size) {
            DBG_PRINT_PD765 VERBOSE_DEBUG,"Reading less data than the track size says. Setting abnormal termination flag");
            //anormal_termination=1; //quiza mantener para el siguiente sense interrupt?
        }

        sector_size=real_sector_size;
        //DBG_PRINT_PD765 VERBOSE_DEBUG,"sector size after: %d",sector_size);

        //se van a leer menos datos
    }

    

    pd765_last_sector_size_read_data=sector_size;

    DBG_PRINT_PD765 VERBOSE_DEBUG,"REAL sector size: %d",pd765_last_sector_size_read_data);


    pd765_handle_command_read_data_put_sector_data_in_bus(sector_size, iniciosector);


    //Evaluar condiciones que hacen abortar el comando
    pd765_handle_command_read_data_read_chrn_etc(pd765_ultimo_sector_fisico_read,0);

   
}


void pd765_write_put_chrn_in_bus(void)
{
  
    z80_byte return_value;

    return_value=pd765_input_parameter_c+1;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning C: %02XH",return_value);
    pd765_put_buffer(return_value);


    return_value=pd765_input_parameter_h;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning H: %02XH",return_value);
    pd765_put_buffer(return_value);


    return_value=1;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning R: %02XH",return_value);
    pd765_put_buffer(return_value);


    return_value=pd765_input_parameter_n;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning N: %02XH",return_value);
    pd765_put_buffer(return_value);    
    

  
}


void pd765_read_parameters_seek(z80_byte value)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Receiving command parameters for SEEK");

    if (pd765_input_parameters_index==1) {
        pd765_input_parameter_hd=(value>>2) & 0x01;
        pd765_input_parameter_us1=(value>>1) & 0x01;
        pd765_input_parameter_us0=value  & 0x01;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: HD=%XH US1=%XH US0=%XH",pd765_input_parameter_hd,pd765_input_parameter_us1,pd765_input_parameter_us0);

        pd765_input_parameters_index++;
    }

    else if (pd765_input_parameters_index==2) {
        pd765_input_parameter_ncn=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: NCN=%XH",pd765_input_parameter_ncn);

        //Fin de comando
        pd765_input_parameters_index=0;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: End command parameters for SEEK");

        pd765_handle_command_seek();
    }       
}



const char *pd765_last_command_name_read_data="READ_DATA";
const char *pd765_last_command_name_read_deleted_data="READ_DELETED_DATA";
const char *pd765_last_command_name_write_data="WRITE_DATA";
const char *pd765_last_command_name_read_track="READ_TRACK";
const char *pd765_last_command_name_format_track="FORMAT_TRACK";
const char *pd765_last_command_name_specify="SPECIFY";
const char *pd765_last_command_name_sense_drive_status="SENSE_DRIVE_STATUS";
const char *pd765_last_command_name_recalibrate="RECALIBRATE";
const char *pd765_last_command_name_sense_interrupt_status="SENSE_INTERRUPT_STATUS";
const char *pd765_last_command_name_seek="SEEK";
const char *pd765_last_command_name_read_id="READ_ID";
const char *pd765_last_command_name_unknown="UNKNOWN";

const char *pd765_last_command_name(void)
{
    switch (pd765_command_received) {

        case PD765_COMMAND_SPECIFY:
            return pd765_last_command_name_specify;
        break;

        case PD765_COMMAND_SENSE_DRIVE_STATUS:
            return pd765_last_command_name_sense_drive_status;
        break;
        case PD765_COMMAND_RECALIBRATE:
            return pd765_last_command_name_recalibrate;
        break;

        case PD765_COMMAND_SENSE_INTERRUPT_STATUS:
            return pd765_last_command_name_sense_interrupt_status;
        break;

        case PD765_COMMAND_SEEK:
            return pd765_last_command_name_seek;
        break;

        case PD765_COMMAND_READ_ID:
            return pd765_last_command_name_read_id;
        break;

        case PD765_COMMAND_READ_DELETED_DATA:
            return pd765_last_command_name_read_deleted_data;
        break;

        case PD765_COMMAND_READ_TRACK:
            return pd765_last_command_name_read_track;
        break;

        case PD765_COMMAND_READ_DATA:
            return pd765_last_command_name_read_data;
        break;

        case PD765_COMMAND_WRITE_DATA:
            return pd765_last_command_name_write_data;
        break;        

        case PD765_COMMAND_FORMAT_TRACK:
            return pd765_last_command_name_format_track;
        break;

        default:
            return pd765_last_command_name_unknown;
        break;

    }

}

void pd765_read_parameters_read_data(z80_byte value)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Receiving command parameters for %s",
    pd765_last_command_name()
    );

    if (pd765_input_parameters_index==1) {
        pd765_input_parameter_hd=(value>>2) & 0x01;
        pd765_input_parameter_us1=(value>>1) & 0x01;
        pd765_input_parameter_us0=value  & 0x01;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: HD=%XH US1=%XH US0=%XH",pd765_input_parameter_hd,pd765_input_parameter_us1,pd765_input_parameter_us0);

        pd765_input_parameters_index++;
    }

    else if (pd765_input_parameters_index==2) {
        pd765_input_parameter_c=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: C=%XH",pd765_input_parameter_c);

        pd765_input_parameters_index++;
    }  

    else if (pd765_input_parameters_index==3) {
        pd765_input_parameter_h=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: H=%XH",pd765_input_parameter_h);

        pd765_input_parameters_index++;
    }

    else if (pd765_input_parameters_index==4) {
        pd765_input_parameter_r=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: R=%XH",pd765_input_parameter_r);

        pd765_input_parameters_index++;
    }

    else if (pd765_input_parameters_index==5) {
        pd765_input_parameter_n=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: N=%XH",pd765_input_parameter_n);

        if (pd765_input_parameter_n==0) {
            //TODO
            DBG_PRINT_PD765 VERBOSE_DEBUG,"N=0 not handled yet!!");
            sleep(5);
        }

        pd765_input_parameters_index++;
    }   

    else if (pd765_input_parameters_index==6) {
        pd765_input_parameter_eot=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: EOT=%XH",pd765_input_parameter_eot);

        pd765_input_parameters_index++;
    } 

    else if (pd765_input_parameters_index==7) {
        pd765_input_parameter_gpl=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: GPL=%XH",pd765_input_parameter_gpl);

        pd765_input_parameters_index++;
    } 

    else if (pd765_input_parameters_index==8) {
        pd765_input_parameter_dtl=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: DTL=%XH",pd765_input_parameter_dtl);


        //Fin de comando
        pd765_input_parameters_index=0;
        
        DBG_PRINT_PD765 VERBOSE_INFO,"PD765: End receiving command parameters for %s."
        " C=%02XH H=%02XH R=%02XH N=%02XH EOT=%02XH GPL=%02XH DTL=%02XH. Current track: %02XH",
        pd765_last_command_name(),
        pd765_input_parameter_c,pd765_input_parameter_h,pd765_input_parameter_r,pd765_input_parameter_n,
        pd765_input_parameter_eot,pd765_input_parameter_gpl,pd765_input_parameter_dtl,pd765_pcn
        
        );


        //Si read track, indicar que ultimo sector es el 0 tal cual
        if (pd765_command_received==PD765_COMMAND_READ_TRACK) {
            //Decir anterior sector leido de esa pista
            pd765_ultimo_sector_fisico_read=-1;

            //Y conteo total de sectores a 0
            pd765_total_sectors_read_track=0;
        }

        pd765_read_command_searching_parameter_r=pd765_input_parameter_r;

        pd765_handle_command_read_data();
    }       


}

int pd765_last_inicio_sector_read=0;

void pd765_handle_command_start_write_data(void)
{


    pd765_write_command_must_stop_anormal_termination=0;

    //Inicializar buffer retorno
    pd765_reset_buffer();    


    //de momento esto a 0 por si el comando no lee (por dsk no insertado, seek beyond limit, etc)
    pd765_last_sector_size_write_data=0;

    if (pd765_common_dsk_not_inserted_readwrite()) {

        pd765_write_command_state=PD765_WRITE_COMMAND_STATE_ENDING_WRITING_DATA; 

        pd765_write_command_must_stop_anormal_termination=1;

        DBG_PRINT_PD765 VERBOSE_DEBUG,"Anormal termination dsk no insertado");


        return;
    }

    //Si pista no formateada
    //TODO: de momento solo cara 0
    if (pd765_common_if_track_unformatted(pd765_pcn,0)) {

        pd765_write_command_state=PD765_WRITE_COMMAND_STATE_ENDING_WRITING_DATA; 

        pd765_write_command_must_stop_anormal_termination=1;

        DBG_PRINT_PD765 VERBOSE_DEBUG,"Anormal termination porque pista no formateada");


        return;
    }    

    //En fase de ejecucion se activa
    pd765_set_interrupt_pending();    

    //Cambiamos a fase de resultado
    //TODO: realmente la fase seria ejecucion, arreglar esto
    //pd765_phase=PD765_PHASE_RESULT;

    pd765_write_command_state=PD765_WRITE_COMMAND_STATE_WRITING_DATA;

    //E indicar que hay que leer datos
    //pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

    //E indice a 0
    pd765_output_parameters_index=0;

    //E indicar fase ejecucion ha empezado
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_EXM_MASK;    

    //Mientras dura, indicar que FDC esta busy
    //TODO: aunque creo que esto iria en la fase de ejecucion y no en la de resultado
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_CB_MASK;    


    //E indicar que hay que escribir datos
    pd765_main_status_register &=(255-PD765_MAIN_STATUS_REGISTER_DIO_MASK);

    //Metemos resultado de leer en buffer de salida

    /*
    ST0
    ST1
    ST2
    C
    H
    R
    N

    */


   //TODO: esto solo es asi cuando N es 0
   int sector_size=pd765_input_parameter_dtl;



   sector_size=dsk_get_sector_size_track(pd765_pcn,0); //TODO: de momento una cara solamente; 

 
    z80_byte sector_fisico;


    //primero intentar obtener sector siguiente dentro de la pista
    int iniciosector;

    int search_deleted=0;

    //TODO: write deleted data command
    //if (pd765_command_received==PD765_COMMAND_WRITE_DELETED_DATA) search_deleted=1;

    if (search_deleted) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"search deleted");
        //sleep(5);
    }


    //TODO: write track command
    if (0/*pd765_command_received==PD765_COMMAND_WRITE_TRACK*/) {
        //Obtener el siguiente sector sin comparar R
        iniciosector=dsk_get_sector(pd765_pcn,pd765_write_command_searching_parameter_r,&sector_fisico,pd765_ultimo_sector_fisico_write,search_deleted,pd765_input_parameter_sk,0);
    }

    else {


        DBG_PRINT_PD765 VERBOSE_DEBUG,"Trying to seek next sector after physical %d on track %d with id %02XH",pd765_ultimo_sector_fisico_write,pd765_pcn,pd765_write_command_searching_parameter_r);
        iniciosector=dsk_get_sector(pd765_pcn,pd765_write_command_searching_parameter_r,&sector_fisico,pd765_ultimo_sector_fisico_write,search_deleted,pd765_input_parameter_sk,1);

        
        if (iniciosector<0) {
            //no hay siguiente, volver a girar la pista
            DBG_PRINT_PD765 VERBOSE_DEBUG,"Next sector with asked ID not found. Starting from the beginning of track");
            iniciosector=dsk_get_sector(pd765_pcn,pd765_write_command_searching_parameter_r,&sector_fisico,-1,search_deleted,pd765_input_parameter_sk,1);
        }

    }

    //gestionar error si sector no encontrado
    //Megaphoenix esta dando este error: 
    //NOT Found sector ID 02H on track 4
    //Rainbow islands tambien, intenta leer de pista 39, que no esta formateada
    //Tambien abadia del crimen
    if (iniciosector<0) {
        /*
        If the FDC detects the Index Hole twice without finding the right sector, (indicated in "R"), 
        then the FDC sets the ND (No Data) flag in Status Register 1 to a 1 (high), and terminates the Read Data Command.
        (Status Register 0 also has bits 7 and 6 set to 0 and 1 respectively.)
        */

       //TODO: valores de retorno de CRHN son correctos? Que se devuelve en este caso?
       //Creo que no aplicaria la tabla de la pagina 9, porque siempre habla de "ultimo sector transferido", y si 
       //por ejemplo no hemos ni transferido un sector, que habria que poner entonces?

        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: sector not found");

        //E indicar fase ejecucion ha finalizado
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);


        //Cambiamos a fase de resultado
        pd765_phase=PD765_PHASE_RESULT;

        pd765_set_interrupt_pending();

        //E indicar que hay que leer datos
        pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;
           

        z80_byte return_value=pd765_get_st0();

        //abnormal termination
        return_value |=PD765_STATUS_REGISTER_ZERO_AT;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST0: %02XH (%s)",return_value,(return_value & 32 ? "SE" : ""));
        pd765_put_buffer(return_value);


        return_value=PD765_STATUS_REGISTER_ONE_ND_MASK|PD765_STATUS_REGISTER_ONE_MA_MASK;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST1: %02XH",return_value);
        pd765_put_buffer(return_value);

        return_value=PD765_STATUS_REGISTER_TWO_MD_MASK;        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST2: %02XH",return_value);
        pd765_put_buffer(return_value);     


        pd765_write_put_chrn_in_bus();


        pd765_write_command_state=PD765_WRITE_COMMAND_STATE_ENDING_WRITING_DATA;  

        return;

    }

    pd765_total_sectors_write_track++;

    //Indicar ultimo sector leido para debug
    pd765_debug_last_sector_write=sector_fisico;

    //Ultimo sector leido 
    pd765_ultimo_sector_fisico_write=sector_fisico;


    //anormal_termination=0;

    //Tamanyo real para caso discos extendidos
    if (dsk_file_type_extended) {
        //DBG_PRINT_PD765 VERBOSE_DEBUG,"sector size before: %d",sector_size);
        //Tamanyo que dice el sector realmente
        int real_sector_size=dsk_get_real_sector_size_extended(pd765_pcn,0,sector_fisico); //TODO de momento solo cara 0

        //sector_size es el tamaño que decia del sector en la info de pista

        //TODO: esto tambien pasa cuando es mayor?
        //cuando es mayor lo que sucede es que es un sector escrito varias veces con diferentes datos,
        //en el disco real esta escrito una vez pero con datos "debiles" lo cual aporta datos cambiantes cada vez que se lea,
        //de ahi que haya que simularlo escogiendo una de las copias ¿al azar?
        if (real_sector_size<sector_size) {
            DBG_PRINT_PD765 VERBOSE_DEBUG,"Reading less data than the track size says. Setting abnormal termination flag");
            //anormal_termination=1; //quiza mantener para el siguiente sense interrupt?
        }

        sector_size=real_sector_size;
        //DBG_PRINT_PD765 VERBOSE_DEBUG,"sector size after: %d",sector_size);

        //se van a leer menos datos
    }

    

    pd765_last_sector_size_write_data=sector_size;

    DBG_PRINT_PD765 VERBOSE_DEBUG,"REAL sector size to write: %d",pd765_last_sector_size_write_data);
    //sleep(2);


    //TODO
    //pd765_handle_command_write_data_put_sector_data_in_bus(sector_size, iniciosector);


    //Evaluar condiciones que hacen abortar el comando
    //TODO
    //pd765_handle_command_write_data_write_chrn_etc(pd765_ultimo_sector_fisico_write,0);

    pd765_last_inicio_sector_read=iniciosector;

   
}


void pd765_handle_command_write_data_put_sector_data_from_bus(int sector_size, int iniciosector)
{
    int indice;

    for (indice=0;indice<sector_size;indice++) {
        //DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Inicio sector de C: %d R: %d : %XH",pd765_input_parameter_c,pd765_input_parameter_r,iniciosector);
        z80_byte byte_sector=pd765_get_buffer();

        //DBG_PRINT_PD765 VERBOSE_DEBUG,"%c",(byte_sector>=32 && byte_sector<=126 ? byte_sector : '?'));
    
        plus3dsk_put_byte_disk(iniciosector+indice,byte_sector);
        

    }    
    //DBG_PRINT_PD765 VERBOSE_DEBUG,"");
}

int pd765_if_write_protected(void)
{
    //Si no tiene proteccion habilitada, volver tal cual
    if (dskplusthree_write_protection.v==0) return 0;

    //Y si tiene modo silent protection, volver tambien, pues hay que aparentar que se sigue escribiendo igual
    //luego desde plus3dsk_put_byte_disk se revisara que dskplusthree_write_protection.v esta activo y no escribira el dato realmente
    if (pd765_silent_write_protection.v) return 0;

    //Cambiamos a fase de resultado
    pd765_phase=PD765_PHASE_RESULT;

    pd765_set_interrupt_pending();

    //E indicar que hay que leer datos
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;     

    //E indicar fase ejecucion ha finalizado
    pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);                       

    //No esperamos mas parametros de input
    //Fin de comando
    pd765_input_parameters_index=0;

    // meter datos st0,st1, st2,chrn
    pd765_reset_buffer();

    z80_byte leido_st0=pd765_get_st0();
    z80_byte leido_st1=pd765_get_st1();
    z80_byte leido_st2=pd765_get_st2();

    leido_st0 |=0x40;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST0: %02XH (%s)",leido_st0,(leido_st0 & 32 ? "SE" : ""));
    pd765_put_buffer(leido_st0);

    
    leido_st1 |=PD765_STATUS_REGISTER_ONE_NW_MASK;

    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST1: %02XH",leido_st1);
    pd765_put_buffer(leido_st1);

    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST2: %02XH",leido_st2);
    pd765_put_buffer(leido_st2);

    z80_byte return_value;

    return_value=pd765_input_parameter_c;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning C: %02XH",return_value);
    pd765_put_buffer(return_value);


    return_value=pd765_input_parameter_h;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning H: %02XH",return_value);
    pd765_put_buffer(return_value);


    return_value=pd765_input_parameter_r+1;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning R: %02XH",return_value);
    pd765_put_buffer(return_value);


    return_value=pd765_input_parameter_n;
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning N: %02XH",return_value);
    pd765_put_buffer(return_value);                                

    return 1;

}



void pd765_read_parameters_write_data(z80_byte value)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Receiving command parameters for %s. Value=%02XH",
    pd765_last_command_name(),value
    );

    if (pd765_if_write_protected()) return;

    if (pd765_input_parameters_index==1) {
        pd765_input_parameter_hd=(value>>2) & 0x01;
        pd765_input_parameter_us1=(value>>1) & 0x01;
        pd765_input_parameter_us0=value  & 0x01;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: HD=%XH US1=%XH US0=%XH",pd765_input_parameter_hd,pd765_input_parameter_us1,pd765_input_parameter_us0);

        pd765_input_parameters_index++;
    }

    else if (pd765_input_parameters_index==2) {
        pd765_input_parameter_c=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: C=%XH",pd765_input_parameter_c);

        pd765_input_parameters_index++;
    }  

    else if (pd765_input_parameters_index==3) {
        pd765_input_parameter_h=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: H=%XH",pd765_input_parameter_h);

        pd765_input_parameters_index++;
    }

    else if (pd765_input_parameters_index==4) {
        pd765_input_parameter_r=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: R=%XH",pd765_input_parameter_r);

        pd765_input_parameters_index++;
    }

    else if (pd765_input_parameters_index==5) {
        pd765_input_parameter_n=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: N=%XH",pd765_input_parameter_n);

        if (pd765_input_parameter_n==0) {
            //TODO
            DBG_PRINT_PD765 VERBOSE_DEBUG,"N=0 not handled yet!!");
            sleep(5);
        }

        pd765_input_parameters_index++;
    }   

    else if (pd765_input_parameters_index==6) {
        pd765_input_parameter_eot=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: EOT=%XH",pd765_input_parameter_eot);

        pd765_input_parameters_index++;
    } 

    else if (pd765_input_parameters_index==7) {
        pd765_input_parameter_gpl=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: GPL=%XH",pd765_input_parameter_gpl);

        pd765_input_parameters_index++;
    } 

    else if (pd765_input_parameters_index==8) {
        pd765_input_parameter_dtl=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: DTL=%XH",pd765_input_parameter_dtl);


        //Fin de comando
        //pd765_input_parameters_index=0;
        pd765_input_parameters_index++;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: End command parameters for %s. Writing to CHRN=%02XH %02XH %02XH %02XH ",
        pd765_last_command_name(),pd765_input_parameter_c,pd765_input_parameter_h,pd765_input_parameter_r,pd765_input_parameter_n
        );
        //sleep(1);


        pd765_write_command_searching_parameter_r=pd765_input_parameter_r;

        pd765_handle_command_start_write_data();

    }    

    //Leyendo datos de sector a escribir
    else if (pd765_input_parameters_index>=9 && pd765_input_parameters_index<9+pd765_last_sector_size_write_data) {   
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Writing sector index %d",pd765_input_parameters_index-9);

        //notificar visual floppy
        menu_visual_floppy_buffer_add(pd765_pcn,pd765_ultimo_sector_fisico_write,pd765_input_parameters_index-9);

        //Estadisticas escritura
        pd765_write_stats_bytes_sec_acumulated++;        

        //meter dato en buffer
        pd765_put_buffer(value);

        pd765_input_parameters_index++;
    }

    //Si llega al final del disco
    //TODO: si el usuario sigue escribiendo.... no podra escribir mas alla de lo que dice el sector size del disco
    if (pd765_input_parameters_index>=9+pd765_last_sector_size_write_data) {
            DBG_PRINT_PD765 VERBOSE_DEBUG,"End of sector on write data");

                //- escribir sector en dsk
                int longitud=pd765_last_sector_size_write_data;

                pd765_handle_command_write_data_put_sector_data_from_bus(longitud,pd765_last_inicio_sector_read);




                //E indicar fase ejecucion ha finalizado
                pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);


                pd765_set_interrupt_pending();    

                //Cambiamos a fase de resultado
                pd765_phase=PD765_PHASE_RESULT;

                //E indicar que hay que leer datos
                pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

                pd765_read_command_state=PD765_WRITE_COMMAND_STATE_ENDING_WRITING_DATA;

                // meter datos st0,st1, st2,chrn
                pd765_reset_buffer();

                z80_byte leido_st0=pd765_get_st0();
                z80_byte leido_st1=pd765_get_st1();
                z80_byte leido_st2=pd765_get_st2();                

                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST0: %02XH (%s)",leido_st0,(leido_st0 & 32 ? "SE" : ""));
                pd765_put_buffer(leido_st0);    

                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST1: %02XH",leido_st1);
                pd765_put_buffer(leido_st1);    

                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST2: %02XH",leido_st2);
                pd765_put_buffer(leido_st2);


                z80_byte return_value;

                return_value=pd765_input_parameter_c;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning C: %02XH",return_value);
                pd765_put_buffer(return_value);


                return_value=pd765_input_parameter_h;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning H: %02XH",return_value);
                pd765_put_buffer(return_value);


                return_value=pd765_input_parameter_r+1;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning R: %02XH",return_value);
                pd765_put_buffer(return_value);


                return_value=pd765_input_parameter_n;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning N: %02XH",return_value);
                pd765_put_buffer(return_value);  

                              


        
    }


}



int pd765_formatting_total_sectores=9;

int pd765_formatting_currentsector=0;


void pd765_format_sector_track(int track,int sector,int sector_size,z80_byte fill_byte,
    z80_byte parametro_c,z80_byte parametro_h,z80_byte parametro_r,z80_byte parametro_n)
{
    //TODO escribir valores CHRN sector y otros necesarios... realmente solo seria CHRN, 
    //resto de info del dsk se tiene que conservar y tener creado al inicializar un dsk.
    //O no? por si partimos de un dsk con formato extraño y lo queremos inicializar entero? por tanto hay que escribir mas cosas aparte de CHRN
    //tal y como dice la especificacion, meter formato IBM noseque...

    //Escribir CHRN
    //TODO: solo una cara 
    dsk_put_chrn(track,0,sector,parametro_c,parametro_h,parametro_r,parametro_n);

    //Escribir valores ST1 y ST2
    //TODO: metemos esto siempre a 0, tecnicamente ya estaria bien,
    //aunque si se guarda algun disco "especial" tipo protegido o speedlock, se tiene que poder escribir tal cual
    //el st1 y st2 de la controladora, con sus errores si conviene
    //TODO: solo una cara
    //Nota: si por lo que fuera, los valores de st1 o st2 fueran corruptos, dando error de crc,
    //desde +3 basic no deja formatear, pues intenta leer primero si hay datos, antes de formatear,
    //y como lee esos crc error de los st1 y st2 corruptos, cree que el disco esta estropeado
    //Creo que la unica manera de corregir el disco seria lanzando comando de formateo directamente a la controladora de disco,
    //sin usar comando del +3 basic format "a:"
    //TODO: hacer un item de menu que haga el formateo de un disco
    dsk_put_st12(track,0,sector,0,0);


    //Rellenamos sector con byte

    //calcular inicio sector

    int iniciosector=dsk_get_physical_sector(track,sector);

    //gestionar error si sector no encontrado

    if (iniciosector<0) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"TODO gestion error");
        return;
    }   

    int indice;

    for (indice=0;indice<sector_size;indice++) {
        //DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Inicio sector de C: %d R: %d : %XH",pd765_input_parameter_c,pd765_input_parameter_r,iniciosector);

        plus3dsk_put_byte_disk(iniciosector+indice,fill_byte);

        //notificar visual floppy
        menu_visual_floppy_buffer_add(track,sector,indice);

        //Estadisticas escritura
        pd765_write_stats_bytes_sec_acumulated++;        


    }
    //DBG_PRINT_PD765 VERBOSE_DEBUG,"");
  
}


void pd765_read_parameters_format_track(z80_byte value)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Receiving command parameters for %s",
    pd765_last_command_name()
    );

    if (pd765_if_write_protected()) return;

    if (pd765_input_parameters_index==1) {
        pd765_input_parameter_hd=(value>>2) & 0x01;
        pd765_input_parameter_us1=(value>>1) & 0x01;
        pd765_input_parameter_us0=value  & 0x01;
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: HD=%XH US1=%XH US0=%XH",pd765_input_parameter_hd,pd765_input_parameter_us1,pd765_input_parameter_us0);

        pd765_input_parameters_index++;
    }

    else if (pd765_input_parameters_index==2) {
        pd765_input_parameter_n_format=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: N=%XH",pd765_input_parameter_n_format);

        pd765_input_parameters_index++;
    }  

    else if (pd765_input_parameters_index==3) {
        pd765_input_parameter_sc=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: SC=%XH",pd765_input_parameter_sc);

        pd765_input_parameters_index++;
    }

    else if (pd765_input_parameters_index==4) {
        pd765_input_parameter_gpl=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: GPL=%XH",pd765_input_parameter_gpl);

        pd765_input_parameters_index++;
    }

    else if (pd765_input_parameters_index==5) {
        pd765_input_parameter_d=value;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: D=%XH",pd765_input_parameter_d);

        pd765_input_parameters_index++;

        //En fase de ejecucion se activa 
        pd765_set_interrupt_pending();

        //TODO calcular bien este valore
        pd765_formatting_total_sectores=9;

        pd765_formatting_currentsector=0;


        //Y decir que ya no hay que devolver mas datos
        /*
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_DIO_MASK);

        //Decir que ya no esta busy
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_CB_MASK);            

        //Y pasamos a fase command
        pd765_phase=PD765_PHASE_COMMAND;

        //Fin de comando
        pd765_input_parameters_index=0;   
        */

        //sleep(3);
      

    //E indicar fase ejecucion para recibir valores CHRN de este sector a formatear
    pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_EXM_MASK;        
    

    }   
    
    else if (pd765_input_parameters_index>=6) {
        //Recepcion de cada chrn de cada sector
        int indice=pd765_input_parameters_index-6;
        int sector=indice/4;
        int chrn_id=indice%4;

        switch (chrn_id) {
            case 0:
                pd765_input_parameter_c=value;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: C=%XH",value);
            break;

            case 1:
                pd765_input_parameter_h=value;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: H=%XH",value);
            break;

            case 2:
                pd765_input_parameter_r=value;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: R=%XH",value);
            break;

            case 3:
                pd765_input_parameter_n=value;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: N=%XH",value);
            break;

        }

        //Hemos leido el valor de N
        if (chrn_id==3) {
            //hacer efectivo el formateo de ese sector
            DBG_PRINT_PD765 VERBOSE_DEBUG,"Formatting sector %d. Current track: %02XH",sector,pd765_pcn);

            int sector_size=dsk_get_sector_size_from_n_value(pd765_input_parameter_n_format);

            pd765_format_sector_track(pd765_pcn,sector,sector_size,pd765_input_parameter_d,
                pd765_input_parameter_c,pd765_input_parameter_h,pd765_input_parameter_r,pd765_input_parameter_n);
        }

        //si final
        if (sector==pd765_formatting_total_sectores-1 && chrn_id==3) {
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---Fin de formateo pista");

                //Cambiamos a fase de resultado
                pd765_phase=PD765_PHASE_RESULT;

                //E indicar que hay que leer datos
                pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;     

                //E indicar fase ejecucion ha finalizado
                pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);                       

                //No esperamos mas parametros de input
                //Fin de comando
                pd765_input_parameters_index=0;

                pd765_set_interrupt_pending();    

                // meter datos st0,st1, st2,chrn
                pd765_reset_buffer();

                z80_byte leido_st0=pd765_get_st0();
                z80_byte leido_st1=pd765_get_st1();
                z80_byte leido_st2=pd765_get_st2();

                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST0: %02XH (%s)",leido_st0,(leido_st0 & 32 ? "SE" : ""));
                pd765_put_buffer(leido_st0);

                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST1: %02XH",leido_st1);
                pd765_put_buffer(leido_st1);

                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST2: %02XH",leido_st2);
                pd765_put_buffer(leido_st2);

                z80_byte return_value;

                return_value=pd765_input_parameter_c;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning C: %02XH",return_value);
                pd765_put_buffer(return_value);


                return_value=pd765_input_parameter_h;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning H: %02XH",return_value);
                pd765_put_buffer(return_value);


                return_value=pd765_input_parameter_r+1;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning R: %02XH",return_value);
                pd765_put_buffer(return_value);


                return_value=pd765_input_parameter_n;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning N: %02XH",return_value);
                pd765_put_buffer(return_value);                                


            //sleep(5);
        }

        else {
            pd765_input_parameters_index++;
            //sleep(1);
        }
    } 
    





    //Si llega al final del disco
    //TODO: si el usuario sigue escribiendo.... no podra escribir mas alla de lo que dice el sector size del disco
    /*
    else if (pd765_input_parameters_index>=9+pd765_last_sector_size_write_data) {
            DBG_PRINT_PD765 VERBOSE_DEBUG,"End of sector on write data");

                //- escribir sector en dsk
                int longitud=pd765_last_sector_size_write_data;

                pd765_handle_command_write_data_put_sector_data_from_bus(longitud,pd765_last_inicio_sector_read);




                //E indicar fase ejecucion ha finalizado
                pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);


                pd765_set_interrupt_pending();    

                //Cambiamos a fase de resultado
                pd765_phase=PD765_PHASE_RESULT;

                //E indicar que hay que leer datos
                pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

                pd765_read_command_state=PD765_WRITE_COMMAND_STATE_ENDING_WRITING_DATA;

                // meter datos st0,st1, st2,chrn
                pd765_reset_buffer();

                z80_byte leido_st0=pd765_get_st0();
                z80_byte leido_st1=pd765_get_st1();
                z80_byte leido_st2=pd765_get_st2();                

                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST0: %02XH (%s)",leido_st0,(leido_st0 & 32 ? "SE" : ""));
                pd765_put_buffer(leido_st0);    

                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST1: %02XH",leido_st1);
                pd765_put_buffer(leido_st1);    

                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST2: %02XH",leido_st2);
                pd765_put_buffer(leido_st2);


                z80_byte return_value;

                return_value=pd765_input_parameter_c;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning C: %02XH",return_value);
                pd765_put_buffer(return_value);


                return_value=pd765_input_parameter_h;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning H: %02XH",return_value);
                pd765_put_buffer(return_value);


                return_value=pd765_input_parameter_r+1;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning R: %02XH",return_value);
                pd765_put_buffer(return_value);


                return_value=pd765_input_parameter_n;
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning N: %02XH",return_value);
                pd765_put_buffer(return_value);  

                              


        
    }
    */


}

void pd765_write_handle_phase_command(z80_byte value)
{
    //Hay que recibir comando aun
    if (pd765_input_parameters_index==0) {
        //Hay que recibir el comando
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Byte command: %02XH",value);
        
        //si esta haciendo seek y se lanza otro comando no seek, no aceptar
        if (signal_se.running) {
            if (value!=7 && value!=0xf) {
                DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: Ignore command on seek phase. Counter to finish seek: %d",signal_se.current_counter);
                return;
            }
        }
        

        if (value==8 && !pd765_interrupt_pending) {
            DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: SENSE INTERRUPT command without interrupt pending. Will generate invalid command");
        }

        if (value==3) {
            //Specify
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: SPECIFY command");
            pd765_command_received=PD765_COMMAND_SPECIFY;
            pd765_input_parameters_index++;
        }

        else if (value==4) {
            //Sense drive status
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: SENSE DRIVE STATUS command");
            pd765_command_received=PD765_COMMAND_SENSE_DRIVE_STATUS;
            pd765_input_parameters_index++;            
        }

        else if (value==7) {
            //Recalibrate
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: RECALIBRATE command");
            pd765_command_received=PD765_COMMAND_RECALIBRATE;
            pd765_input_parameters_index++;            
        }

        else if (value==8 && pd765_interrupt_pending) {
            //Sense interrupt status
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: SENSE INTERRUPT STATUS command");
            pd765_command_received=PD765_COMMAND_SENSE_INTERRUPT_STATUS;

            pd765_interrupt_pending=0;

            
            //No tiene parametros. Solo resultados
            pd765_handle_command_sense_interrupt_status();
        }

        else if ((value & 0xBF)==0x0A) {
            //Read id
            //TODO: bit MF
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: READ ID command. Current track: %02XH",pd765_pcn);
            pd765_command_received=PD765_COMMAND_READ_ID;
            pd765_input_parameters_index++; ;            
        }

        else if ((value & 0x1F)==0x06) {
            //Read data
            //TODO: bits MT, MF
            pd765_input_parameter_mt=(value>>7)&1;
            pd765_input_parameter_mf=(value>>6)&1;
            pd765_input_parameter_sk=(value>>5)&1;
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: READ DATA command. MT=%d MF=%d SK=%d. Current track: %02XH",
                pd765_input_parameter_mt,pd765_input_parameter_mf,pd765_input_parameter_sk,pd765_pcn);
            if (pd765_input_parameter_mt) {
                DBG_PRINT_PD765 VERBOSE_DEBUG,"MT parameter not handled yet");
                sleep(3);
            }


            pd765_command_received=PD765_COMMAND_READ_DATA;

            pd765_input_parameters_index++;         
        }    

        else if ((value & 0x1F)==0x0c) {
            //Read deleted data
            //TODO: bits MT, MF
            pd765_input_parameter_mt=(value>>7)&1;
            pd765_input_parameter_mf=(value>>6)&1;
            pd765_input_parameter_sk=(value>>5)&1;
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: READ DELETED DATA command. MT=%d MF=%d SK=%d. Current track: %02XH",
                pd765_input_parameter_mt,pd765_input_parameter_mf,pd765_input_parameter_sk,pd765_pcn);
            //sleep(10);
            
            if (pd765_input_parameter_mt) {
                DBG_PRINT_PD765 VERBOSE_DEBUG,"MT parameter not handled yet");
                sleep(3);
            }

             
            pd765_command_received=PD765_COMMAND_READ_DELETED_DATA;

            pd765_input_parameters_index++;         
        }   

        //Batman - The Movie (Erbe).dsk usa esto
        else if ((value & 0x9F)==0x02) {
            //Read track
            //TODO: bits MF
            pd765_input_parameter_mt=0;
            pd765_input_parameter_mf=(value>>6)&1;
            pd765_input_parameter_sk=(value>>5)&1;
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: READ TRACK command. MF=%d SK=%d. Current track: %02XH",
                pd765_input_parameter_mf,pd765_input_parameter_sk,pd765_pcn);
           

            pd765_command_received=PD765_COMMAND_READ_TRACK;

            pd765_input_parameters_index++;         
        }

        else if ((value & 0x3F)==0x05) {
            //write data
            //TODO: bits MT, MF
            pd765_input_parameter_mt=(value>>7)&1;
            pd765_input_parameter_mf=(value>>6)&1;
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: WRITE DATA command. MT=%d MF=%d Current track: %02XH",
                pd765_input_parameter_mt,pd765_input_parameter_mf,pd765_pcn);
            if (pd765_input_parameter_mt) {
                DBG_PRINT_PD765 VERBOSE_DEBUG,"MT parameter not handled yet");
                sleep(3);
            }


            pd765_command_received=PD765_COMMAND_WRITE_DATA;

            //Indicamos que el icono es el de grabado, 3 segundos de tiempo
            cf2_floppy_icon_is_saving=3;            

            pd765_input_parameters_index++;         
        }

        else if ((value & 0xBF)==0x0D) {
            //format track
            //TODO: bit MF
            pd765_input_parameter_mf=(value>>6)&1;
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: FORMAT TRACK command. MF=%d Current track: %02XH",
                pd765_input_parameter_mf,pd765_pcn);


            pd765_command_received=PD765_COMMAND_FORMAT_TRACK;

            //Indicamos que el icono es el de grabado, 3 segundos de tiempo
            cf2_floppy_icon_is_saving=3;

            pd765_input_parameters_index++;         
        }                                    

        else if (value==0x0F) {
            //Seek
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: SEEK command");
            pd765_command_received=PD765_COMMAND_SEEK;
            pd765_input_parameters_index++;            
        }                

        else {
            DBG_PRINT_PD765 VERBOSE_DEBUG,"---PD765: INVALID command");

            pd765_command_received=PD765_COMMAND_INVALID;

            //No tiene parametros. Solo resultados
            pd765_handle_command_invalid();

            if (value!=8) {
                //sleep(3);
                debug_printf(VERBOSE_ERR,"PD765: Invalid command %02XH on PC=%04XH",value,reg_pc);
                //DBG_PRINT_PD765 VERBOSE_DEBUG,"!!!!!!PD765: Invalid command %02XH on PC=%04XH",value,reg_pc);
            }
        }
    }
    else {
        //Recibiendo parametros de comando o en el caso de write, tambien escribiendo datos de sector
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Receiving command parameters. Index=%d",pd765_input_parameters_index);
        switch(pd765_command_received) {
            case PD765_COMMAND_SPECIFY:
                pd765_read_parameters_specify(value); 
            break;

            case PD765_COMMAND_SENSE_DRIVE_STATUS:
                pd765_read_parameters_sense_drive_status(value); 
            break;    

            case PD765_COMMAND_RECALIBRATE:
                pd765_read_parameters_recalibrate(value); 
            break;

            case PD765_COMMAND_SENSE_INTERRUPT_STATUS:
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: ERROR SENSE_INTERRUPT_STATUS has no input parameters");
            break;


            case PD765_COMMAND_READ_ID:
                pd765_read_parameters_read_id(value); 
            break; 

            case PD765_COMMAND_READ_DATA:
            case PD765_COMMAND_READ_DELETED_DATA:
            case PD765_COMMAND_READ_TRACK:
                pd765_read_parameters_read_data(value); 
            break;

            case PD765_COMMAND_WRITE_DATA:
                pd765_read_parameters_write_data(value); 
            break;

            case PD765_COMMAND_FORMAT_TRACK:
                pd765_read_parameters_format_track(value); 
            break;                             

            case PD765_COMMAND_SEEK:
                pd765_read_parameters_seek(value); 
            break;   

            case PD765_COMMAND_INVALID:
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: ERROR INVALID command has no input parameters");
            break;                                         
        }
    }
}

void pd765_write(z80_byte value)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Write command on pc %04XH: %02XH",reg_pc,value);

    switch (pd765_phase) {
        case PD765_PHASE_COMMAND:
            pd765_write_handle_phase_command(value);
        break;

        case PD765_PHASE_EXECUTION:
            DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Write command on phase execution SHOULD NOT happen");
            //TODO: no se puede escribir en este estado?

            //temporal
            //pd765_sc_handle_running(&signal_se);
        break;

        case PD765_PHASE_RESULT:
            DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Write command on phase result SHOULD NOT happen");
            //TODO: no se puede escribir en este estado?
        break;
    }
}


//
// FIN Gestion de escrituras de puerto
//

//
// Gestion de lecturas de puerto
//

z80_byte pd765_read_result_command_sense_drive_status(void)
{
    if (pd765_output_parameters_index==0) {
        z80_byte return_value=pd765_get_st3();
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST3: %02XH",return_value);

        //Y decir que ya no hay que devolver mas datos
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_DIO_MASK);

        //Y pasamos a fase command
        pd765_phase=PD765_PHASE_COMMAND;

        return return_value;
    }
    else {
        return 255;
    }
}

z80_byte pd765_read_result_command_invalid(void)
{
    if (pd765_output_parameters_index==0) {

        //Retornar ST0=80H
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Command invalid return 80H");

        //Y decir que ya no hay que devolver mas datos
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_DIO_MASK);

        //Y pasamos a fase command
        pd765_phase=PD765_PHASE_COMMAND;

        /*
        INVALID
        If an invalid command is sent to the FDC (a commend not defined above), then the FDC will terminate the command 
        after bits 7 and 6 of Status Register 0 are set to 1 and 0 respectively. No interrupt is generated by the MPD766 
        during this condition. Bit 6 and bit 7 (DIO and RQM) in the Main Status Register are both high ("1") indicating 
        to the processor that the uPD765 is in the Result Phase and the contents of Status Register 0 (STO) must be read. 
        When the processor reads Status Register 0 it will find a 80 hex indicating an invalid command was received.
        A Sense Interrupt Status Command must be sent after a Seek or Recalibrate Interrupt, otherwise the FDC will consider 
        the next command to be an Invelid Command.
        In some applications the user may wish to use this command as a No-Op command, to place the FDC in a standby or no operation state.
        */        

        return 0x80;
    }
    else {
        return 255;
    }
}

z80_byte pd765_read_result_command_sense_interrupt_status(void)
{
    //ST0, PCN
    /*
    SENSE INTERRUPT STATUS
    An Interrupt signal is generated by the FDC for one of the following reasons:
    1. Upon entering the Result Phase of:
    а. Read Data Command
    b. Read a Track Command
    c. Read ID Command
    d. Read Deleted Data Command
    e. Write Data Command
    f. Format a Cylinder Command
    g. Write Deleted Data Command
    h. Scan Commands

    2.    Ready Line of FDD changes state
    3.    End of Seek or Recalibrate Command
    4.    During Execution Phase in the NON-DMA Mode

    Interrupts caused by reasons 1 and 4 above occur during normal command operations and are easily 
    discernible by the processor. During an execution phase in NON-DMA Mode, DB5 in Main Status Register is high. 
    Upon entering Result Phase this bit gets clear. Reason 1 and 4 does not require Sense Interrupt Status command. 
    The interrupt is cleared by reading/writing data to FDC. 
    Interrupts caused by reasons 2 and 3 above may be uniquely 
    identified with the aid of the Sense Interrupt Status Commend. This command when issued resets the interrupt signal 
    and via bits 5, 6, and 7 of Status Register 0 identifies the cause of the interrupt.

    SEEK END |  INTERRUPT CODE | CAUSE
    Bit 5    |  Bit 6   Bit 7  |
    --------------------------------------------------------------------------------
    0           1          1   | Ready Line changed state, either polarity
    1           0          0   | Normal Termination of Seek or Recalibrate Command
    1           1          0   | Abnormal Termination of Seek or Recalibrate Command

    Neither the Seek or Recalibrate Command have a Result Phase. Therefore, it is mandatory to use the 
    Sense Interrupt Status Command after these commends to effectively terminate them and to provide verification 
    of where the heed is positioned (PCN).
    Issuing Sense Interrupt Status Command without interrupt pending is treated as an invalid command.
    */

    //This command when issued resets the interrupt signal and via bits 5, 6, and 7 of Status Register 0 identifies the cause of the interrupt.
    //pd765_interrupt_pending=0;

    if (pd765_output_parameters_index==0) {
        /*if (pd765_sc_get(&signal_se)) {
            DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Reset DB0 etc");

            pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_D0B_MASK - PD765_MAIN_STATUS_REGISTER_D1B_MASK - PD765_MAIN_STATUS_REGISTER_D2B_MASK - PD765_MAIN_STATUS_REGISTER_D3B_MASK);                
        }*/


        z80_byte return_value=pd765_get_st0();

        //TODO: esto viene de read data y creo que aqui no deberia hacerse
        /*if (anormal_termination) {
            return_value |= 0x40;

            //Y quitamos anormal_termination
            //TODO: esto se deberia hacer directamente en un byte de valor st0 y alterar ahi?
            anormal_termination=0;
        } */ 

        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning ST0: %02XH (%s)",return_value,(return_value & 32 ? "SE" : ""));

        pd765_output_parameters_index++;

        //Quitar flags de seek siempre que seek esté finalizado
        if (pd765_sc_get(&signal_se)) {
            DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Reset SE etc");

            //TODO: realmente hay que quitar señal SE  al leerlo desde sense interrupt?
            pd765_sc_reset(&signal_se);


            //TODO: dudoso si hacer esto aqui o donde: se resetea D0B, D1B etc antes o despues del sense interrupt?
            //pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_D0B_MASK - PD765_MAIN_STATUS_REGISTER_D1B_MASK - PD765_MAIN_STATUS_REGISTER_D2B_MASK - PD765_MAIN_STATUS_REGISTER_D3B_MASK);                

        }

        //else {
            //Pero si no estaba haciendo seek, no hay interrupcion y mandamos error
            //TODO mejorar esto
            //return_value |=0x80;
        //}

        return return_value;        
    }

    else if (pd765_output_parameters_index==1) {
        z80_byte return_value=pd765_pcn;
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Returning PCN: %02XH",return_value);

        //Y decir que ya no hay que devolver mas datos
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_DIO_MASK);

        //Decir que ya no esta busy
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_CB_MASK);        

        //Y pasamos a fase command
        pd765_phase=PD765_PHASE_COMMAND;

        return return_value;
    }
    else {
        return 255;
    }
}



z80_byte pd765_read_result_command_read_id(void)
{


    z80_byte return_value=pd765_get_buffer();
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Return byte from READ_ID at index %d: %02XH",pd765_output_parameters_index,return_value);
    pd765_output_parameters_index++;


    //if (pd765_output_parameters_index>=pd765_result_buffer_length) {
    if (pd765_buffer_read_is_final()) {        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: End of result buffer of READ_ID");

        //Y decir que ya no hay que devolver mas datos
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_DIO_MASK);

        //Decir que ya no esta busy
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_CB_MASK);

        //Y pasamos a fase command
        pd765_phase=PD765_PHASE_COMMAND;


        //pd765_set_interrupt_pending();

    }


    return return_value;        

}

z80_byte pd765_read_result_command_format_track(void)
{


    z80_byte return_value=pd765_get_buffer();
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Return byte from FORMAT_TRACK at index %d: %02XH",pd765_output_parameters_index,return_value);
    pd765_output_parameters_index++;


    //if (pd765_output_parameters_index>=pd765_result_buffer_length) {
    if (pd765_buffer_read_is_final()) {        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: End of result buffer of FORMAT_TRACK");

        //Y decir que ya no hay que devolver mas datos
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_DIO_MASK);

        //Decir que ya no esta busy
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_CB_MASK);

        //Y pasamos a fase command
        pd765_phase=PD765_PHASE_COMMAND;

        //sleep(2);

    }


    return return_value;        

}


z80_byte pd765_read_result_command_read_data(void)
{

    //Si finalizado por Terminal Count Signal
    /*
    if (pd765_terminal_count_signal.v && pd765_read_command_state==PD765_READ_COMMAND_STATE_READING_DATA) {
        DBG_PRINT_PD765 VERBOSE_INFO,"PD765: Stopping reading data because a Terminal Count signal has been fired");
        //sleep(10);

        pd765_reset_buffer();

        pd765_handle_command_read_data_read_chrn_etc(pd765_ultimo_sector_fisico_read,1);


        //E indicar fase ejecucion ha finalizado
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);


        pd765_set_interrupt_pending();    

        //Cambiamos a fase de resultado
        pd765_phase=PD765_PHASE_RESULT;

        //E indicar que hay que leer datos
        pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

        pd765_read_command_state=PD765_READ_COMMAND_STATE_ENDING_READING_DATA;

        z80_byte return_value=pd765_get_buffer();

        //TODO: no se si realmente hay que hacer clear, porque en este caso, para que querria el PCW
        //otro comando para hacer el clear si ya lo hace aqui automaticamente?
        //pd765_reset_terminal_count_signal();

        return return_value;

    }
    */


    if (pd765_read_command_state==PD765_READ_COMMAND_STATE_READING_DATA) {
        //notificar buffer de visual floppy
        menu_visual_floppy_buffer_add(pd765_pcn,pd765_ultimo_sector_fisico_read,pd765_result_bufer_read_pointer);

        //Estadisticas lectura
        pd765_read_stats_bytes_sec_acumulated++;
    }


    z80_byte return_value=pd765_get_buffer();
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Return byte from %s at index %d: %02XH",
        pd765_last_command_name(),
        pd765_result_bufer_read_pointer-1,return_value);
    //pd765_output_parameters_index++;



    //DBG_PRINT_PD765 VERBOSE_DEBUG,"Buscando sector size para pista %d",pd765_pcn);


    int sector_size=pd765_last_sector_size_read_data;

    if (sector_size==0) DBG_PRINT_PD765 VERBOSE_DEBUG,"SIZE: %d",sector_size);
    //sleep(5);

    if (pd765_buffer_read_is_final()) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: End of result buffer of %s",
        pd765_last_command_name()
        );

        //Fin de buffer de lectura
        pd765_reset_buffer();

        //Estaba leyendo datos de sector
        if (pd765_read_command_state==PD765_READ_COMMAND_STATE_READING_DATA) {

            int condition_end_sector=0;

            //Si leyendo pista, acabamos cuando el conteo total de sectores llega a EOT
            if (pd765_command_received==PD765_COMMAND_READ_TRACK) {
                if (pd765_total_sectors_read_track==pd765_input_parameter_eot) {
                    condition_end_sector=1;
                }
            }

            else {
                //Cumple EOT. No leer mas
                if (pd765_input_parameter_eot==pd765_read_command_searching_parameter_r) {
                    condition_end_sector=1;
                }
            }
   
            if (condition_end_sector || pd765_read_command_must_stop_anormal_termination) {
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Stopping reading next sector because R (%02XH) is EOT (%02XH) or Anormal termination, and send output parameters ST0,1,2, CHRN",
                    pd765_read_command_searching_parameter_r,pd765_input_parameter_eot);

                //pd765_input_parameter_r++;
                //pd765_read_command_searching_parameter_r++;
                    
                pd765_handle_command_read_data_read_chrn_etc(pd765_ultimo_sector_fisico_read,1);


                //E indicar fase ejecucion ha finalizado
                pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);


                pd765_set_interrupt_pending();    

                //Cambiamos a fase de resultado
                pd765_phase=PD765_PHASE_RESULT;

                //E indicar que hay que leer datos
                pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

                pd765_read_command_state=PD765_READ_COMMAND_STATE_ENDING_READING_DATA;
            }
            else {
                DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Reading next sector because R (%02XH) is not EOT (%02XH)",
                    pd765_read_command_searching_parameter_r,pd765_input_parameter_eot);
                //sleep(5);

                //pd765_input_parameter_r++;
                pd765_read_command_searching_parameter_r++;

                pd765_handle_command_read_data();
            }

        }


        //Estaba enviando valores ST1, ..CHRN 
        else {
            DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: End returning output parameters ST0,1,2, CHRN");

            //Y decir que ya no hay que devolver mas datos
            pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_DIO_MASK);

            //Decir que ya no esta busy
            pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_CB_MASK);            

            //Y pasamos a fase command
            pd765_phase=PD765_PHASE_COMMAND;

                   

        }
    }


    return return_value;        

}


z80_byte pd765_read_result_command_write_data(void)
{

 
    z80_byte return_value=pd765_get_buffer();
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Return byte from %s at index %d: %02XH",
        pd765_last_command_name(),
        pd765_result_bufer_read_pointer-1,return_value);
    //pd765_output_parameters_index++;



    //DBG_PRINT_PD765 VERBOSE_DEBUG,"Buscando sector size para pista %d",pd765_pcn);


    int sector_size=pd765_last_sector_size_write_data;

    if (sector_size==0) DBG_PRINT_PD765 VERBOSE_DEBUG,"SIZE: %d",sector_size);
    //sleep(5);

    if (pd765_buffer_read_is_final()) {
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: End of result buffer of %s",
        pd765_last_command_name()
        );

        //Fin de buffer de lectura
        pd765_reset_buffer();

       
        //Estaba enviando valores ST1, ..CHRN 
        
        DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: End returning output parameters ST0,1,2, CHRN from WRITE_DATA");

        //Y decir que ya no hay que devolver mas datos
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_DIO_MASK);

        //Decir que ya no esta busy
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_CB_MASK);            

        //Y pasamos a fase command
        pd765_phase=PD765_PHASE_COMMAND;

        //Fin de comando
        pd765_input_parameters_index=0;            

        //sleep(1);

                   

        
    }


    return return_value;        

}

z80_byte pd765_read_handle_phase_result(void)
{
    switch(pd765_command_received) {
        case PD765_COMMAND_SPECIFY:
            //No tiene resultado 
        break;

        case PD765_COMMAND_SENSE_DRIVE_STATUS:
            return pd765_read_result_command_sense_drive_status();
        break;            

        case PD765_COMMAND_RECALIBRATE:
            //No tiene resultado 
        break;

        case PD765_COMMAND_SEEK:
            //No tiene resultado 
        break;         

        case PD765_COMMAND_SENSE_INTERRUPT_STATUS:
            return pd765_read_result_command_sense_interrupt_status();
        break;

        case PD765_COMMAND_READ_ID:
            return pd765_read_result_command_read_id();
        break; 

        case PD765_COMMAND_READ_DATA:
        case PD765_COMMAND_READ_DELETED_DATA:
        case PD765_COMMAND_READ_TRACK:
            return pd765_read_result_command_read_data();
        break;

        case PD765_COMMAND_WRITE_DATA:
            return pd765_read_result_command_write_data();
        break;

        case PD765_COMMAND_FORMAT_TRACK:
            return pd765_read_result_command_format_track();
        break;             

        case PD765_COMMAND_INVALID:
            return pd765_read_result_command_invalid();
        break;                     
    }    

    return 255;
}

z80_byte pd765_read(void)
{
    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Read command on pc %04XH",reg_pc);
    dsk_show_activity();

    switch (pd765_phase) {
        case PD765_PHASE_COMMAND:
            //TODO: no se puede leer en este estado?
        break;

        case PD765_PHASE_EXECUTION:
            //TODO: no se puede leer en este estado?
        break;

        case PD765_PHASE_RESULT:
            return pd765_read_handle_phase_result();
        break;
    }    

    return 255;
}

//
// FIN Gestion de lecturas de puerto
//


z80_byte pd765_read_status_register(void)
{

    dsk_show_activity();

    if (signal_se.running) {
        //Mientras estamos en fase ejecucion, mantener pending_interrupt
        DBG_PRINT_PD765 VERBOSE_DEBUG," PD765: mantener pd765_interrupt_pending pues esta seek activo. Counter to finish seek: %d",signal_se.current_counter);
        //TODO no estoy seguro de esto pd765_set_interrupt_pending();
    }


    DBG_PRINT_PD765 VERBOSE_DEBUG,"PD765: Reading main status register on pc %04XH: %02XH (%s %s %s %s %s %s %s %s)",reg_pc,pd765_main_status_register,
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_RQM_MASK ? "RQM" : ""),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_DIO_MASK ? "DIO" : ""),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_EXM_MASK ? "EXM" : ""),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_CB_MASK  ? "CB" : ""),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_D3B_MASK ? "D3B" : ""),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_D2B_MASK ? "D2B" : ""),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_D1B_MASK ? "D1B" : ""),
            (pd765_main_status_register & PD765_MAIN_STATUS_REGISTER_D0B_MASK ? "D0B" : "")
    );


    //sleep(1);



    return pd765_main_status_register;
}

/*
void pd765_out_port_1ffd(z80_byte value)
{
    //0x1ffd: Setting bit 3 high will turn the drive motor (or motors, if you have more than one drive attached) on. 
    //Setting bit 3 low will turn them off again. (0x1ffd is also used for memory control).

    if (value&8) {
        dsk_show_activity();
        pd765_motor_on();
    }
    else {
        //Pues realmente si motor va a off, no hay actividad
        pd765_motor_off();
    }
 
}
*/

void pd765_out_port_data_register(z80_byte value)
{
    dsk_show_activity();

    //Puertos disco +3
    pd765_write(value);
}

void pd765_set_terminal_count_signal(void)
{

    
    
    if (pd765_command_received==PD765_COMMAND_READ_DATA && pd765_read_command_state==PD765_READ_COMMAND_STATE_READING_DATA) {
        DBG_PRINT_PD765 VERBOSE_INFO,"PD765: Stopping reading data because a Terminal Count signal has been fired");
        //sleep(10);

        pd765_reset_buffer();

        pd765_handle_command_read_data_read_chrn_etc(pd765_ultimo_sector_fisico_read,1);


        //E indicar fase ejecucion ha finalizado
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);


        pd765_set_interrupt_pending();    

        //Cambiamos a fase de resultado
        pd765_phase=PD765_PHASE_RESULT;

        //E indicar que hay que leer datos
        pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

        pd765_read_command_state=PD765_READ_COMMAND_STATE_ENDING_READING_DATA;

        

        //TODO: no se si realmente hay que hacer clear, porque en este caso, para que querria el PCW
        //otro comando para hacer el clear si ya lo hace aqui automaticamente?  
        //pd765_reset_terminal_count_signal();   
    }

    if (pd765_command_received==PD765_COMMAND_WRITE_DATA && pd765_write_command_state==PD765_WRITE_COMMAND_STATE_WRITING_DATA) {
        DBG_PRINT_PD765 VERBOSE_INFO,"PD765: Stopping writing data because a Terminal Count signal has been fired");
        //sleep(10);

        pd765_reset_buffer();

        //TODO: no estoy seguro de esto
        pd765_handle_command_read_data_read_chrn_etc(pd765_ultimo_sector_fisico_write,1);


        //E indicar fase ejecucion ha finalizado
        pd765_main_status_register &=(0xFF - PD765_MAIN_STATUS_REGISTER_EXM_MASK);


        pd765_set_interrupt_pending();    

        //Cambiamos a fase de resultado
        pd765_phase=PD765_PHASE_RESULT;

        //E indicar que hay que leer datos
        pd765_main_status_register |=PD765_MAIN_STATUS_REGISTER_DIO_MASK;

        pd765_read_command_state=PD765_WRITE_COMMAND_STATE_ENDING_WRITING_DATA;

        

        //TODO: no se si realmente hay que hacer clear, porque en este caso, para que querria el PCW
        //otro comando para hacer el clear si ya lo hace aqui automaticamente?  
        //pd765_reset_terminal_count_signal();   
    }         
}

/*
void pd765_reset_terminal_count_signal(void)
{
    pd765_terminal_count_signal.v=0;
}
*/

//Para indicar que en los ultimos 3 segundos estabamos grabando (write_data, format, ...)
//Aunque se cuele algun comando de lectura en los ultimos 3 segundos, mientras haya escrituras en ese intervalo,
//el icono sera el de salvado
//Icono de lectura: "Puntito" circular verde
//Icono de escritura: "Puntito" circular rojo
int cf2_floppy_icon_is_saving=0;

int cf2_floppy_icon_activity_antes_motor=0;

void cf2_floppy_icon_activity(void)
{

    if (!(MACHINE_IS_SPECTRUM || MACHINE_IS_CPC_HAS_FLOPPY || MACHINE_IS_PCW)) return;

    //printf("Cinta\n");
    lowericon_cf2_floppy_frame++;
    if (lowericon_cf2_floppy_frame==4) lowericon_cf2_floppy_frame=0;

    if (pd765_motor_status) {
        //printf("Refrescar icono floppy con motor encendido\n");
        menu_draw_ext_desktop();      
    }

    else {
        //Si ha cambiado de encendido a apagado
        if (cf2_floppy_icon_activity_antes_motor!=pd765_motor_status) {
    
            //Refrescar para hacer desaparecer el "puntito" del icono de floppy cuando se apaga motor
            //Y no quiero que salga el icono en color inverso, vendria inverso por el envio de algun comando final

            //printf("Refrescar icono floppy cuando se apaga motor\n");
            
            zxdesktop_icon_plus3_inverse=0;
            menu_draw_ext_desktop(); 
        }
    
    }

   cf2_floppy_icon_activity_antes_motor=pd765_motor_status;

    //Decrementar contador de salvado
    if (cf2_floppy_icon_is_saving!=0) cf2_floppy_icon_is_saving--;

}