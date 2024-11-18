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
#include "timer.h"
#include "zxvision.h"
#include "compileoptions.h"
#include "contend.h"
#include "ula.h"
#include "utils.h"
#include "realjoystick.h"
#include "z88.h"
#include "chardetect.h"

#include "scrstdout.h"
#include "settings.h"

#include "snap_zsf.h"
#include "zeng.h"
#include "zeng_online_client.h"

z80_byte byte_leido_core_z88;

int tempcontafifty=0;

//contador que se incrementa al final de cada frame (5 ms)
//cuando llega a 3 (20 ms) se refresca pantalla y lee teclado.. y otras cosas que suceden cada 20ms (como envio de sonido)
int z88_5ms_contador=0;



//int contador_sonido_3200hz=0;
int contador_z88_sonido_3200hz=0;

void z88_gestionar_tim(void)
{

	//Si esta en reset, no hacer nada
	if (blink_com&16) return;

	//printf ("z88_gestionar_tim\n");


	int signalTimeInterrupt=0;


                        //Gestion RTC.
                        z80_byte TIM0=blink_tim[0];
                        z80_byte TIM1=blink_tim[1];
			//Estos dos siguientes son int porque sumando aqui salen de rango 255
                        z80_int TIM2=blink_tim[2];
                        z80_int TIM3=blink_tim[3];
                        z80_byte TIM4=blink_tim[4];

                // fire a single interrupt for TSTA.TICK register,
                // but only if the flap is closed (the Blink doesn't emit
                // RTC interrupts while the flap is open, even if INT.TIME
                // is enabled)
                if ( (blink_sta & BM_STAFLAPOPEN) == 0 && ((blink_tmk & BM_TMKTICK) == BM_TMKTICK) && ((blink_tsta & BM_TSTATICK) == 0)) {
                    if (((blink_int & BM_INTTIME) == BM_INTTIME) & ((blink_int & BM_INTGINT) == BM_INTGINT)) {
                        // INT.TIME interrupts are enabled, and Blink may signal it as IM 1
                        // TMK.TICK interrupts are enabled, signal that a tick occurred
                        blink_tsta |= BM_TSTATICK;
                        blink_sta |= BM_STATIME;

                        signalTimeInterrupt = 1;
                    }
                }

                if (TIM0 > 199) {
                    // When this counter reaches 200, a second has passed... (200 * 5 ms ticks = 1 sec)
                    TIM0 = 0;
                } else {
                    TIM0++;
                }

                if (TIM0 == 0x80) {
                    // According to blink dump monitoring on Z88, when TIM0 reaches 0x80, a second interrupt
                    // is signaled
                    ++TIM1;

                    // but only if the flap is closed (the Blink doesn't emit RTC interrupts while the flap is open,
                    // even if INT.TIME is enabled)
                    if ((blink_sta & BM_STAFLAPOPEN) == 0 && (blink_tmk & BM_TMKSEC) == BM_TMKSEC && ((blink_tsta & BM_TSTASEC) == 0)) {
                        // TMK.SEC interrupts are enabled, signal that a second occurred only if it not already signaled

                        if (((blink_int & BM_INTTIME) == BM_INTTIME) & ((blink_int & BM_INTGINT) == BM_INTGINT)) {
                            // INT.TIME interrupts are enabled, and Blink may signal it as IM 1
                            blink_tsta |= BM_TSTASEC;
                            blink_sta |= BM_STATIME;

                            signalTimeInterrupt = 1;
                        }
                    }
                }

                if (TIM1 > 59) {
                    // 1 minute has passed
                    TIM1 = 0;

                    if ((blink_sta & BM_STAFLAPOPEN) == 0 && (blink_tmk & BM_TMKMIN) == BM_TMKMIN && ((blink_tsta & BM_TSTAMIN) == 0)) {
                        // TMK.MIN interrupts are enabled, signal that a minute occurred only if it not already signaled.
                        // but only if the flap is closed (the Blink doesn't emit RTC interrupts while the flap is open,
                        // even if INT.TIME is enabled)
                        if (((blink_int & BM_INTTIME) == BM_INTTIME) & ((blink_int & BM_INTGINT) == BM_INTGINT)) {
                            // INT.TIME interrupts are enabled, and Blink may signal it as IM 1
                            blink_tsta |= BM_TSTAMIN;
                            blink_sta |= BM_STATIME;

                            signalTimeInterrupt = 1;
                        }
                    }

                    if (++TIM2 > 255) {
                        TIM2 = 0; // 256 minutes has passed
                        if (++TIM3 > 255) {
                            TIM3 = 0; // 65536 minutes has passed
                            if (++TIM4 > 31) {
                                TIM4 = 0; // 65536 * 32 minutes has passed
                            }
                        }
                    }
                }



                        blink_tim[0]=TIM0;
                        blink_tim[1]=TIM1;
                        blink_tim[2]=TIM2;
                        blink_tim[3]=TIM3;
                        blink_tim[4]=TIM4;

			//temp
			//debug_printf (VERBOSE_PARANOID,"registros RTC TIM: %d %d %d %d %d",blink_tim[0],blink_tim[1],blink_tim[2],blink_tim[3],blink_tim[4]);


                if (signalTimeInterrupt) {
			//printf ("reloj. despertar de snooze y coma\n");
                    z88_awake_from_snooze();
                    z88_awake_from_coma();

                    // Signal INT interrupt to Z80...
                    interrupcion_maskable_generada.v=1;
                }


}


