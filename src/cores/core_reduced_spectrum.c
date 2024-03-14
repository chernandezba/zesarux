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


#include "core_reduced_spectrum.h"
#include "core_spectrum.h"
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
#include "ula.h"
#include "utils.h"
#include "realjoystick.h"
#include "chardetect.h"
#include "diviface.h"
#include "timex.h"
#include "zxuno.h"
#include "prism.h"
#include "snap_rzx.h"
#include "superupgrade.h"
#include "pd765.h"
#include "tsconf.h"

#include "scrstdout.h"
#include "settings.h"

#include "snap_zsf.h"
#include "zeng.h"
#include "snap_ram.h"
#include "pd765.h"



//bucle principal de ejecucion de la cpu de spectrum
void cpu_core_loop_reduced_spectrum(void)
{

		timer_check_interrupt();

        //Eventos de la controladora de disco
        pd765_next_event_from_core();

		//Gestionar autoload
		gestionar_autoload_spectrum();


		if (tap_load_detect()) {
			//si estamos en pausa, no hacer nada
			if (!tape_pause) {
					audio_playing.v=0;

					draw_tape_text();

					tap_load();
					all_interlace_scr_refresca_pantalla();

					//printf ("refresco pantalla\n");
					//audio_playing.v=1;
					timer_reset();
			}

			else {
				core_spectrum_store_rainbow_current_atributes();
				//generamos nada. como si fuera un NOP

				contend_read( reg_pc, 4 );


			}
		}


		else {
			if (esperando_tiempo_final_t_estados.v==0) {

				core_spectrum_store_rainbow_current_atributes();



#ifdef DEBUG_SECOND_TRAP_STDOUT

        //Para poder debugar rutina que imprima texto. Util para aventuras conversacionales
        //hay que definir este DEBUG_SECOND_TRAP_STDOUT manualmente en compileoptions.h despues de ejecutar el configure

	scr_stdout_debug_print_char_routine();

#endif

				if (MACHINE_IS_TSCONF) tsconf_handle_frame_interrupts();



				contend_read( reg_pc, 4 );
				byte_leido_core_spectrum=fetch_opcode();




#ifdef EMULATE_CPU_STATS
				util_stats_increment_counter(stats_codsinpr,byte_leido_core_spectrum);
#endif

                //Si la cpu está detenida por señal HALT, reemplazar opcode por NOP
                if (z80_halt_signal.v) {
                    byte_leido_core_spectrum=0;
                }
                else {
                    reg_pc++;
                }

				reg_r++;



                z80_no_ejecutado_block_opcodes();
				codsinpr[byte_leido_core_spectrum]  () ;

				//printf ("t_estados:%d\n",t_estados);



            }
        }



		//ejecutar esto al final de cada una de las scanlines (312)
		//esto implica que al final del frame de pantalla habremos enviado 312 bytes de sonido


		//A final de cada scanline
		if ( (t_estados/screen_testados_linea)>t_scanline  ) {
			//printf ("%d\n",t_estados);
			//if (t_estados>69000) printf ("t_scanline casi final: %d\n",t_scanline);

			if (si_siguiente_sonido() ) {

				audio_valor_enviar_sonido=0;

				audio_valor_enviar_sonido +=da_output_ay();


				if (beeper_enabled.v) {
					if (beeper_real_enabled==0) {
						audio_valor_enviar_sonido += value_beeper;
					}

					else {
						audio_valor_enviar_sonido += get_value_beeper_sum_array();
						beeper_new_line();
					}

				}



				if (realtape_inserted.v && realtape_playing.v) {
					realtape_get_byte();
					if (realtape_loading_sound.v) {
                        reset_silence_detection_counter();
						audio_valor_enviar_sonido /=2;
						audio_valor_enviar_sonido += realtape_last_value/2;
						//Sonido alterado cuando top speed
						if (timer_condicion_top_speed() ) audio_valor_enviar_sonido=audio_change_top_speed_sound(audio_valor_enviar_sonido);
					}
				}

				//Ajustar volumen
				if (audiovolume!=100) {
					audio_valor_enviar_sonido=audio_adjust_volume(audio_valor_enviar_sonido);
				}

				//if (audio_valor_enviar_sonido>127 || audio_valor_enviar_sonido<-128) printf ("Error audio value: %d\n",audio_valor_enviar_sonido);

				audio_send_mono_sample(audio_valor_enviar_sonido);


				ay_chip_siguiente_ciclo();

				if (MACHINE_IS_TSCONF) {
					//y reseteo de esto, que es para interrupciones frame
					tsconf_handle_frame_interrupts_prev_horiz=9999;
				}

			}



			//final de linea

			//copiamos contenido linea y border a buffer rainbow
			if (rainbow_enabled.v==1) {
				if (next_frame_skip_render_scanlines) {
					//if ((t_estados/screen_testados_linea)>319) printf ("-Not storing rainbow buffer as framescreen_saltar is %d or manual frameskip\n",framescreen_saltar);
				}

				else {
					//if ((t_estados/screen_testados_linea)>319) printf ("storing rainbow buffer\n");
					screen_store_scanline_rainbow_solo_border();
					screen_store_scanline_rainbow_solo_display();
				}

				//t_scanline_next_border();

			}

			t_scanline_next_line();

			//se supone que hemos ejecutado todas las instrucciones posibles de toda la pantalla. refrescar pantalla y
			//esperar para ver si se ha generado una interrupcion 1/50

            if (t_estados>=screen_testados_total) {

				//tsconf_last_frame_y=-1;

				if (rainbow_enabled.v==1) t_scanline_next_fullborder();

				t_scanline=0;

				//printf ("final scan lines. total: %d\n",screen_scanlines);
				if (MACHINE_IS_INVES) {
						//Inves
						t_scanline_draw=screen_indice_inicio_pant;
						//printf ("reset inves a inicio pant : %d\n",t_scanline_draw);
				}

				else {
						//printf ("reset no inves\n");
						set_t_scanline_draw_zero();

				}


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

				//Para paperboy, thelosttapesofalbion0 y otros que hacen letras en el border, para que no se desplacen en diagonal
				//t_estados=0;
				//->paperboy queda fijo. thelosttapesofalbion0 no se desplaza, sino que tiembla si no forzamos esto


				//Final de instrucciones ejecutadas en un frame de pantalla
				if (iff1.v==1) {
					interrupcion_maskable_generada.v=1;

					//En Timex, ver bit 6 de puerto FF
					if ( MACHINE_IS_TIMEX_TS_TC_2068 && ( timex_port_ff & 64) ) interrupcion_maskable_generada.v=0;

					//TSConf lo gestiona mediante interrupciones de frame
					if (MACHINE_IS_TSCONF) interrupcion_maskable_generada.v=0;



				}


				cpu_loop_refresca_pantalla();


				siguiente_frame_pantalla();


				if (debug_registers) scr_debug_registers();

				contador_parpadeo--;
				//printf ("Parpadeo: %d estado: %d\n",contador_parpadeo,estado_parpadeo.v);
				if (!contador_parpadeo) {
						contador_parpadeo=16;
						toggle_flash_state();
				}


				if (!interrupcion_timer_generada.v) {
					//Llegado a final de frame pero aun no ha llegado interrupcion de timer. Esperemos...
					//printf ("no demasiado\n");
					esperando_tiempo_final_t_estados.v=1;
				}

				else {
					//Llegado a final de frame y ya ha llegado interrupcion de timer. No esperamos.... Hemos tardado demasiado
					//printf ("demasiado\n");
					esperando_tiempo_final_t_estados.v=0;
				}

				core_end_frame_check_zrcp_zeng_snap.v=1;

                //snapshot en ram
                snapshot_add_in_ram();

			}
			//Fin bloque final de pantalla


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
			//printf ("Generada interrupcion timer\n");
			interrupcion_timer_generada.v=0;
			esperando_tiempo_final_t_estados.v=0;
			interlaced_numero_frame++;
			//printf ("%d\n",interlaced_numero_frame);
		}


		//Interrupcion de cpu. gestion im0/1/2. Esto se hace al final de cada frame en spectrum o al cambio de bit6 de R en zx80/81
		if (interrupcion_maskable_generada.v || interrupcion_non_maskable_generada.v) {

			debug_fired_interrupt=1;

			//printf ("Generada interrupcion Z80\n");

			z80_adjust_flags_interrupt_block_opcode();

			//if (interrupcion_non_maskable_generada.v) printf ("generada nmi\n");

			//ver si esta en HALT
			if (z80_halt_signal.v) {
				z80_halt_signal.v=0;
				//reg_pc++;
			}


			if (interrupcion_non_maskable_generada.v) {
				debug_anota_retorno_step_nmi();
				//printf ("generada nmi\n");
				interrupcion_non_maskable_generada.v=0;


				//NMI wait 14 estados
				t_estados += 14;



				push_valor(reg_pc,PUSH_VALUE_TYPE_NON_MASKABLE_INTERRUPT);


				reg_r++;
				iff1.v=0;
				//printf ("Calling NMI with pc=0x%x\n",reg_pc);

				//Otros 6 estados
				t_estados += 6;

				//Total NMI: NMI WAIT 14 estados + NMI CALL 12 estados
				reg_pc= 0x66;



				t_estados -=15;

				if (superupgrade_enabled.v) {
					//Saltar a NMI de ROM0. TODO: que pasa con puertos 32765 y 8189?
					superupgrade_puerto_43b = 0;
					puerto_32765=0;
					puerto_8189=0;
					superupgrade_set_memory_pages();
				}



			}


			//justo despues de EI no debe generar interrupcion
			//e interrupcion nmi tiene prioridad
			if (interrupcion_maskable_generada.v && byte_leido_core_spectrum!=251) {

				//printf ("Lanzada interrupcion spectrum normal\n");

				debug_anota_retorno_step_maskable();
				//Tratar interrupciones maskable
				interrupcion_maskable_generada.v=0;

				interrupcion_si_despues_lda_ir();


				push_valor(reg_pc,PUSH_VALUE_TYPE_MASKABLE_INTERRUPT);

				reg_r++;

				//Caso Inves. Hacer poke (I*256+R) con 255
				if (MACHINE_IS_INVES) {
					z80_byte reg_r_total=(reg_r&127) | (reg_r_bit7 &128);

					z80_int dir=reg_i*256+reg_r_total;

					poke_byte_no_time(dir,255);
				}


				//desactivar interrupciones al generar una
				iff1.v=iff2.v=0;
				//Modelos spectrum

				if (im_mode==0 || im_mode==1) {
					cpu_common_jump_im01();
				}

				else {
				//IM 2.

					z80_int temp_i;
					z80_byte dir_l,dir_h;

					if (MACHINE_IS_TSCONF) temp_i=reg_i*256+tsconf_vector_fired_interrupt;

					else temp_i=get_im2_interrupt_vector();
					dir_l=peek_byte(temp_i++);
					dir_h=peek_byte(temp_i);
					reg_pc=value_8_to_16(dir_h,dir_l);
					t_estados += 7;


					//Para mejorar demos ula128 y scroll2017
					//Pero esto hace empeorar la demo ulatest3.tap
					if (ula_im2_slow.v) t_estados++;
				}

			}


  	  	}
	  	//Fin gestion interrupciones

		//Aplicar snapshot pendiente de ZRCP y ZENG envio snapshots. Despues de haber gestionado interrupciones
		if (core_end_frame_check_zrcp_zeng_snap.v) {
			core_end_frame_check_zrcp_zeng_snap.v=0;
			check_pending_zrcp_put_snapshot();
			zeng_send_snapshot_if_needed();
		}



}
