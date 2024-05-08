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


#include "scmp.h"
#include "cpu.h"
#include "debug.h"
#include "audio.h"

#include "screen.h"
#include "ay38912.h"
#include "operaciones.h"
#include "timer.h"
#include "zxvision.h"
#include "compileoptions.h"
#include "contend.h"
#include "utils.h"
#include "realjoystick.h"
#include "ula.h"
#include "tape.h"
#include "settings.h"

#include "snap_zsf.h"
#include "zeng.h"
#include "zeng_online_client.h"



//bucle principal de ejecucion de la cpu de mk14
void cpu_core_loop_mk14(void)
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
  //if (chardetect_detect_char_enabled.v) chardetect_detect_char();
  //if (chardetect_printchar_enabled.v) chardetect_printchar();


  if (0==1) { }

  else {
	   if (esperando_tiempo_final_t_estados.v==0) {



#ifdef EMULATE_CPU_STATS
                                //util_stats_increment_counter(stats_codsinpr,byte_leido_core_ql);
#endif


//temp debug
/*
char buffer[80];
printf ("%04XH ",scmp_m_PC.w.l);
int longitud=scmp_CPU_DISASSEMBLE( scmp_m_PC.w.l , peek_byte_no_time(scmp_m_PC.w.l), peek_byte_no_time(scmp_m_PC.w.l+1), buffer);
printf ("%02X ",peek_byte_no_time(scmp_m_PC.w.l));
if (longitud>1) printf ("%02X ",peek_byte_no_time(scmp_m_PC.w.l+1));
else printf ("   ");

 //printf ("longitud: %d %s\n",longitud,buffer);
 printf ("%s\n",buffer);
*/


#ifdef EMULATE_VISUALMEM
        set_visualmemopcodebuffer(scmp_m_PC.w.l);
#endif


      scmp_run_opcode();


    }
  }



  //Esto representa final de scanline

	//normalmente
	if ( (t_estados/screen_testados_linea)>t_scanline  ) {

			t_scanline++;



                        //Envio sonido

                        audio_valor_enviar_sonido=0;

                        audio_valor_enviar_sonido +=da_output_ay();




                        if (realtape_inserted.v && realtape_playing.v) {
                            realtape_get_byte();
                            //audio_valor_enviar_sonido += realtape_last_value;
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

                core_end_frame_check_zrcp_zeng_snap.v=1;


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



					}


					if (1==1) {


					//justo despues de EI no debe generar interrupcion
					//e interrupcion nmi tiene prioridad

						/*if (interrupcion_maskable_generada.v && byte_leido_core_ql!=251) {
						debug_anota_retorno_step_maskable();
						//Tratar interrupciones maskable


          }*/
				    }


            }

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