//gestionar INT o NMI
void z88_gestionar_interrupcion(void)
{

	debug_fired_interrupt=1;

    z80_adjust_flags_interrupt_block_opcode();


                        //ver si esta en HALT
                        if (z80_halt_signal.v) {
                                        z80_halt_signal.v=0;
                                        //reg_pc++;
					//z88_awake_from_coma();
                        //printf ("final de halt en refresca: %d max_cpu_cycles: %d\n",refresca,max_cpu_cycles);
                        }

			//z88_awake_from_snooze();


                         if (interrupcion_non_maskable_generada.v) {
						debug_anota_retorno_step_nmi();
                                                interrupcion_non_maskable_generada.v=0;
                                                //printf ("Calling NMI with pc=0x%x\n",reg_pc);
                                                //call_address(0x66);
						iff1.v=0;
                                                push_valor(reg_pc,PUSH_VALUE_TYPE_NON_MASKABLE_INTERRUPT);
                                                reg_pc=0x66;


                                        }

                        else {

			//si estamos en DI, volver


			if (iff1.v==0) {
				if (interrupcion_maskable_generada.v) interrupcion_maskable_generada.v=0;
				//printf ("generada interrupcion pero DI. volvemos sin hacer nada\n");
				return;
			}




                                        //justo despues de EI no debe generar interrupcion
                                        //e interrupcion nmi tiene prioridad
                               if (interrupcion_maskable_generada.v && byte_leido_core_z88!=251) {
						debug_anota_retorno_step_maskable();
                                                //Tratar interrupciones maskable
                                                interrupcion_maskable_generada.v=0;




                                                push_valor(reg_pc,PUSH_VALUE_TYPE_MASKABLE_INTERRUPT);

                                                reg_r++;



                                                //desactivar interrupciones al generar una
						iff1.v=iff2.v=0;

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
                                                        t_estados += 7;


                                                }
					}
			}


}


//char temp_valor_z88_speaker=255;

