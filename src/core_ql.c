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



#include <stdlib.h>
#include <stdio.h>

#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>


#include "cpu.h"
#include "debug.h"
#include "tape.h"
#include "audio.h"
#include "screen.h"
#include "ay38912.h"
#include "operaciones.h"
#include "snap.h"
#include "timer.h"
#include "zxvision.h"
#include "compileoptions.h"
#include "contend.h"
#include "utils.h"
#include "realjoystick.h"
#include "chardetect.h"
#include "m68k.h"
#include "ula.h"
#include "settings.h"
#include "ql_i8049.h"
#include "ql_qdos_handler.h"
#include "ql_zx8302.h"
#include "zeng.h"
#include "snap_zsf.h"
#include "snap_ram.h"
#include "zeng_online_client.h"


z80_byte byte_leido_core_ql;



/*
void ql_chapuza_parpadeo_cursor(void)
{

        if (ql_simular_parpadeo_cursor.v==0) return;

                                //SV_FSTAT $AA word flashing cursor status

                        z80_byte parpadeo=peek_byte_z80_moto(0x280aa);
                        z80_byte parpadeo2=peek_byte_z80_moto(0x280ab);
                        //printf("parpadeo: %02X%02XH\n",parpadeo,parpadeo2);

                        temporal_parpadeo_ql++;
                        if ((temporal_parpadeo_ql % 16)==0) {

                                //Solo lo hago en estos casos y no siempre,
                                //para que no cambie cuando esta el boot y el test ram
                                //porque entonces el test ram fallaria
                                if (parpadeo==0 && parpadeo2==0x0C) {
                                        //printf("invertir parpadeo\n");
                                        parpadeo2=0x00;
                                }
                                else if (parpadeo==0 && parpadeo2==0x00) {
                                        //printf("invertir parpadeo\n");
                                        parpadeo2=0x0C;
                                }
                                //parpadeo ^=0x4E;
                                //poke_byte_z80_moto(0x280aa,parpadeo);
                                poke_byte_z80_moto(0x280ab,parpadeo2);
                        }
}
*/

