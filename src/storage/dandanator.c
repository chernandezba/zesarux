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

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif


#include "dandanator.h"
#include "cpu.h"
#include "debug.h"
#include "utils.h"
#include "operaciones.h"
#include "ula.h"
#include "audio.h"
#include "screen.h"
#include "menu_items.h"


z80_bit dandanator_enabled={0};

char dandanator_rom_file_name[PATH_MAX]="";

z80_byte *dandanator_memory_pointer;


//Rutinas originales antes de cambiarlas
//void (*dandanator_original_poke_byte)(z80_int dir,z80_byte valor);
//void (*dandanator_original_poke_byte_no_time)(z80_int dir,z80_byte valor);
//z80_byte (*dandanator_original_peek_byte)(z80_int dir);
//z80_byte (*dandanator_original_peek_byte_no_time)(z80_int dir);

//puntero a cpu core normal sin dandanator
//void (*cpu_core_loop_no_dandanator) (void);

z80_bit dandanator_switched_on={0};
//z80_bit dandanator_accepting_commands={0};

//z80_bit dandanator_status_blocked={0};

int dandanator_nested_id_core;
int dandanator_nested_id_poke_byte;
int dandanator_nested_id_poke_byte_no_time;
int dandanator_nested_id_peek_byte;
int dandanator_nested_id_peek_byte_no_time;


z80_byte dandanator_cpc_config_1;
/*
when bit 7=0:
b4-b3: FollowRomEnable zone 0 slot lower bits: slots 28,29,30 and 31
bit 2: Out bit to USB (Serial Bitbanging): “1” Idle.
bit 1: EEprom write enable “1” or disable “0”
bit 0: Serial port ena/dis - When enabled, LD A,(HL) returns serial RX in bit 0.


*/


z80_byte dandanator_cpc_config_2;
/*
when bit 7=1:
bit 6: Wait for "RET" (0xC9) to execute actions
bit 5: Disable Further Dandanator commands until reset
bit 4: Enable FollowRomEn on RET (only read if bit 6 = 1)
b3-b2: A15 values for zone 1 and zone 0. Zone 0 can be at 0x0000 or 0x8000, zone 1 can be at 0x4000 or 0xC000
b1-b0: Status of EEPROM_CE for zone 1 and zone 0. “0”: Enabled, “1” Disabled.
*/

z80_bit dandanator_cpc_received_preffix;

/*
Bits 4-0: Slot to assign to zone
Bit 5: Eeprom Chip Enable for zone. ‘0’ is enabled, ‘1’ is disabled.
Other bits: ignored.
*/
z80_byte dandanator_cpc_zone_slots[2];



z80_bit dandanator_cpc_pending_wait_ret;


z80_byte dandanator_cpc_change_ret_config;


/*
El Dandanator tiene tres formas de comportarse ante peticiones del Spectrum (las escrituras a rom):
  	A) Deshabilitado : Pasa de ellas.
	B) Bloqueado : Espera un comando especial específico (46) para pasar al estado C. Si llega cualquier otra cosa, lo ignora.
	C) Activo: Atiende a cualquier comando.
*/

//Estado A: dandanator_switched_on.v=0
//Estado B: dandanator_switched_on.v=1. dandanator_status_blocked.v=1
//Estado C: dandanator_switched_on.v=1

//Banco activo. 0..31 dandanator. 32=rom interna de spectrum
z80_byte dandanator_active_bank=0;


z80_byte dandanator_received_command;
z80_byte dandanator_received_data1;
z80_byte dandanator_received_data2;

z80_byte dandanator_contador_command; //Veces recibido
//z80_byte dandanator_contador_data1; //Veces recibido
//z80_byte dandanator_contador_data2; //Veces recibido


//0=Standby. Acabado de inicializar o finalizado un comando
//1=Recibidiendo comando
//#define DANDANATOR_COMMAND_STATUS_STANDBY 0
//#define DANDANATOR_COMMAND_STATUS_RECEIVING_COMMAND 1
//#define Pending_Executing_Command 2
//int dandanator_command_state;

//t_stados que hay que esperar antes de ejecutar comando
unsigned int dandanator_needed_t_states_command;

typedef enum                                                                    // State Machine for incoming communications from Spectrum
{
  Commands_Disabled,
  Commands_Locked,
  Wait_Normal,
  In_Progress_Normal,
  In_Progress_Locked,
  In_Progress_Special_Data1,
  In_Progress_Special_Data2,
  In_Progress_Fast,
  Pending_Processing,
  Pending_Executing_Command  //Este estado me dice que tengo que esperar determinados t-estados antes de ejecutar
} ZXCmdStateMachine;