//bucle principal de ejecucion de la cpu de spectrum
void cpu_core_loop_z88(void)
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




		if (0==1) {
		}

		else {
			if (esperando_tiempo_final_t_estados.v==0) {


				if (z88_snooze.v==0) {
                                	contend_read( reg_pc, 4 );

					byte_leido_core_z88=fetch_opcode();

#ifdef EMULATE_CPU_STATS
					util_stats_increment_counter(stats_codsinpr,byte_leido_core_z88);
#endif

                    //Si la cpu está detenida por señal HALT, reemplazar opcode por NOP
                    if (z80_halt_signal.v) {
                        byte_leido_core_z88=0;
                    }
                    else {
                        reg_pc++;
                    }

					reg_r++;

                            z80_no_ejecutado_block_opcodes();
	                		codsinpr[byte_leido_core_z88]  () ;

					//printf ("t_estados: %d\n",t_estados);


				}

				else {
					//printf ("modo snooze reg_pc=%d\n",reg_pc);
					//si esta en modo snooze o coma, no hacer mucho
					//contend_read( reg_pc, 4 );
					//lee NOP
					byte_leido_core_z88=0;

					//simulamos que es un nop pero con t_estados muy largo, para liberar la cpu bastante
					t_estados +=Z88_T_ESTADOS_COMA_SNOOZE;


				}

                                //si en modo snooze o coma, leer teclado para notificar
                                if (z88_snooze.v || z88_coma.v) {

                                        if (z88_return_keyboard_port_value(0)!=255) {
                                                //printf ("notificar tecla\n");
                                                z88_notificar_tecla();

                                                //printf ("tecla kbd A10: %d A11: %d\n", blink_kbd_a10, blink_kbd_a11);
                                        }

                                }


				//Si en modo coma, alargar los t_estados del halt, para liberar la cpu bastante
				if (z88_snooze.v==0 && z88_coma.v==1 && z80_halt_signal.v) {
					t_estados +=Z88_T_ESTADOS_COMA_SNOOZE;
				}



                        }
                }



		//ejecutar esto al final de cada una de las scanlines (312)
		//esto implica que al final del frame de pantalla habremos enviado 312 bytes de sonido


		//A final de cada scanline
		if ( (t_estados/screen_testados_linea)>t_scanline  ) {
			//printf ("%d\n",t_estados);


				//Esto sucede con 15600hz
				contador_z88_sonido_3200hz +=3200;

				//si esta en la mitad del periodo, conmutar
				if (contador_z88_sonido_3200hz>=15600/2) {
					contador_z88_sonido_3200hz -=15600/2;
					valor_sonido_3200hz = -valor_sonido_3200hz;
				}



				audio_valor_enviar_sonido=0;

                        /*
7           SRUN        Speaker source (0=SBIT, 1=TxD or 3200hz)
6           SBIT        SRUN=0: 0=low, 1=high; SRUN=1: 0=3200 hz, 1=TxD
                        */

			  if (beeper_enabled.v) {

					z80_byte tipo_sonido=blink_com & (64+128);
					if (tipo_sonido>=128) {
					//Sonido 3200hz
        		if (tipo_sonido == 128) {
            	audio_valor_enviar_sonido = valor_sonido_3200hz;
            	//Sonido continuo 3200 hz. resetear contador silencio
          		reset_beeper_silence_detection_counter();
	        	}

						//Sonido de TxD. no implementado
						if (tipo_sonido == 128+64) {
						}

					}

					else {


						if (beeper_real_enabled==0) {
							audio_valor_enviar_sonido +=z88_get_beeper_sound();
						}
						else {
							audio_valor_enviar_sonido += get_value_beeper_sum_array();
		          beeper_new_line();
						}


					}

				}
	                        //Ajustar volumen
        	                if (audiovolume!=100) {
                	                audio_valor_enviar_sonido=audio_adjust_volume(audio_valor_enviar_sonido);
                        	}

                                audio_send_mono_sample(audio_valor_enviar_sonido);



			//}



			//final de linea

			t_scanline_next_line();



			//se supone que hemos ejecutado todas las instrucciones posibles de toda la pantalla. refrescar pantalla y
			//esperar para ver si se ha generado una interrupcion 1/50

                        if (t_estados>=screen_testados_total) {

				//printf ("total t_estados: %d\n",screen_testados_total);


		                 t_scanline=0;

                                 timer_get_elapsed_core_frame_post();

					set_t_scanline_draw_zero();


                                t_estados -=screen_testados_total;

				if (z88_5ms_contador==3) {
					//printf ("z88_5ms_contador=%d\n",z88_5ms_contador);

					cpu_loop_refresca_pantalla();

					vofile_send_frame(rainbow_buffer);

					siguiente_frame_pantalla();


					if (debug_registers) scr_debug_registers();

					//printf ("Parpadeo: %d estado: %d\n",contador_parpadeo,estado_parpadeo.v);
					//estado_parpadeo en z88 es para texto. a cada segundo cambia el estado
					//estado_parpadeo_cursor en z88 es para el cursor. 0.7 segundos invertido. 0.3 segundos normal

					contador_parpadeo--;
					if (!contador_parpadeo) {
						contador_parpadeo=50;
						toggle_flash_state();
						//printf ("cambio parpadeo. result: %d\n",estado_parpadeo.v);
					}

					//50*0.3=15
					if (contador_parpadeo>=15) estado_parpadeo_cursor.v=0;
					else estado_parpadeo_cursor.v=1;

					//printf ("contador: %d parpadeo_cursor: %d\n",contador_parpadeo,estado_parpadeo_cursor.v);
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


			}

		}

		if (esperando_tiempo_final_t_estados.v) {
			timer_pause_waiting_end_frame();
		}




              //Interrupcion de 1/200s. mapa teclas activas y joystick
                if (interrupcion_fifty_generada.v) {
                        interrupcion_fifty_generada.v=0;

			if (z88_5ms_contador==3) {
	                        //y de momento actualizamos tablas de teclado segun tecla leida
				//printf ("Actualizamos tablas teclado %d ", temp_veces_actualiza_teclas++);
                	       scr_actualiza_tablas_teclado();


	                       //lectura de joystick
        	               realjoystick_main();

			}

                        z88_5ms_contador++;
                        if (z88_5ms_contador==4) z88_5ms_contador=0;

                        //printf ("temp conta fifty: %d z88_5ms_contador: %d\n",tempcontafifty++,z88_5ms_contador);


			//Top speed no funciona bien en Z88. Si estaba activo, desactivarlo
			if (top_speed_timer.v) top_speed_timer.v=0;
                }


                //Interrupcion de procesador y marca final de frame
                if (interrupcion_timer_generada.v) {
                        interrupcion_timer_generada.v=0;
                        esperando_tiempo_final_t_estados.v=0;
                        interlaced_numero_frame++;
			z88_contador_para_flap++;
                        //printf ("%d\n",interlaced_numero_frame);
			z88_gestionar_tim();

			//printf ("registros RTC TIM: %d %d %d %d %d\n",blink_tim[0],blink_tim[1],blink_tim[2],blink_tim[3],blink_tim[4]);

			//Gestionar interrupciones al final del frame
			if (interrupcion_maskable_generada.v || interrupcion_non_maskable_generada.v) {
				z88_gestionar_interrupcion();
			}

            //Aplicar snapshot pendiente de ZRCP y ZENG envio snapshots. Despues de haber gestionado interrupciones
            if (core_end_frame_check_zrcp_zeng_snap.v) {
                core_end_frame_check_zrcp_zeng_snap.v=0;
                check_pending_zrcp_put_snapshot();
                zeng_send_snapshot_if_needed();

                zeng_online_client_end_frame_from_core_functions();

            }

                        //Para calcular lo que se tarda en ejecutar todo un frame
                        timer_get_elapsed_core_frame_pre();


                }

                debug_get_t_stados_parcial_post();

}
