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


#include "cpu.h"
#include "debug.h"
#include "tape.h"
#include "audio.h"
#include "screen.h"
#include "ay38912.h"
#include "operaciones.h"
#include "snap.h"
#include "jupiterace.h"
#include "timer.h"
#include "menu.h"
#include "compileoptions.h"
#include "contend.h"
#include "utils.h"
#include "realjoystick.h"
#include "chardetect.h"
#include "ula.h"
#include "settings.h"




z80_byte byte_leido_core_ace;


//bucle principal de ejecucion de la cpu de jupiter ace
void cpu_core_loop_ace(void)
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


		//Autoload
		//Si hay cinta insertada
		if (  (tape_loadsave_inserted & TAPE_LOAD_INSERTED)!=0 || (realtape_inserted.v==1)

		) {
			//Y si hay que hacer autoload
			if (initial_tap_load.v==1 && initial_tap_sequence==0) {

				//Para Ace. TODO
		                /*if (reg_pc==0xXXXXX) {
        		                debug_printf (VERBOSE_INFO,"Autoload tape");
                		        initial_tap_sequence=1;
		                }		
				*/


			}
		}

		
		

			if (tap_load_detect_ace()) {
				audio_playing.v=0;

				draw_tape_text();

				tap_load_ace();


	                        //audio_playing.v=1;
        	                timer_reset();

			}
		//Interceptar rutina de grabacion. TODO

			else	if (tap_save_detect_ace()) {
        	                	audio_playing.v=0;

					draw_tape_text();

					tap_save_ace();

	                        	timer_reset();

        	        }
	

		else {
			if (esperando_tiempo_final_t_estados.v==0) {


				byte_leido_core_ace=fetch_opcode();

				contend_read( reg_pc, 4 );

#ifdef EMULATE_CPU_STATS
                                util_stats_increment_counter(stats_codsinpr,byte_leido_core_ace);
#endif

				
				
                //Si la cpu está detenida por señal HALT, reemplazar opcode por NOP
                if (z80_halt_signal.v) {
                    byte_leido_core_ace=0;             
                }
                else {
                    reg_pc++;
                }

				//reg_r_antes_zx8081=reg_r;
				
				reg_r++;

	                	codsinpr[byte_leido_core_ace]  () ;



				/*
				if (iff1.v==1) {	
	
					//solo cuando cambia de 1 a 0
					if ( (reg_r_antes_zx8081 & 64)==64 && (reg_r & 64)==0 ) {

						interrupcion_maskable_generada.v=1;
						
					}
				}
				*/


                        }
                }



		//Esto representa final de scanline

		//normalmente
		if ( (t_estados/screen_testados_linea)>t_scanline  ) {

			t_scanline++;

			//generar_zx8081_horiz_sync();


                        //Envio sonido

                        audio_valor_enviar_sonido=0;

                        audio_valor_enviar_sonido +=da_output_ay();


			if (beeper_enabled.v) {

	                        if (beeper_real_enabled==0) {
        	                        audio_valor_enviar_sonido += da_amplitud_speaker_ace();
                	        }

                        	else {
                                	audio_valor_enviar_sonido += get_value_beeper_sum_array();
	                                beeper_new_line();
        	                }

			}

			//else {
			//	//Si no hay vsync sound, beeper sound off, forzamos que hay silencio de beepr
			//	beeper_silence_detection_counter=SILENCE_DETECTION_MAX;
			//}



                        if (realtape_inserted.v && realtape_playing.v) {
                                realtape_get_byte();
                                //audio_valor_enviar_sonido += realtape_last_value;
				if (realtape_loading_sound.v) {
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


			audio_send_mono_sample(audio_valor_enviar_sonido);


                        ay_chip_siguiente_ciclo();

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


				if (debug_registers) scr_debug_registers();

				//Hacer esto aun en esta maquina porque ese parpadeo tambien se usa en menu (cursor parpadeante)
                                contador_parpadeo--;
                                //printf ("Parpadeo: %d estado: %d\n",contador_parpadeo,estado_parpadeo.v);
                                if (!contador_parpadeo) {
                                        contador_parpadeo=16;
                                        toggle_flash_state();
                                }



				if (!interrupcion_timer_generada.v) {
					//Llegado a final de frame pero aun no ha llegado interrupcion de timer. Esperemos...
					esperando_tiempo_final_t_estados.v=1;
				}

				else {
					//Llegado a final de frame y ya ha llegado interrupcion de timer. No esperamos.... Hemos tardado demasiado
					//printf ("demasiado\n");
					esperando_tiempo_final_t_estados.v=0;
				}

                                //Final de instrucciones ejecutadas en un frame de pantalla
                                if (iff1.v==1) {
                                        interrupcion_maskable_generada.v=1;
                                }


			
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



		//Interrupcion de cpu. gestion im0/1/2. 
		if (interrupcion_maskable_generada.v || interrupcion_non_maskable_generada.v) {

			debug_fired_interrupt=1;

                        //ver si esta en HALT
                        if (z80_halt_signal.v) {
                                        z80_halt_signal.v=0;
                                        //reg_pc++;

                        }

			if (1==1) {

					if (interrupcion_non_maskable_generada.v) {
						debug_anota_retorno_step_nmi();
						interrupcion_non_maskable_generada.v=0;


						//NMI wait 14 estados
						t_estados += 14;


                                            

						//3 estados	
                                                
						//3 estados
                                         

												push_valor(reg_pc,PUSH_VALUE_TYPE_NON_MASKABLE_INTERRUPT);


						reg_r++;
						iff1.v=0;
						//printf ("Calling NMI with pc=0x%x\n",reg_pc);

						//Otros 6 estados
						t_estados += 6;

						//Total NMI: NMI WAIT 14 estados + NMI CALL 12 estados
						reg_pc= 0x66;

						//temp

						t_estados -=15;

						
					}

					//else {
					if (1==1) {

						
					//justo despues de EI no debe generar interrupcion
					//e interrupcion nmi tiene prioridad
						if (interrupcion_maskable_generada.v && byte_leido_core_ace!=251) {
						debug_anota_retorno_step_maskable();
						//Tratar interrupciones maskable


                                                //INT wait 10 estados. Valor de pruebas
                                                t_estados += 10;

						interrupcion_maskable_generada.v=0;

						

						push_valor(reg_pc,PUSH_VALUE_TYPE_MASKABLE_INTERRUPT);
						
						reg_r++;

						iff1.v=iff2.v=0;



						//IM0/1
						if (im_mode==0 || im_mode==1) {
							cpu_common_jump_im01();
						}
						else {
						//IM 2.

							z80_int temp_i;
							z80_byte dir_l,dir_h;	
							temp_i=get_im2_interrupt_vector();
							dir_l=peek_byte(temp_i++);
							dir_h=peek_byte(temp_i);
							reg_pc=value_8_to_16(dir_h,dir_l);
							t_estados += 7 ;

						}

					}
				}


			}

                }

                debug_get_t_stados_parcial_post();

}