/*
Esos son los estados, te los voy contando.

Commands_Disabled es el estado en el que arranca el Dandanator. Sólo cambia a Wait_Normal (que es que acepta comandos) si se selecciona el Slot 0 (o alguno de los dos slots definidos por el comando 49, que creo que no lo tienes implementado, pero no es importante ahora)
Se selecciona el slot 0 siempre al darle al botón. A este estado se vuelve mediante un comando 46 o mediante un comando 40 con el bit 3 activo. Date cuenta que el dandanator no cambia de Slot ni de rom externa/interna por ninguno de sus estados. Eso hay que cambiarlo explícitamente.

Commands_Locked es el estado al que se accede por el comando 46 o el comando 40 con el bit 2 activo. Ese lo tienes controlado. Sólo se sale de él volviendo al slot 0 (botón) o con un comando 46.

In_progress_Normal Se están recibiendo pulsos de un COMANDO, en tu caso es la parte en la que recibes cosas en la dirección 0x0001

In_progress_Locked Se están recibiendo pulsos de un COMANDO 46. (lo mismo que antes, pero veníamos de estar locked)

In_progress_Special_Data1/ In_progress_Special_Data2 se están recibiendo datos, en tu caso es cuando recibes cosas en las direcciones 0x0002 y 0x0003 respectivamente. Estos estados son iguales independientemente de que partiéramos de Wait_Normal o Commands_Locked.

In_progress_fast Se han recibido Comando, data1 y data 2 y se está esperando a recibir el pulso de confirmación (en tu caso, el pulso en la dirección 0x0000)

Pending processing, los comandos normales, los de 1 byte, se procesan el bucle principal. Este estado indica que hay comandos pendientes de procesar. Cuando se procesan se vuelve a Wait_Normal o a Commands_Disabled en el caso del comando 34.


Resumen importante. Ningún comando que no cambie explícitamente el banco del dandanator, debe producir cambios sobre los bancos, ya sean externos o internos. El botón equivale a un comando 40,0,1. Los comandos se habilitan sólos (Wait_Normal) por estar en el slot 0.

*/



ZXCmdStateMachine dandanator_state;


void dandanator_run_command_46(void)
{
                        if (dandanator_received_data1!=dandanator_received_data2) {
                                debug_printf (VERBOSE_DEBUG,"Dandanator: Data1 != Data2. Ignoring");
                                return;
                        }

	if (dandanator_received_data1==1) dandanator_state=Commands_Locked;
	if (dandanator_received_data1==16) dandanator_state=Wait_Normal;
	if (dandanator_received_data1==31) dandanator_state=Commands_Disabled;
}


void dandanator_footer_operating(void)
{
    generic_footertext_print_operating("DANDAN");

    //Y poner icono en inverso
    if (!zxdesktop_icon_dandanator_inverse) {
        //printf("icon activity\n");
        zxdesktop_icon_dandanator_inverse=1;
        menu_draw_ext_desktop();
    }
}



