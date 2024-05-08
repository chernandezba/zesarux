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


#include "core_pcw.h"
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

#include "scrstdout.h"
#include "pcw.h"
#include "settings.h"

#include "snap_zsf.h"
#include "zeng.h"
#include "snap_ram.h"
#include "pd765.h"

#include "zeng_online_client.h"

z80_byte byte_leido_core_pcw;

void core_pcw_final_frame(void)
{


    t_scanline=0;
    set_t_scanline_draw_zero();

    timer_get_elapsed_core_frame_post();

    //TODO: controlar si t_scanline_draw se va "por debajo" del borde inferior
    //tampoco deberia pasar nada porque al hacer render rainbow ya se controla que sea superior y en ese caso no renderiza nada
    //printf ("End video frame en pcw_scanline_counter: %d t: %d scanline_draw: %d\n",pcw_scanline_counter,t_estados,t_scanline_draw);


    //Aqui no se deberia resetear, solo cuando hay vsync, pero algo hay erroneo en mi codigo que si no pongo esto,
    //hay "parpadeos" de cambio de modo en prince of persia, ianna (en menus), dizzy 5 no va, etc
    //if (pcw_endframe_workaround.v) {
    //    t_scanline_draw=0;
    //}



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


void core_pcw_end_scanline_stuff(void)
{


    //audio_valor_enviar_sonido=0;

    audio_valor_enviar_sonido_izquierdo=audio_valor_enviar_sonido_derecho=0;

    //audio_valor_enviar_sonido +=da_output_ay();

    audio_valor_enviar_sonido_izquierdo +=da_output_ay_izquierdo();
    audio_valor_enviar_sonido_derecho +=da_output_ay_derecho();



    //TODO real beeper
    if (beeper_enabled.v) {
        //if (beeper_real_enabled==0) {
            audio_valor_enviar_sonido_izquierdo += value_beeper;
            audio_valor_enviar_sonido_derecho += value_beeper;
        //}

        /*else {
            char suma_beeper=get_value_beeper_sum_array();
            audio_valor_enviar_sonido_izquierdo += suma_beeper;
            audio_valor_enviar_sonido_derecho += suma_beeper;
            beeper_new_line();
        }*/


    }

    //Ajustar volumen
    if (audiovolume!=100) {
        audio_valor_enviar_sonido_izquierdo=audio_adjust_volume(audio_valor_enviar_sonido_izquierdo);
        audio_valor_enviar_sonido_derecho=audio_adjust_volume(audio_valor_enviar_sonido_derecho);
    }

    if (audio_tone_generator) {
        audio_send_mono_sample(audio_tone_generator_get() );
    }

    else {
        audio_send_stereo_sample(audio_valor_enviar_sonido_izquierdo,audio_valor_enviar_sonido_derecho);
    }

    ay_chip_siguiente_ciclo();


    //printf("Llega Info %d t: %d pcw_crtc_contador_scanline %d t_scanline_draw %d\n",
    //    pcw_scanline_counter,t_estados,pcw_crtc_contador_scanline,t_scanline_draw);

    //final de linea
    //copiamos contenido linea y border a buffer rainbow

    /*
    if (rainbow_enabled.v==1) {
        //printf ("render core scanline draw: %d\n",t_scanline_draw);
        screen_store_scanline_rainbow_pcw_border_and_display();
    }
    */

    t_scanline_next_line();

    pcw_scanline_counter++;

    //pcw_handle_vsync_state();






    //pcw genera interrupciones a 300 hz
    //Esto supone lanzar 6  (50*6=300) interrupciones en cada frame
    //al final de un frame ya va una interrupcion
    //generar otras 5
    //tenemos unas 300 scanlines en cada pantalla
    //generamos otras 5 interrupciones en cada scanline: 50,100,150,200,250

    //Esto tiene que ir antes de pcw_handle_vsync_state
    //pcw_scanline_counter++;

    //printf ("crtc counter: %d t: %d scanline_draw: %d\n",pcw_scanline_counter,t_estados,t_scanline_draw);



    //Con ay player, interrupciones a 50 Hz


    if (pcw_scanline_counter>=52 && ay_player_playing.v==0) {
        pcw_pending_interrupt.v=1;

        //printf ("Llega interrupcion crtc del Z80 en counter: %d pcw_crtc_contador_scanline: %d t: %d scanline_draw: %d\n",
        //pcw_scanline_counter,pcw_crtc_contador_scanline,t_estados,t_scanline_draw);


        if (iff1.v==1) {
            //printf ("Llega interrupcion crtc con interrupciones habilitadas del Z80 en counter: %d t: %d t_scanline_draw %d\n",pcw_scanline_counter,t_estados,t_scanline_draw);

        }

        else {
            //printf ("Llega interrupcion crtc con interrupciones DESHABILITADAS del Z80 en counter: %d t: %d\n",pcw_scanline_counter,t_estados);
        }
        pcw_scanline_counter=0;

        pcw_increment_interrupt_counter();
    }


/*
    //Ver si resetear t_scanline_draw
    int final_pantalla=pcw_get_crtc_final_display_zone();
    //printf("final pantalla: %d\n",final_pantalla);
    if (t_scanline_draw>=final_pantalla) {
        //printf("reseteando t_scanline_draw en %d\n",t_scanline_draw);
        set_t_scanline_draw_zero();
        //pcw_crtc_contador_scanline=0;
    }
*/

    //se supone que hemos ejecutado todas las instrucciones posibles de toda la pantalla. refrescar pantalla y
    //esperar para ver si se ha generado una interrupcion 1/50

    if (t_estados>=screen_testados_total) {
        core_pcw_final_frame();
    }

    //Fin final de frame



}



void core_pcw_handle_interrupts(void)
{

    debug_fired_interrupt=1;

    z80_adjust_flags_interrupt_block_opcode();



    //if (interrupts.v==1) {   //esto ya no se mira. si se ha producido interrupcion es porque estaba en ei o es una NMI
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

        //temp

        t_estados -=15;



    }



    //justo despues de EI no debe generar interrupcion
    //e interrupcion nmi tiene prioridad
    if (interrupcion_maskable_generada.v && byte_leido_core_pcw!=251) {
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



//bucle principal de ejecucion de la cpu de pcw
void cpu_core_loop_pcw(void)
{

    debug_get_t_stados_parcial_pre();


    timer_check_interrupt();


//#ifdef COMPILE_STDOUT
//		if (screen_stdout_driver) scr_stdout_printchar();
//#endif
//
//#ifdef COMPILE_SIMPLETEXT
//                if (screen_simpletext_driver) scr_simpletext_printchar();
//#endif
    if (chardetect_detect_char_enabled.v) chardetect_detect_char();
    if (chardetect_printchar_enabled.v) chardetect_printchar();





    if (esperando_tiempo_final_t_estados.v==0) {

        //Eventos de la controladora de disco
        pd765_next_event_from_core();

        //Eventos de boot disco, volver a disco anterior cuando se haya iniciado CP/M
        pcw_handle_end_boot_disk();

        pcw_boot_check_dsk_not_bootable();

#ifdef DEBUG_SECOND_TRAP_STDOUT

    //Para poder debugar rutina que imprima texto. Util para aventuras conversacionales
    //hay que definir este DEBUG_SECOND_TRAP_STDOUT manualmente en compileoptions.h despues de ejecutar el configure

        scr_stdout_debug_print_char_routine();

#endif


        contend_read( reg_pc, 4 );
        byte_leido_core_pcw=fetch_opcode();



#ifdef EMULATE_CPU_STATS
        util_stats_increment_counter(stats_codsinpr,byte_leido_core_pcw);
#endif

        //Si la cpu está detenida por señal HALT, reemplazar opcode por NOP
        if (z80_halt_signal.v) {
            byte_leido_core_pcw=0;
        }
        else {
            reg_pc++;
        }

        reg_r++;

        z80_no_ejecutado_block_opcodes();
        codsinpr[byte_leido_core_pcw]  () ;


    }




    //A final de cada scanline
    if ( (t_estados/screen_testados_linea)>t_scanline  ) {

        core_pcw_end_scanline_stuff();

    }



    if (esperando_tiempo_final_t_estados.v) {
        timer_pause_waiting_end_frame();
    }



    //Interrupcion de 1/50s. mapa teclas activas y joystick
    if (interrupcion_fifty_generada.v) {
        interrupcion_fifty_generada.v=0;

        //y de momento actualizamos tablas de teclado segun tecla leida
        scr_actualiza_tablas_teclado();

        //TODO: no estoy seguro cuando hay que ejecutar esto
        pcw_keyboard_ticker_update();


        //lectura de joystick
        realjoystick_main();

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



    //Si habia interrupcion pendiente  y están las interrupciones habilitadas

    if (pcw_pending_interrupt.v && iff1.v==1) {


        pcw_pending_interrupt.v=0;

        interrupcion_maskable_generada.v=1;


        pcw_scanline_counter=0;

        //printf("Generada interrupcion maskable y atendida en core\n");
        //sleep(2);

    }

    //TODO si habia interrupcion pendiente y no se atiende por estar en DI, la perdemos??
    //if (pcw_pending_interrupt.v) pcw_pending_interrupt.v=0;

    //Interrupcion de cpu. gestion im0/1/2. Esto se hace al final de cada frame en pcw o al cambio de bit6 de R en zx80/81
    if (interrupcion_maskable_generada.v || interrupcion_non_maskable_generada.v) {
        core_pcw_handle_interrupts();

    }
	//Fin gestion interrupciones

	//Aplicar snapshot pendiente de ZRCP y ZENG envio snapshots. Despues de haber gestionado interrupciones
	if (core_end_frame_check_zrcp_zeng_snap.v) {
		core_end_frame_check_zrcp_zeng_snap.v=0;
		check_pending_zrcp_put_snapshot();
		zeng_send_snapshot_if_needed();

        zeng_online_client_end_frame_from_core_functions();
	}

	debug_get_t_stados_parcial_post();

}




