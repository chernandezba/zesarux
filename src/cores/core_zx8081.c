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
#include "zx8081.h"
#include "timer.h"
#include "zxvision.h"
#include "compileoptions.h"
#include "contend.h"
#include "snap_zx8081.h"
#include "utils.h"
#include "realjoystick.h"
#include "chardetect.h"
#include "ula.h"
#include "settings.h"

#include "snap_zsf.h"
#include "zeng.h"
#include "snap_ram.h"
#include "zeng_online_client.h"
#include "menu_items_storage.h"
#include "tv.h"


int core_zx8081_t_estados_antes=0;


z80_byte byte_leido_core_zx8081;

int core_zx8081_medio_scanline=0;

//la generada por cambio bit 6 registro R
z80_bit pendiente_maskable_generada={0};

//bucle principal de ejecucion de la cpu de zx80/81
void cpu_core_loop_zx8081(void)
{

    debug_get_t_stados_parcial_pre();

    timer_check_interrupt();


    if (chardetect_detect_char_enabled.v) chardetect_detect_char();
    if (chardetect_printchar_enabled.v) chardetect_printchar();



    //Autoload
    //Si hay cinta insertada
    if (  (tape_loadsave_inserted & TAPE_LOAD_INSERTED)!=0 || (realtape_inserted.v==1) ) {
        //Y si hay que hacer autoload
        if (initial_tap_load.v==1 && initial_tap_sequence==0) {

            //Para zx80
            if (MACHINE_IS_ZX80_TYPE && reg_pc==0x0283) {
                    debug_printf (VERBOSE_INFO,"Autoload tape");
                    initial_tap_sequence=1;
            }


            //Para zx81
            if (MACHINE_IS_ZX81_TYPE && reg_pc==0x0487) {
                    debug_printf (VERBOSE_INFO,"Autoload tape");
                    initial_tap_sequence=1;
            }
        }
    }


    //Interceptar rutina de carga

    if (new_tap_load_detect_zx8081()) {
        audio_playing.v=0;

        draw_tape_text();

        if (MACHINE_IS_ZX80_TYPE) new_tape_load_zx80();
        else new_tape_load_zx81();

        //audio_playing.v=1;
        timer_reset();

    }

    else if (new_tap_save_detect_zx8081()) {
        audio_playing.v=0;

        draw_tape_text();

        if (MACHINE_IS_ZX80_TYPE) new_tape_save_zx80();
        else new_tape_save_zx81();

        //audio_playing.v=1;
        timer_reset();

    }

    else {
        if (esperando_tiempo_final_t_estados.v==0) {

            if (video_zx8081_ula_video_output) zx80801_last_sprite_video=255;
            else zx80801_last_sprite_video=0;

            zx80801_last_sprite_video_tinta=0;
            zx80801_last_sprite_video_papel=15;

            byte_leido_core_zx8081=fetch_opcode();

            contend_read( reg_pc, 4 );

#ifdef EMULATE_CPU_STATS
            util_stats_increment_counter(stats_codsinpr,byte_leido_core_zx8081);
#endif



            //Si la cpu está detenida por señal HALT, reemplazar opcode por NOP
            if (z80_halt_signal.v) {
                byte_leido_core_zx8081=0;

                //Ese halt tiene que durar 1 t estado
                if (MACHINE_IS_ZX81_TYPE && nmi_generator_active.v/*&& interrupcion_non_maskable_generada.v*/) t_estados -=3;
            }
            else {
                reg_pc++;
            }

            reg_r_antes_zx8081=reg_r;

            reg_r++;

            z80_no_ejecutado_block_opcodes();
            codsinpr[byte_leido_core_zx8081]  () ;


            //calcular t_estados pasados desde ejecucion anterior
            int delta=0;
            if (t_estados<core_zx8081_t_estados_antes) {
                //dado la vuelta
                delta=(screen_testados_total-core_zx8081_t_estados_antes)+t_estados;
            }
            else {
                delta=t_estados-core_zx8081_t_estados_antes;
            }
            core_zx8081_t_estados_antes=t_estados;

            //TODO: al final ula_zx80_time_event y ula_zx81_time_event deberian ser lo mismo
            if (MACHINE_IS_ZX80_TYPE) {
                ula_zx80_time_event(delta);
            }
            if (MACHINE_IS_ZX81_TYPE) {
                ula_zx81_time_event(delta);
            }



    if (pendiente_maskable_generada.v) {
        pendiente_maskable_generada.v=0;
        interrupcion_maskable_generada.v=1;
    }


            if (iff1.v) {
                //solo cuando cambia de 1 a 0 bit 6 de R
                if ( (reg_r_antes_zx8081 & 64)==64 && (reg_r & 64)==0 ) {
                    //interrupcion_maskable_generada.v=1;
                    pendiente_maskable_generada.v=1;
                }
            }

        }

    }


    //A mitad de scanline
    //Hacemos cosas como leer sample de audio de cable externo, pues leemos a 31200 hz (el doble de lo que seria cada scanline)
    if (!core_zx8081_medio_scanline) {
        int estados_en_linea=t_estados % screen_testados_linea;
        if (estados_en_linea>screen_testados_linea/2) {
            //printf("mitad scanline. %5d %5d\n",estados_en_linea,t_estados);
            //Indicamos que ya hemos pasado el medio scanline
            core_zx8081_medio_scanline=1;
            if (audio_can_record_input()) {
                if (audio_is_recording_input) {
                    //En este caso simplemente leemos el valor que luego el core lo interpreta en el puerto EAR
                    //En cambio no alimentamos con ese valor el buffer de sonido que permite escuchar el sonido de cable externo,
                    //no hace falta complicarse tanto
                    //digamos que de esos 31200 hz, 1 de cada dos samples no lo escuchamos, aunque por el puerto EAR se interpretan los dos
                    audio_read_sample_audio_input();
                    realtape_last_value=audio_last_record_input_sample;
                    //return;
                }
            }
        }
    }


    //Esto representa final de scanline

    //normalmente
    if ( (t_estados/screen_testados_linea)>t_scanline  ) {

        //Indicamos que no hemos pasado el medio scanline
        core_zx8081_medio_scanline=0;

        t_scanline++;




        //Envio sonido
        audio_valor_enviar_sonido=0;
        audio_valor_enviar_sonido +=da_output_ay();

        if (zx8081_vsync_sound.v) {
            if (beeper_real_enabled==0) {
                    audio_valor_enviar_sonido += da_amplitud_speaker_zx8081();
            }

            else {
                    audio_valor_enviar_sonido += get_value_beeper_sum_array();
                    beeper_new_line();
            }
        }

        else {
            //Si no hay vsync sound, beeper sound off, forzamos que hay silencio de beepr
            beeper_silence_detection_counter=SILENCE_DETECTION_MAX;
        }


        int leer_cinta_real=0;

        if (realtape_inserted.v && realtape_playing.v) leer_cinta_real=1;

        if (audio_can_record_input()) {
            if (audio_is_recording_input) {
                leer_cinta_real=1;
            }
        }


        if (leer_cinta_real) {
            realtape_get_byte();
            //audio_valor_enviar_sonido += get_realtape_last_value();
            if (realtape_loading_sound.v) {
                reset_silence_detection_counter();
                audio_valor_enviar_sonido /=2;
                audio_valor_enviar_sonido += get_realtape_last_value()/2;
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

            //Siguiente frame de pantalla
            //Aunque el ZX80/81 tienen su propio sistema para dibujar la pantalla por tanto
            //en este momento no implica ni vsync ni nada parecido
            timer_get_elapsed_core_frame_post();



            t_scanline=0;

            //Parche para maquinas que no generan 312 lineas, porque si enviamos menos sonido se escuchara un click al final
            //Es necesario que cada frame de pantalla contenga 312 bytes de sonido
            //Igualmente en la rutina de envio_audio se vuelve a comprobar que todo el sonido a enviar
            //este completo; esto es necesario para Z88

            int linea_estados=t_estados/screen_testados_linea;

            while (linea_estados<312) {
                audio_send_mono_sample(audio_valor_enviar_sonido);
                linea_estados++;
            }

            if (convert_audio_to_zx81_thread_running) {
                menu_convert_audio_to_zx81_get_audio_buffer();
            }


            t_estados -=screen_testados_total;



            cpu_loop_refresca_pantalla();

            vofile_send_frame(rainbow_buffer);

            siguiente_frame_pantalla();


            if (debug_registers) scr_debug_registers();

            //Hacer esto aun en esta maquina porque ese parpadeo tambien se usa en menu (cursor parpadeante)
            contador_parpadeo--;

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

            core_end_frame_check_zrcp_zeng_snap.v=1;

            //snapshot en ram
            snapshot_add_in_ram();

            //para el detector de vsync sound
            if (zx8081_detect_vsync_sound.v) {
                //printf("counter: %d\n",zx8081_detect_vsync_sound_counter);

                //0,1,2 frames con vsync fuera de sitio no se consideran sonido, para evitar falsos positivos
                if (zx8081_detect_vsync_sound_counter<=2) {
                    //caso normal con vsync
                    //hay vsync el suficiente tiempo. desactivar sonido
                    //printf ("desactivar sonido\n");
                    zx8081_vsync_sound.v=0;

                }

                else {
                    //no hay vsync. por tanto hay sonido
                    //printf ("activar sonido. contador: %d\n",zx8081_detect_vsync_sound_counter);
                    zx8081_vsync_sound.v=1;

                }


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
        scr_actualiza_tablas_teclado();


        //lectura de joystick
        realjoystick_main();

    }


    //Interrupcion de procesador y marca final de frame
    if (interrupcion_timer_generada.v) {
        interrupcion_timer_generada.v=0;
        esperando_tiempo_final_t_estados.v=0;
        interlaced_numero_frame++;

        //Para calcular lo que se tarda en ejecutar todo un frame
        timer_get_elapsed_core_frame_pre();

    }


    //Interrupcion de cpu. gestion im0/1/2. Esto se hace al cambio de bit6 de R en zx80/81
    if (interrupcion_maskable_generada.v || interrupcion_non_maskable_generada.v) {
        //printf("Generada maskable o nmi\n");

        debug_fired_interrupt=1;

        z80_adjust_flags_interrupt_block_opcode();

        //ver si esta en HALT
        if (z80_halt_signal.v) {
            z80_halt_signal.v=0;
        }



        if (interrupcion_non_maskable_generada.v) {
            // NMI starts with a dummy opcode fetch
            t_estados += 4;

            // sumar wait cycles (minimo 1, maximo 16) - waitmap

            //Esto influye en el modo slow, cuando se transiciona del borde superior, donde se usa nmi, a la zona de caracteres,
            //donde se usa interrupciones por registro R, ese halt (en la rom direccion 79H) permite sincronizar la nmi con el hsync,
            //o lo que es lo mismo, sincronizarse luego el hsync con la interrupción por registro R
            //int sumar_estados = get_waitmap_value(t_estados % WAITMAP_SIZE);

            //printf("sumar %d\n",sumar_estados);

            //TODO: De momento no sumamos waitmap, no funciona bien
            //t_estados +=sumar_estados;

            //esto se ha probado a base de prueba y error para que los modos fast y slow queden centrados igual
            //si no, modos fast y slow difieren en 2 columnas de caracteres
            //TODO: lo ideal seria sumar el valor de waitmap
            t_estados +=9;


            debug_anota_retorno_step_nmi();
            interrupcion_non_maskable_generada.v=0;

            //6 T
            push_valor(reg_pc,PUSH_VALUE_TYPE_NON_MASKABLE_INTERRUPT);

            reg_r++;
            iff1.v=0;

            reg_pc= 0x66;

        }


        //justo despues de EI no debe generar interrupcion
        //e interrupcion nmi tiene prioridad
        if (interrupcion_maskable_generada.v && byte_leido_core_zx8081!=251) {

            if (MACHINE_IS_ZX81_TYPE) {
                //printf("hsync desde interrupt\n");
                //TODO: se deberia mantener durante 16 estados, pero hago 32 o si no, no se ve bien
                generate_zx81_delayed_hsync(32,16);

                //cuando se genera la delayed hsync se vuelve a resetear este contador, puede parecer redundante,
                //pero creo que de alguna manera, al resetearlo aquí, evitamos que se lance otro hsync derivado
                //del propio contador cuando llega a 207, así no se lanza dos veces seguidas
                ula_zx81_time_event_t_estados=0;
            }

            debug_anota_retorno_step_maskable();

            //Tratar interrupciones maskable

            interrupcion_maskable_generada.v=0;

            //+6 t-estados
            push_valor(reg_pc,PUSH_VALUE_TYPE_MASKABLE_INTERRUPT);


            reg_r++;

            //desactivar interrupciones al generar una
            iff1.v=iff2.v=0;

            //Prueba de ajuste para corregir problemas de video con la ASZMIC ROM
            //Actualmente el fallo con esa rom es:
            //Usandola combinada con la SPROM, en modo gráfico, la imagen suele temblar al dibujar en pantalla y/o leer del teclado
            //Este ajuste no lo corregia, por tanto lo desactivamos
            /*
            if (MACHINE_IS_ZX81_TYPE && aszmic_rom_video_adjust.v) {
                t_estados--;
            }
            */

            //IM0/1
            if (im_mode==0 || im_mode==1) {
                //+7 t-estados
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

            //ZX80 lanza hsync al hacer ack de interrupción
            if (MACHINE_IS_ZX80_TYPE) {
                generar_zx80_hsync();
                video_zx8081_lcntr++;
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