void dandanator_run_command(void)
{
	debug_printf (VERBOSE_DEBUG,"Dandanator: Running command %d",dandanator_received_command);
	if (dandanator_state==Commands_Locked) {
		if (dandanator_received_command==46) {
			/*
Comando especial 46: Bloquear, desbloquear, deshabilitar. – Este comando bloquea, desbloquea o deshabilita futuros comandos.
Es el único comando reconocido si los comandos están bloqueados.
Parámetros:
- Data 1: 1 para bloquear, 16 para desbloquear y habilitar comandos,
31 para deshabilitar comandos hasta un reboot frío. - Data 2: Debe ser igual a Data1 o el comando será ignorado.
			*/
			debug_printf (VERBOSE_DEBUG,"Dandanator: was in blocked mode. Received command 46 with data1 %d data2 %d",
		                dandanator_received_data1,dandanator_received_data2);

			dandanator_run_command_46();


		}

		else {
			debug_printf (VERBOSE_DEBUG,"Dandanator: is in blocked mode and command received is not 46. Not accepting anything");
		}

		return;
	}

	if (dandanator_received_command>=1 && dandanator_received_command<=33) {
		//Cambio banco de rom
		dandanator_active_bank=dandanator_received_command-1;
		debug_printf (VERBOSE_DEBUG,"Dandanator: Switching to dandanator bank %d",dandanator_active_bank);
	}

	else if (dandanator_received_command==34) {
		//El comando 34 -> es como el 33 pero bloquea futuros comandos.
                dandanator_state=Commands_Locked;
                debug_printf (VERBOSE_DEBUG,"Dandanator: Switching to normal ROM and locking dandanator future commands until command 46");
		dandanator_active_bank=32;
	}

	else if (dandanator_received_command==36) {
		reset_cpu();
	}

	else if (dandanator_received_command==39) {
		//TODO: establece el slot actual como el slot al que hay que volver si se pulsa un reset—>
		//Esto es, si recibes un comando 39 y estas en el slot 14 por ejemplo, cuando se pulse un reset en el
		// spectrum, independientemente de en qué slot estés, se selecciona primero el slot 14 y se hace un reset.
	}



	else if (dandanator_received_command>=40 && dandanator_received_command<=49) {
		debug_printf (VERBOSE_DEBUG,"Dandanator: Running special command %d with data1 %d data2 %d",
		dandanator_received_command,dandanator_received_data1,dandanator_received_data2);

		switch (dandanator_received_command) {
			case 40:
/*
Comando especial 40: Cambio Rápido – Este comando cambia a un banco determinado y
ejecuta una acción en el momento de recibir el pulso de confirmación. Normalmente se usa para devolver el control a un
software cambiando rápidamente de banco sin esperar los 5ms de la ventana de cambio normal.
Pruebas empíricas determinan el cambio de banco en el rango de los 12us.
Parámetros:
- Data 1: Número de banco para ejecutar el cambio.
- Data 2: Acción a ejecutar tras el cambio de banco (mascara de bits)
o Bits 4-7: Reservados. Siempre 0.
o Bit 3: Desactivar comandos futuros. Desactiva la recepción de comandos futuros por parte del Dandanator hasta un reset frio.
o Bit 2: Bloquear comandos futuros. Desactiva la recepción de comandos futuros hasta un comando especial determinado (46).
o Bit 1: NMI. Ejecuta una NMI tras cambiar el banco.
o Bit 0: Reset. Ejecuta un Reset del Z80 tras el cambio de banco.
*/

				dandanator_active_bank=dandanator_received_data1-1;
				debug_printf (VERBOSE_DEBUG,"Dandanator: Switching to dandanator bank %d and run some actions: %d",dandanator_active_bank,dandanator_received_data2);
				if (dandanator_received_data2 & 8) {
					debug_printf (VERBOSE_DEBUG,"Dandanator: Disabling dandanator");
					dandanator_state=Commands_Disabled;
				}
				if (dandanator_received_data2 & 4) {
					dandanator_state=Commands_Locked;
					debug_printf (VERBOSE_DEBUG,"Dandanator: Locking dandanator future commands until command 46");
				}

				if (dandanator_received_data2 & 2) {
					generate_nmi();
				}

				if (dandanator_received_data2 & 1) {
					reset_cpu();
				}

			break;

			case 41:
			case 42:
			case 43:
				debug_printf (VERBOSE_DEBUG,"Dandanator: Unusable on emulation");

			break;

			case 46:
				 dandanator_run_command_46();
			break;

			default:
				debug_printf (VERBOSE_DEBUG,"Dandanator: UNKNOWN command %d",dandanator_received_command);
			break;
		}
	}

	else {
		debug_printf (VERBOSE_DEBUG,"Dandanator: UNKNOWN command %d",dandanator_received_command);
	}
}

void dandanator_set_pending_run_command(void)
{
	dandanator_state=Pending_Executing_Command;
	debug_t_estados_parcial=0;
	debug_printf (VERBOSE_DEBUG,"Dandanator: Schedule pending command %d after %d t-states. PC=%d",dandanator_received_command,dandanator_needed_t_states_command,reg_pc);
}