//bucle principal de ejecucion de la cpu de jupiter ace
void cpu_core_loop_ql(void)
{

    debug_get_t_stados_parcial_pre();


    timer_check_interrupt();

//#ifdef COMPILE_STDOUT
//              if (screen_stdout_driver) scr_stdout_printchar();
//#endif
//
//#ifdef COMPILE_SIMPLETEXT
//                if (screen_simpletext_driver) scr_simpletext_printchar();
//#endif
    if (chardetect_detect_char_enabled.v) chardetect_detect_char();
    if (chardetect_printchar_enabled.v) chardetect_printchar();



    ql_rom_traps();



		if (0==1) { }




		else {
			if (esperando_tiempo_final_t_estados.v==0) {



#ifdef EMULATE_CPU_STATS
                util_stats_increment_counter(stats_codsinpr,byte_leido_core_ql);
#endif




	//Ejecutar opcode
#ifdef EMULATE_VISUALMEM
        set_visualmemopcodebuffer(get_pc_register()&QL_MEM_LIMIT ); //Le hago el AND porque a veces se sale de limite
#endif

                // Values to execute determine the interleave rate.
                // Smaller values allow for more accurate interleaving with multiple
                // devices/CPUs but is more processor intensive.
                // 100000 is usually a good value to start at, then work from there.

                // Note that I am not emulating the correct clock speed!
                z80_byte byte_primero=peek_byte_z80_moto(get_pc_register());
                z80_byte byte_segundo=peek_byte_z80_moto(get_pc_register()+1);

                if (byte_primero==0x4E && ((byte_segundo & 0xF0)==0x40) ) {
                        z80_byte trap_number=byte_segundo & 0xF;
                        //printf("Trap %d en %x\n",trap_number,get_pc_register());

                        if (ql_last_trap==4) {
                                ql_previous_trap_was_4=1;
                        }
                        else {
                                ql_previous_trap_was_4=0;
                        }

                        ql_last_trap=trap_number;
                }

                //if (get_pc_register()==0x79ca) {
                //        printf ("%x %x\n",byte_primero,byte_segundo);
                //}

		//	if (REG_IR==0x4E44) {
		//		printf("Possible Trap 4 en %x\n",get_pc_register());
		//	}

                m68k_execute(1);



				//Simplemente incrementamos los t-estados un valor inventado, aunque luego al final parece ser parecido a la realidad
				t_estados+=4;


                        }
                }



		//Esto representa final de scanline

		//normalmente
		if ( (t_estados/screen_testados_linea)>t_scanline  ) {

			t_scanline++;






            //Envio sonido

            audio_valor_enviar_sonido=0;

            //Usando simulacion por AY
            //audio_valor_enviar_sonido +=da_output_ay();

            //Usando emulacion del chip intel
            audio_valor_enviar_sonido +=ql_audio_da_output();



            //Ajustar volumen
            if (audiovolume!=100) {
                    audio_valor_enviar_sonido=audio_adjust_volume(audio_valor_enviar_sonido);
            }


            audio_send_mono_sample(audio_valor_enviar_sonido);




            ql_audio_next_cycle();

			//se supone que hemos ejecutado todas las instrucciones posibles de toda la pantalla. refrescar pantalla y
			//esperar para ver si se ha generado una interrupcion 1/50

			//Final de frame

			if (t_estados>=screen_testados_total) {

				t_scanline=0;

				timer_get_elapsed_core_frame_post();


                //Parche para maquinas que no generan 312 lineas, porque si enviamos menos sonido se escuchara un click al final
                //Es necesario que cada frame de pantalla contenga 312 bytes de sonido
                //Igualmente en la rutina de envio_audio se vuelve a comprobar que todo el sonido a enviar
                //este completo; esto es necesario para Z88

                int linea_estados=t_estados/screen_testados_linea;

                while (linea_estados<312) {
                        audio_send_mono_sample(audio_valor_enviar_sonido);
                        linea_estados++;
                }


				t_estados -=screen_testados_total;

				cpu_loop_refresca_pantalla();


				vofile_send_frame(rainbow_buffer);

				siguiente_frame_pantalla();

                contador_parpadeo--;
                //printf ("Parpadeo: %d estado: %d\n",contador_parpadeo,estado_parpadeo.v);
                if (!contador_parpadeo) {
                        contador_parpadeo=20; //TODO no se si esta es la frecuencia normal de parpadeo
                        toggle_flash_state();
                }


				if (debug_registers) scr_debug_registers();


				if (!interrupcion_timer_generada.v) {
					//Llegado a final de frame pero aun no ha llegado interrupcion de timer. Esperemos...
					esperando_tiempo_final_t_estados.v=1;
				}

				else {
					//Llegado a final de frame y ya ha llegado interrupcion de timer. No esperamos.... Hemos tardado demasiado
					//printf ("demasiado\n");
					esperando_tiempo_final_t_estados.v=0;
				}



/*
* read addresses
pc_intr equ     $18021  bits 4..0 set as pending level 2 interrupts
*/

                //Sirve para algo esto????
                //ql_pc_intr |=31;

                //hace que se lea tecla desde menu. Aunque con el 8 ya es suficiente
                //ql_pc_intr |=2;

                //frame. hace parpadear pantalla
                ql_pc_intr |=8;

                //No estoy seguro si esto son las interrupciones que genera el timer o no
                //Esto acaba generando llamadas a leer PC_INTR		Interrupt register
                m68k_set_irq(2);


                core_end_frame_check_zrcp_zeng_snap.v=1;

                //snapshot en ram
                snapshot_add_in_ram();

			}



		}

		if (esperando_tiempo_final_t_estados.v) {
			timer_pause_waiting_end_frame();
		}


              //Interrupcion de 1/50s. mapa teclas activas y joystick
                if (interrupcion_fifty_generada.v) {
                        interrupcion_fifty_generada.v=0;

                        //y de momento actualizamos tablas de teclado segun tecla leida
                        //printf ("Actualizamos tablas teclado %d ", temp_veces_actualiza_teclas++);
                       scr_actualiza_tablas_teclado();


                       //lectura de joystick
                       realjoystick_main();

                        //printf ("temp conta fifty: %d\n",tempcontafifty++);
                }


                //Interrupcion de procesador y marca final de frame
                if (interrupcion_timer_generada.v) {
                        interrupcion_timer_generada.v=0;
                        esperando_tiempo_final_t_estados.v=0;
                        interlaced_numero_frame++;
                        //printf ("%d\n",interlaced_numero_frame);

                        //Para calcular lo que se tarda en ejecutar todo un frame
                        timer_get_elapsed_core_frame_pre();
                }

                //Aplicar snapshot pendiente de ZRCP y ZENG envio snapshots. Despues de haber gestionado interrupciones
                if (core_end_frame_check_zrcp_zeng_snap.v) {
                    core_end_frame_check_zrcp_zeng_snap.v=0;
                    check_pending_zrcp_put_snapshot();
                    zeng_send_snapshot_if_needed();

                    zeng_online_client_end_frame_from_core_functions();
                }


                debug_get_t_stados_parcial_post();


}