void dandanator_write_byte_spectrum(z80_int dir,z80_byte valor)
{
	if (dandanator_state!=Commands_Disabled) {
		//printf ("Escribiendo dir %d valor %d PC=%d\n",dir,valor,reg_pc);

		if (dir==0) {
            //printf("actividad dandanator dir %d valor %d\n",dir,valor);
            dandanator_footer_operating();
			//Confirmacion de un comando de 3 bytes.

			//Ver si estabamos recibiendo comando
			if (dandanator_state==In_Progress_Fast) return;

			//Contador para ejecutar comando especial despues de 35 t-estados
			dandanator_needed_t_states_command=35;
			dandanator_set_pending_run_command();

		}

		if (dir==1) {
            //printf("actividad dandanator dir %d valor %d\n",dir,valor);
            dandanator_footer_operating();
			if (dandanator_state==Wait_Normal) {
				dandanator_contador_command=0;
				dandanator_received_command=valor;
				dandanator_state=In_Progress_Normal;
			}

			if (dandanator_state==In_Progress_Normal) {
				dandanator_contador_command++;

				//Si comando de 1 byte y se ha recibido el byte tantas veces como el valor de comando, ejecutar
				if (valor<40 && valor==dandanator_contador_command) {
					//Contador para ejecutar comando simple despues de X t-estados
					if (MACHINE_IS_SPECTRUM_16_48) {
						dandanator_needed_t_states_command=119;
					}
					else {
						dandanator_needed_t_states_command=121;
					}

					dandanator_set_pending_run_command();

				}
				else {
					//Si valor recibido no era el que habia de comando, resetear. Esto no deberia pasar...
					if (valor!=dandanator_received_command) {
						dandanator_state=In_Progress_Normal;
						dandanator_contador_command=1;
						dandanator_received_command=valor;
						debug_printf (VERBOSE_DEBUG,"Dandanator: Received different command before finishing previous...");
					}
				}


			}
		}

		if (dir==2) {
            //printf("actividad dandanator dir %d valor %d\n",dir,valor);
            dandanator_footer_operating();
			dandanator_received_data1=valor;  //Para simplificar no contamos numero de veces
		}

		if (dir==3) {
            //printf("actividad dandanator dir %d valor %d\n",dir,valor);
            dandanator_footer_operating();
			dandanator_received_data2=valor;  //Para simplificar no contamos numero de veces
		}


	}
}

//int temp_conta;

int dandanator_check_if_rom_area(z80_int dir)
{
    if (dandanator_switched_on.v) {
        if (dir<16384) {
            //temp_conta++;
            //if ((temp_conta % 1000)==0) printf("accessing rom space with dandanator switched on dir %d\n",dir);
            return 1;
        }
    }
	return 0;
}

z80_byte dandanator_read_byte_spectrum(z80_int dir)
{
	//Si banco apunta a 32, es la rom interna
	//if (dandanator_active_bank==32) return dandanator_original_peek_byte_no_time(dir);
	if (dandanator_active_bank==32) return debug_nested_peek_byte_no_time_call_previous(dandanator_nested_id_peek_byte_no_time,dir);

	//Si no, memoria dandanator
	int puntero=dandanator_active_bank*16384+dir;
	return dandanator_memory_pointer[puntero];
}


z80_byte dandanator_poke_byte_spectrum(z80_int dir,z80_byte valor)
{

	//dandanator_original_poke_byte(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_call_previous(dandanator_nested_id_poke_byte,dir,valor);

	if (dandanator_check_if_rom_area(dir)) {
		dandanator_write_byte_spectrum(dir,valor);
	}

	//temp
	//if (dir<16384 && !dandanator_check_if_rom_area(dir)) {
	//	printf ("Recibido write en %d valor %d con dandanator desactivado\n",dir,valor);
	//}

        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}

z80_byte dandanator_poke_byte_spectrum_no_time(z80_int dir,z80_byte valor)
{
        //dandanator_original_poke_byte_no_time(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_no_time_call_previous(dandanator_nested_id_poke_byte_no_time,dir,valor);


	if (dandanator_check_if_rom_area(dir)) {
                dandanator_write_byte_spectrum(dir,valor);
        }


        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}

z80_byte dandanator_peek_byte_spectrum(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(dandanator_nested_id_peek_byte,dir);

	if (dandanator_check_if_rom_area(dir)) {
       		//t_estados +=3;
		return dandanator_read_byte_spectrum(dir);
	}

	//return dandanator_original_peek_byte(dir);
	return valor_leido;
}

z80_byte dandanator_peek_byte_spectrum_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(dandanator_nested_id_peek_byte_no_time,dir);

	if (dandanator_check_if_rom_area(dir)) {
                return dandanator_read_byte_spectrum(dir);
        }

	//else return debug_nested_peek_byte_no_time_call_previous(dandanator_nested_id_peek_byte_no_time,dir);
	return valor_leido;
}

z80_byte dandanator_cpc_zone(z80_int dir)
{
	z80_int mask=dir & 0xC000;

	if (mask==0x0000 || mask==0x8000) return 0;
	else return 1;
}


//-1 si no mapeado. Otro valor: retorna zona
int dandanator_cpc_is_mapped(z80_int dir)
{
	//zone 0 may be mapped to 0x0000 or to 0x8000 and zone 1 to 0x4000 or 0xC000.
	z80_byte zone=dandanator_cpc_zone(dir);

	//Ver si esa zona esta mapeado dandanator
	if ((dandanator_cpc_zone_slots[zone] & 32)==0) {
		//Bit 5: Eeprom Chip Enable for zone. ‘0’ is enabled, ‘1’ is disabled.
		//b3-b2: A15 values for zone 1 and zone 0.
		//Zone 0 can be at 0x0000 or 0x8000, zone 1 can be at 0x4000 or 0xC000
		z80_int value_a15;
		if (zone==0) {
			value_a15=((dandanator_cpc_config_2 & 4) >>2)*0x8000;
		}
		else value_a15=((dandanator_cpc_config_2 & 8) >>3)*0x8000;

		if ( (dir&0x8000) == value_a15) return zone;


	}

	return -1;

}





z80_byte dandanator_read_byte_cpc(z80_int dir,z80_byte zone)
{

	//printf ("Reading dir %04XH zone %d\n",dir,zone);

	z80_byte slot=dandanator_cpc_zone_slots[zone] & 31;

	//if (slot!=0) printf ("Reading dir %04XH zone %d slot %d\n",dir,zone,slot);

	int puntero=slot*16384+(dir & 16383);
	return dandanator_memory_pointer[puntero];

}


z80_byte dandanator_poke_byte_cpc(z80_int dir,z80_byte valor)
{

	//dandanator_original_poke_byte(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_call_previous(dandanator_nested_id_poke_byte,dir,valor);


        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}

z80_byte dandanator_poke_byte_cpc_no_time(z80_int dir,z80_byte valor)
{
        //dandanator_original_poke_byte_no_time(dir,valor);
        //Llamar a anterior
        debug_nested_poke_byte_no_time_call_previous(dandanator_nested_id_poke_byte_no_time,dir,valor);


        //Para que no se queje el compilador, aunque este valor de retorno no lo usamos
        return 0;


}

z80_byte dandanator_peek_byte_cpc(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_call_previous(dandanator_nested_id_peek_byte,dir);

	int zone=dandanator_cpc_is_mapped(dir);

	if (zone!=-1) {
       		//t_estados +=3;
		return dandanator_read_byte_cpc(dir,zone);
	}

	return valor_leido;
}

z80_byte dandanator_peek_byte_cpc_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{

	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(dandanator_nested_id_peek_byte_no_time,dir);

	int zone=dandanator_cpc_is_mapped(dir);

	if (zone!=-1) {
       		//t_estados +=3;
		return dandanator_read_byte_cpc(dir,zone);
	}

	return valor_leido;
}


//Establecer rutinas propias
void dandanator_set_peek_poke_functions(void)
{

	if (MACHINE_IS_SPECTRUM) {
   		debug_printf (VERBOSE_DEBUG,"Setting dandanator poke / peek Spectrum functions");

		//Asignar mediante nuevas funciones de core anidados
		dandanator_nested_id_poke_byte=debug_nested_poke_byte_add(dandanator_poke_byte_spectrum,"Dandanator poke_byte");
		dandanator_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(dandanator_poke_byte_spectrum_no_time,"Dandanator poke_byte_no_time");
		dandanator_nested_id_peek_byte=debug_nested_peek_byte_add(dandanator_peek_byte_spectrum,"Dandanator peek_byte");
		dandanator_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(dandanator_peek_byte_spectrum_no_time,"Dandanator peek_byte_no_time");

	}

	else {
		debug_printf (VERBOSE_DEBUG,"Setting dandanator poke / peek CPC functions");

		//Asignar mediante nuevas funciones de core anidados
		dandanator_nested_id_poke_byte=debug_nested_poke_byte_add(dandanator_poke_byte_cpc,"Dandanator poke_byte");
		dandanator_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(dandanator_poke_byte_cpc_no_time,"Dandanator poke_byte_no_time");
		dandanator_nested_id_peek_byte=debug_nested_peek_byte_add(dandanator_peek_byte_cpc,"Dandanator peek_byte");
		dandanator_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(dandanator_peek_byte_cpc_no_time,"Dandanator peek_byte_no_time");

	}

}

//Restaurar rutinas de dandanator
void dandanator_restore_peek_poke_functions(void)
{
                debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before dandanator");
                //poke_byte=dandanator_original_poke_byte;
                //poke_byte_no_time=dandanator_original_poke_byte_no_time;
                //peek_byte=dandanator_original_peek_byte;
                //peek_byte_no_time=dandanator_original_peek_byte_no_time;


	debug_nested_poke_byte_del(dandanator_nested_id_poke_byte);
	debug_nested_poke_byte_no_time_del(dandanator_nested_id_poke_byte_no_time);
	debug_nested_peek_byte_del(dandanator_nested_id_peek_byte);
	debug_nested_peek_byte_no_time_del(dandanator_nested_id_peek_byte_no_time);
}




z80_byte cpu_core_loop_spectrum_dandanator(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{

	//Llamar a anterior
	debug_nested_core_call_previous(dandanator_nested_id_core);


    if (dandanator_state==Pending_Executing_Command) {
                if (debug_t_estados_parcial>dandanator_needed_t_states_command) {
                        debug_printf (VERBOSE_DEBUG,"Dandanator: Run command after needed %d t-states",dandanator_needed_t_states_command);
                        dandanator_run_command();
                        //Volver a standby normalmente.
                        if (dandanator_state==Pending_Executing_Command) dandanator_state=Wait_Normal;
                }
        }

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;

}

//Cambio de parametros cuando hay delayed RET pero estos settings no son delayed
void dandanator_cpc_execute_ret_nondelayed_config(z80_byte value)
{
	//printf ("Changing non delayed config parameters on delayed execution\n");
                                        if (value & 128) {

						//O sea, solo cambiar bits 7 y 6, resto eliminar

						dandanator_cpc_config_2 &=63;
                                                dandanator_cpc_config_2 |=(value&(128+64));


                                        }
                                        else {
                                                dandanator_cpc_config_1=value;
                                        }
}

//Cambio de parametros que se ven retrasados hasta un RET
void dandanator_cpc_execute_ret_delayed_config(z80_byte value)
{
/*
Delayed configuration parameters are:
- Disable further commands until reset ->  dandanator_cpc_config_2 bit 5
- Trigger FollowRomEnable, so Dandanator will only be enabled if a Rom is selected
in the CPC. Useful for low-high rom substitution (poor-man rombox).  -> dandanator_cpc_config_2 bit 4
- A15 values for Zone 0 and Zone 1 so you can change them between segments  -> dandanator_cpc_config_2 bit 3-2
- Enable status for Zone 0 and Zone 1 -> Zones may remain enabled or get disabled. -> dandanator_cpc_config_2 bits 1-0

O sea, conservar bits 7 y 6, resto eliminar

*/

	//printf ("Changing delayed config parameters on delayed execution\n");

                                        if (value & 128) {

						dandanator_cpc_config_2 &=(128+64);
                                                dandanator_cpc_config_2 |= (value&63);

                                                //Escribir en los dos bits bajos es lo mismo que escribir en bit 5 de los dos slots
                                                //b1-b0: Status of EEPROM_CE for zone 1 and zone 0. “0”: Enabled, “1” Disabled.
                                                z80_byte slot_enabled_zone0=value&1;
                                                z80_byte slot_enabled_zone1=(value&2)>>1;

                                                dandanator_cpc_zone_slots[0] &=31;
                                                dandanator_cpc_zone_slots[0] |= (32*slot_enabled_zone0);

                                                dandanator_cpc_zone_slots[1] &=31;
                                                dandanator_cpc_zone_slots[1] |= (32*slot_enabled_zone1);


                                        }

}

void dandanator_cpc_execute_ret_config(z80_byte value)
{

	//printf ("Executing immediate config change\n");



                                        if (value & 128) {
												//printf ("Setting config 2 to %02XH\n",value);

                                                dandanator_cpc_config_2=value;

                                                //Escribir en los dos bits bajos es lo mismo que escribir en bit 5 de los dos slots
                                                //b1-b0: Status of EEPROM_CE for zone 1 and zone 0. “0”: Enabled, “1” Disabled.
                                                z80_byte slot_enabled_zone0=value&1;
                                                z80_byte slot_enabled_zone1=(value&2)>>1;

                                                dandanator_cpc_zone_slots[0] &=31;
                                                dandanator_cpc_zone_slots[0] |= (32*slot_enabled_zone0);

                                                dandanator_cpc_zone_slots[1] &=31;
                                                dandanator_cpc_zone_slots[1] |= (32*slot_enabled_zone1);


                                        }
                                        else {
											//printf ("Setting config 1 to %02XH\n",value);

                                                dandanator_cpc_config_1=value;
                                        }

}


z80_byte cpu_core_loop_cpc_dandanator(z80_int dir GCC_UNUSED, z80_byte value GCC_UNUSED)
{

	z80_byte preffix=peek_byte_no_time(reg_pc);
	z80_byte opcode=peek_byte_no_time(reg_pc+1);
	z80_byte opcode2=peek_byte_no_time(reg_pc+2);
	//z80_int reg_pc_previous=reg_pc;

	//Llamar a anterior core
	debug_nested_core_call_previous(dandanator_nested_id_core);


	//Ver bit config Disable Further Dandanator commands until reset
	//bit 5: Disable Further Dandanator commands until reset
	if ((dandanator_cpc_config_2 & 32)==0) {


		//Gestion opcodes
		/*
		- Zone 0 Command: Trigger + LD (IY+0),B
		- Zone 1 Command: Trigger + LD (IY+0),C
		*/


		//printf ("%04X %02X %02X\n",reg_pc,preffix,opcode);


		if (preffix==201 && dandanator_cpc_pending_wait_ret.v) {
			dandanator_cpc_execute_ret_delayed_config(dandanator_cpc_change_ret_config);
			dandanator_cpc_pending_wait_ret.v=0;
            //printf("1\n");
            dandanator_footer_operating();
		}


		if (preffix==0xFD) {
			if (opcode==0xFD) { // && opcode2==0xFD) {
				if (opcode2==0xFD) {
 					//printf ("Recibido FDFDFD on PC=%04XH\n",reg_pc_previous);
					dandanator_cpc_received_preffix.v=1;
				}
			}
			else {
				if (dandanator_cpc_received_preffix.v) {
					switch (opcode) {
					case 112:
						//LD (IY+d),B
						//Zone 0 Command: Trigger + LD (IY+0),B
						dandanator_cpc_zone_slots[0]=reg_b;
                        //printf("2\n");
                        //dandanator_footer_operating();
						//printf ("Setting zone 0 slot %d PC=%04XH\n",dandanator_cpc_zone_slots[0],reg_pc_previous);
					break;

					case 113:
						//LD (IY+d),C
						//Zone 1 Command: Trigger + LD (IY+0),C
						dandanator_cpc_zone_slots[1]=reg_c;
                        //printf("3\n");
                        //dandanator_footer_operating();
						//printf ("Setting zone 1 slot %d PC=%04XH\n",dandanator_cpc_zone_slots[1],reg_pc_previous);
					break;

					case 119:
						//LD (IY+d),A
						//printf ("Setting config value reg_a = %02XH PC=%04XH\n",reg_a,reg_pc_previous);

						//Esto se ve afectado por el setting "wait for ret", que se lee del valor enviado actual
                        //printf("4\n");
                        dandanator_footer_operating();
						if (reg_a & 64) {

							/*
							Delayed configuration parameters are:
							- Disable further commands until reset.
							- Trigger FollowRomEnable, so Dandanator will only be enabled if a Rom is selected
							in the CPC. Useful for low-high rom substitution (poor-man rombox).
							- A15 values for Zone 0 and Zone 1 so you can change them between segments
							- Enable status for Zone 0 and Zone 1 -> Zones may remain enabled or get disabled.

							*/

							//wait for ret
							dandanator_cpc_change_ret_config=reg_a;
							dandanator_cpc_pending_wait_ret.v=1;
							//printf ("Delaying some config change until ret\n");

							dandanator_cpc_execute_ret_nondelayed_config(reg_a);

						}

						else {
							//printf ("Running config change inmmediately\n");
							dandanator_cpc_execute_ret_config(reg_a);
						}


					}
				}

				dandanator_cpc_received_preffix.v=0;

			}

		}


	}

	else {
		//printf ("Dandanator is disabled until reset\n");
	}

	//Para que no se queje el compilador, aunque este valor de retorno no lo usamos
	return 0;

}


void dandanator_set_core_function(void)
{
	debug_printf (VERBOSE_DEBUG,"Setting dandanator Core loop");

	//Asignar mediante nuevas funciones de core anidados
	if (MACHINE_IS_SPECTRUM) {
		dandanator_nested_id_core=debug_nested_core_add(cpu_core_loop_spectrum_dandanator,"Dandanator Spectrum core");
	}
	else {
		dandanator_nested_id_core=debug_nested_core_add(cpu_core_loop_cpc_dandanator,"Dandanator CPC core");
	}
}




void dandanator_restore_core_function(void)
{
        debug_printf (VERBOSE_DEBUG,"Restoring original dandanator core");
	debug_nested_core_del(dandanator_nested_id_core);
}



void dandanator_alloc_memory(void)
{
        int size=(DANDANATOR_SIZE+256);  //+256 bytes adicionales para la memoria no volatil

        debug_printf (VERBOSE_DEBUG,"Allocating %d kb of memory for dandanator emulation",size/1024);

        dandanator_memory_pointer=malloc(size);
        if (dandanator_memory_pointer==NULL) {
                cpu_panic ("No enough memory for dandanator emulation");
        }


}

int dandanator_load_rom(void)
{

        FILE *ptr_dandanator_romfile;
        int leidos=0;

        debug_printf (VERBOSE_INFO,"Loading dandanator rom %s",dandanator_rom_file_name);

  			ptr_dandanator_romfile=fopen(dandanator_rom_file_name,"rb");
                if (!ptr_dandanator_romfile) {
                        debug_printf (VERBOSE_ERR,"Unable to open ROM file");
                }

        if (ptr_dandanator_romfile!=NULL) {

                leidos=fread(dandanator_memory_pointer,1,DANDANATOR_SIZE,ptr_dandanator_romfile);
                fclose(ptr_dandanator_romfile);

        }



        if (leidos!=DANDANATOR_SIZE || ptr_dandanator_romfile==NULL) {
                debug_printf (VERBOSE_ERR,"Error reading dandanator rom");
                return 1;
        }

        return 0;
}



void dandanator_enable(void)
{

  if (!MACHINE_IS_SPECTRUM && !MACHINE_IS_CPC) {
    debug_printf(VERBOSE_INFO,"Can not enable dandanator on non Spectrum or CPC machine");
    return;
  }

	if (dandanator_enabled.v) return;

	if (dandanator_rom_file_name[0]==0) {
		debug_printf (VERBOSE_ERR,"Trying to enable Dandanator but no ROM file selected");
		return;
	}

	dandanator_alloc_memory();
	if (dandanator_load_rom()) return;

	dandanator_set_peek_poke_functions();

	dandanator_set_core_function();


	dandanator_switched_on.v=0;
	//dandanator_accepting_commands.v=0;
	dandanator_state=Commands_Disabled;
	dandanator_active_bank=0;


	/*Quitar audiofilter rom save porque interfiere con efecto colores menu:
	Esto intercepta las llamadas a rom save direcciones entre if (reg_pc>1200 && reg_pc<1350
	Pero precisamente dandanator hace cambio de colores de border en las direcciones:
	1122, 1143, 1154, 1165, 1191, 1201, 1211.
	O sea, hay dos direcciones que entran dentro de la condición y el resto no,
	provocando que se haga sonido de una manera en dos direcciones, y de otra en las otras direcciones
	Resultado: genera un sonido desagradable de 50 hz

	*/

	debug_printf(VERBOSE_DEBUG,"Disabling audio filter on rom save setting because it interfieres with Dandanator border effect");

	output_beep_filter_on_rom_save.v=0;


	dandanator_enabled.v=1;


}

void dandanator_disable(void)
{
	if (dandanator_enabled.v==0) return;

	dandanator_restore_peek_poke_functions();

	free(dandanator_memory_pointer);

	dandanator_restore_core_function();

	dandanator_enabled.v=0;
}


void dandanator_press_button(void)
{

        if (dandanator_enabled.v==0) {
                debug_printf (VERBOSE_ERR,"Trying to press Dandanator button when it is disabled");
                return;
        }

        dandanator_switched_on.v=1;
	//dandanator_accepting_commands.v=1;
	dandanator_active_bank=0;
	dandanator_state=Wait_Normal;
	//dandanator_status_blocked.v=0;


/*
Dandanator CPC:
Normal boot:
• Zone 0 enabled on slot0 and segment 0x0000.
• Zone 1 disabled on slot0 and segment 0x4000.
• USB RX off.
• USB TX on idle state : ‘1’.
• Eeprom writes off.
• FollowRomEnable off.
*/

	dandanator_cpc_config_1=0;
	dandanator_cpc_config_2=0;


	dandanator_cpc_received_preffix.v=0;
	dandanator_cpc_zone_slots[0]=0;
	dandanator_cpc_zone_slots[1]=32;


	dandanator_cpc_pending_wait_ret.v=0;

	reset_cpu();


}
