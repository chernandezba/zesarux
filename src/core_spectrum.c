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
   ZX Spectrum core
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
#include "chardetect.h"
#include "diviface.h"
#include "timex.h"
#include "zxuno.h"
#include "prism.h"
#include "snap_rzx.h"
#include "superupgrade.h"
#include "pd765.h"
#include "tsconf.h"
#include "tbblue.h"

#include "scrstdout.h"
#include "settings.h"
#include "datagear.h"

#include "snap_zsf.h"
#include "zeng.h"
#include "ds1307.h"
#include "gs.h"
#include "snap_ram.h"
#include "codsinpr.h"
#include "menu_items.h"

z80_byte byte_leido_core_spectrum;


//int duracion_ultimo_opcode=0;


int disparada_int_pentagon=0;

int pentagon_inicio_interrupt=160;

//int tempcontafifty=0;
//int temp_xx_veces;

int testados_desde_inicio_pulso_interrupcion=0;

int si_siguiente_sonido(void)
{

	if (MACHINE_IS_PRISM) {
/*
Si son 525 scanlines:
525/5*3=315

0/1.6=0 mod 5=0
1/1.6=0 mod 5=1
2/1.6=1
3/1.6=1
4/1.6=2

5/1.6=3
6/1.6=3
7/1,6=4
8/1.6=4
9/1.6=5

10/1.6=6
Hacer mod 5 y escoger 0,2,4 (de cada 5 coger 3)
*/
		int resto=t_scanline%5;
		if (resto==0 || resto==2 || resto==4) return 1;
		else return 0;
	}

	return 1;
}


/*
void t_scanline_next_border(void)
{
        //resetear buffer border

	int i;
        //en principio inicializamos el primer valor con el ultimo out del border
        buffer_border[0]=out_254 & 7;

	//Siguientes valores inicializamos a 255
        for (i=1;i<BORDER_ARRAY_LENGTH;i++) buffer_border[i]=255;


}
*/

void t_scanline_next_fullborder(void)
{
    //resetear buffer border

    int i;

    //No si esta desactivado en tbblue
    if (MACHINE_IS_TBBLUE && tbblue_store_scanlines_border.v==0) return;

    //a 255
    //for (i=0;i<CURRENT_FULLBORDER_ARRAY_LENGTH;i++) fullbuffer_border[i]=255;
    //mas rapido con memset
    memset(fullbuffer_border,255,CURRENT_FULLBORDER_ARRAY_LENGTH);

	//Resetear buffer border para prism
	if (MACHINE_IS_PRISM) {
		for (i=0;i<CURRENT_PRISM_BORDER_BUFFER;i++) prism_ula2_border_colour_buffer[i]=65535;
	}

	//printf ("max buffer border : %d\n",i);


}


void interrupcion_si_despues_lda_ir(void)
{

	//Esto es diferente en NMOS y en CMOS
    if (MACHINE_IS_TSCONF) return;                //CMOS

	//NMOS
	//printf ("leido %d en interrupt\n",byte_leido_core_spectrum);
	if (byte_leido_core_spectrum==237) {
		//printf ("leido 237 en interrupt, siguiente=%d\n",pref237_opcode_leido);
		if (pref237_opcode_leido==87 || pref237_opcode_leido==95) {
			//printf ("Poner PV a 0 despues de LD a,i o LD a,r\n");
			Z80_FLAGS &=(255-FLAG_PV);
		}
	}
}






void core_spectrum_store_rainbow_current_atributes(void)
{

	//No hacer esto en tbblue
	if (MACHINE_IS_TBBLUE && tbblue_store_scanlines.v==0) return;	

	//En maquina prism, no hacer esto
	if (MACHINE_IS_PRISM) return;

	//En maquina tsconf, no hacer esto tampoco
	if (MACHINE_IS_TSCONF) return;


	//Si no vamos a refrescar pantalla (framedrop), no tiene sentido almacenar nada en el buffer
/*
Sin saltar frame aqui, tenemos por ejemplo
./zesarux --realvideo --frameskip 4  --vo null --exit-after 10
15 % cpu
tiempo de proceso en 10 segundos: user	0m1.498s

Saltando frame aqui,
12% cpu
tiempo de proceso en 10 segundos: user	0m1.239s
*/
    if (next_frame_skip_render_scanlines) {
        //if ((t_estados/screen_testados_linea)>310) printf ("-Not storing rainbow buffer as framescreen_saltar is %d or manual frameskip\n",framescreen_saltar);
        //if ((temp_xx_veces % 50)==0 && ((t_estados % screen_testados_linea)>1640)) printf("Skipping core_spectrum_store_rainbow_current_atributes due to framedrop. scanline %d\n",t_scanline_draw);

        //printf ("-Not storing rainbow buffer as framescreen_saltar is %d or manual frameskip\n",framescreen_saltar);
        return;
    }

    //ULA dibujo de pantalla
    //last_x_atributo guarda ultima posicion (+1) antes de ejecutar el opcode
    //lo que se pretende hacer aqui es guardar los atributos donde esta leyendo la ula actualmente,
    //y desde esta lectura a la siguiente, dado que cada opcode del z80 puede tardar mas de 4 ciclos (en 4 ciclos se generan 8 pixeles)
    //Esto no es exactamente perfecto,
    //lo ideal es que cada vez que avanzase el contador de t_estados se guardase el atributo que se va a leer
    //como esto es muy costoso, hacemos que se guardan los atributos leidos desde el opcode anterior hasta este
    if (rainbow_enabled.v) {
        if (t_scanline_draw>=screen_indice_inicio_pant && t_scanline_draw<screen_indice_fin_pant) {
            int atributo_pos=(t_estados % screen_testados_linea)/4;
            z80_byte *screen_atributo=get_base_mem_pantalla_attributes();
            z80_byte *screen_pixel=get_base_mem_pantalla();

            //printf ("atributos: %p pixeles: %p\n",screen_atributo,screen_pixel);

            int dir_atributo;

            if (timex_si_modo_8x1()) {
                dir_atributo=screen_addr_table[(t_scanline_draw-screen_indice_inicio_pant)*32];
            }

            else {
                dir_atributo=(t_scanline_draw-screen_indice_inicio_pant)/8;
                dir_atributo *=32;
            }


            int dir_pixel=screen_addr_table[(t_scanline_draw-screen_indice_inicio_pant)*32];

            //si hay cambio de linea, empezamos con el 0
            //puede parecer que al cambiar de linea nos perdemos el ultimo atributo de pantalla hasta el primero de la linea
            //siguiente, pero eso es imposible, dado que eso sucede desde el ciclo 128 hasta el 228 (zona de borde)
            // y no hay ningun opcode que tarde 100 ciclos
            if (last_x_atributo>atributo_pos) last_x_atributo=0;
                dir_atributo += last_x_atributo;
                dir_pixel +=last_x_atributo;

            //Puntero al buffer de atributos
            z80_byte *puntero_buffer;
            puntero_buffer=&scanline_buffer[last_x_atributo*2];

            for (;last_x_atributo<=atributo_pos;last_x_atributo++) {
                //printf ("last_x_atributo: %d atributo_pos: %d\n",last_x_atributo,atributo_pos);
                last_ula_pixel=screen_pixel[dir_pixel];
                last_ula_attribute=screen_atributo[dir_atributo];

                //realmente en este array guardamos tambien atributo cuando estamos en la zona de border,
                //nos da igual, lo hacemos por no complicarlo
                //debemos tambien suponer que atributo_pos nunca sera mayor o igual que ATRIBUTOS_ARRAY_LENGTH
                //esto debe ser asi dado que atributo_pos=(t_estados % screen_testados_linea)/4;

                *puntero_buffer++=last_ula_pixel;
                *puntero_buffer++=last_ula_attribute;
                dir_atributo++;
                dir_pixel++;
            }

            //Siguiente posicion se queda en last_x_atributo

            //printf ("fin lectura attr linea %d\n",t_scanline_draw);
        }

        //Para el bus idle le decimos que estamos en zona de border superior o inferior y por tanto retornamos 255
        else {
            last_ula_attribute=get_ula_databus_value();
            last_ula_pixel=get_ula_databus_value();
        }
    }
}

void core_spectrum_fin_frame_pantalla(void)
{
	//Siguiente frame de pantalla
    timer_get_elapsed_core_frame_post();



    if (MACHINE_IS_TBBLUE) {
        if (tbblue_force_disable_cooper.v==0) tbblue_copper_handle_vsync();
    }


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
        audio_send_stereo_sample(audio_valor_enviar_sonido_izquierdo,audio_valor_enviar_sonido_derecho);
        //audio_send_mono_sample(audio_valor_enviar_sonido_izquierdo);
                            linea_estados++;
    }

    if (hilow_convert_audio_thread_running) {
        menu_hilow_convert_get_audio_buffer();
    }


    t_estados -=screen_testados_total;

    //Para paperboy, thelosttapesofalbion0 y otros que hacen letras en el border, para que no se desplacen en diagonal
    //t_estados=0;
    //->paperboy queda fijo. thelosttapesofalbion0 no se desplaza, sino que tiembla si no forzamos esto

    audio_tone_generator_last=-audio_tone_generator_last;


    //Final de instrucciones ejecutadas en un frame de pantalla
    if (iff1.v==1) {
        interrupcion_maskable_generada.v=1;

        testados_desde_inicio_pulso_interrupcion=t_estados;

        //En Timex, ver bit 6 de puerto FF
        if ( MACHINE_IS_TIMEX_TS_TC_2068 && ( timex_port_ff & 64) ) interrupcion_maskable_generada.v=0;

        //En ZXuno, ver bit disvint
        if (MACHINE_IS_ZXUNO || MACHINE_IS_TBBLUE) {

            if (get_zxuno_tbblue_rasterctrl() & 4) {
                //interrupciones normales deshabilitadas
                //printf ("interrupciones normales deshabilitadas\n");
                //Pero siempre que no se haya disparado una maskable generada por raster

                if (zxuno_tbblue_disparada_raster.v==0) {
                    //printf ("interrupciones normales deshabilitadas y no raster disparada\n");
                    interrupcion_maskable_generada.v=0;
                }
            }
        }

        //TSConf lo gestiona mediante interrupciones de frame
        if (MACHINE_IS_TSCONF) interrupcion_maskable_generada.v=0;


        //Si la anterior instruccion ha tardado 32 ciclos o mas
        /*if (duracion_ultimo_opcode>=cpu_duracion_pulso_interrupcion) {
            debug_printf (VERBOSE_PARANOID,"Losing last interrupt because last opcode lasts 32 t-states or more: %d tstates: %d ",duracion_ultimo_opcode,t_estados);
            interrupcion_maskable_generada.v=0;
        }*/
        

        //en el Spectrum la INT comienza en el scanline 248, 0T
        //Pero en Pentagon la interrupción debe dispararse en el scanline 239 (contando desde 0), y 320 pixel clocks (o 160 T estados) tras comenzar dicho scanline
        //La generamos en pentagon desde otro sitio del bucle
        if (MACHINE_IS_PENTAGON) interrupcion_maskable_generada.v=0;


    }

    //Final de frame. Permitir de nuevo interrupciones pentagon
    disparada_int_pentagon=0;				

    cpu_loop_refresca_pantalla();
    //temp_xx_veces++;

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

void core_spectrum_fin_scanline(void)
{
//printf ("%d\n",t_estados);
			//if (t_estados>69000) printf ("t_scanline casi final: %d\n",t_scanline);

    if (si_siguiente_sonido() ) {

        //audio_valor_enviar_sonido=0;

        audio_valor_enviar_sonido_izquierdo=audio_valor_enviar_sonido_derecho=0;

        audio_valor_enviar_sonido_izquierdo +=da_output_ay_izquierdo();
        audio_valor_enviar_sonido_derecho +=da_output_ay_derecho();


        if (beeper_enabled.v) {
            if (beeper_real_enabled==0) {
                audio_valor_enviar_sonido_izquierdo += value_beeper;
                audio_valor_enviar_sonido_derecho += value_beeper;
            }

            else {
                char suma_beeper=get_value_beeper_sum_array();
                audio_valor_enviar_sonido_izquierdo += suma_beeper;
                audio_valor_enviar_sonido_derecho += suma_beeper;
                beeper_new_line();
            }

            
        }

        if (audiodac_enabled.v) {
            audiodac_mix();
        }

        if (gs_enabled.v) {
            gs_mix_dac_channels();
        }


        if (realtape_inserted.v && realtape_playing.v) {
            realtape_get_byte();
            if (realtape_loading_sound.v) {
                audio_valor_enviar_sonido_izquierdo /=2;
                audio_valor_enviar_sonido_izquierdo += realtape_last_value/2;

                audio_valor_enviar_sonido_derecho /=2;
                audio_valor_enviar_sonido_derecho += realtape_last_value/2;

                //Sonido alterado cuando top speed
                if (timer_condicion_top_speed() ) {
                    audio_valor_enviar_sonido_izquierdo=audio_change_top_speed_sound(audio_valor_enviar_sonido_izquierdo);
                    audio_valor_enviar_sonido_derecho=audio_change_top_speed_sound(audio_valor_enviar_sonido_derecho);
                }
            }
        }

        //Ajustar volumen
        if (audiovolume!=100) {
            audio_valor_enviar_sonido_izquierdo=audio_adjust_volume(audio_valor_enviar_sonido_izquierdo);
            audio_valor_enviar_sonido_derecho=audio_adjust_volume(audio_valor_enviar_sonido_derecho);
        }

        //if (audio_valor_enviar_sonido>127 || audio_valor_enviar_sonido<-128) printf ("Error audio value: %d\n",audio_valor_enviar_sonido);


        //Enviar sample de sonido
        if (audio_tone_generator) {
            audio_send_mono_sample(audio_tone_generator_get() );
        }

        else {
            //Pero si tenemos conversion hilow en curso, escuchar eso precisamente
            if (hilow_convert_audio_thread_running) {
                //no poner nada y luego aplicamos el buffer entero cuando haya que enviarlo
                //audio_send_stereo_sample(menu_hilow_convert_audio_last_audio_sample,menu_hilow_convert_audio_last_audio_sample);

                //audio_send_stereo_sample(temp_vv,temp_vv);
                //temp_vv =-temp_vv;
            }

            else audio_send_stereo_sample(audio_valor_enviar_sonido_izquierdo,audio_valor_enviar_sonido_derecho);
        }


        ay_chip_siguiente_ciclo();

        if (MACHINE_IS_TSCONF) {
            tsconf_handle_line_interrupts();

            //y reseteo de esto, que es para interrupciones frame
            tsconf_handle_frame_interrupts_prev_horiz=9999; 
        }

    }

    //final de linea

    //copiamos contenido linea y border a buffer rainbow
    if (rainbow_enabled.v==1) {

        if (next_frame_skip_render_scanlines) {
            //Cuando en el frame anterior se ha hecho skip, en el siguiente lo que hacemos es no renderizar scanlines
            //if ((t_estados/screen_testados_linea)>319) printf ("-Not storing rainbow buffer as framescreen_saltar is %d or manual frameskip\n",framescreen_saltar);
            //if ((temp_xx_veces % 25)==0) printf("Skipping scanline due to framedrop. scanline %d\n",t_estados/screen_testados_linea);
        }

        else {
            //if ((t_estados/screen_testados_linea)>319) printf ("storing rainbow buffer\n");
            TIMESENSOR_ENTRY_PRE(TIMESENSOR_ID_core_spectrum_store_scanline_rainbow);
            screen_store_scanline_rainbow_solo_border();
            screen_store_scanline_rainbow_solo_display();
            TIMESENSOR_ENTRY_POST(TIMESENSOR_ID_core_spectrum_store_scanline_rainbow);
            //if ((temp_xx_veces % 25)==0) printf("NOT Skipping scanline due to framedrop. scanline %d\n",t_estados/screen_testados_linea);
        }

        //t_scanline_next_border();

    }

    //General sound. Ejecutar los opcodes de un frame entero
    if (gs_enabled.v) {
        gs_fetch_opcodes_scanlines();
    }

    TIMESENSOR_ENTRY_PRE(TIMESENSOR_ID_core_spectrum_t_scanline_next_line);
    t_scanline_next_line();
    TIMESENSOR_ENTRY_POST(TIMESENSOR_ID_core_spectrum_t_scanline_next_line);


    //se supone que hemos ejecutado todas las instrucciones posibles de toda la pantalla. refrescar pantalla y
    //esperar para ver si se ha generado una interrupcion 1/50

    if (t_estados>=screen_testados_total) {
        TIMESENSOR_ENTRY_PRE(TIMESENSOR_ID_core_spectrum_fin_frame_pantalla);
        core_spectrum_fin_frame_pantalla();

        //General sound. Avisar de cambio de frame de pantalla
        if (gs_enabled.v) {
            gs_new_video_frame();
        }

        TIMESENSOR_ENTRY_POST(TIMESENSOR_ID_core_spectrum_fin_frame_pantalla);
    } 
    //Fin bloque final de pantalla



}

void core_spectrum_handle_interrupts(void)
{
    debug_fired_interrupt=1;

    z80_adjust_flags_interrupt_block_opcode();

    //ver si esta en HALT
    if (z80_halt_signal.v) {
        z80_halt_signal.v=0;
    }

    //ver si estaba en halt el copper
    //if (MACHINE_IS_TBBLUE) tbblue_if_copper_halt();

    
    

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

        //printf ("generada nmi pc=%04XH\n",reg_pc);

        //temp

        t_estados -=15;

        if (superupgrade_enabled.v) {
            //Saltar a NMI de ROM0. TODO: que pasa con puertos 32765 y 8189?
            superupgrade_puerto_43b = 0;
            puerto_32765=0;
            puerto_8189=0;
            superupgrade_set_memory_pages();
        }

        
        //Al recibir nmi tiene que poner paginacion normal. Luego ya saltara por autotrap de diviface
        if (diviface_enabled.v) {
            //diviface_paginacion_manual_activa.v=0;
            diviface_control_register&=(255-128);
            diviface_paginacion_automatica_activa.v=0;
        }

        generate_nmi_prepare_fetch();


    }

                
								

    //Si el pulso de interrupcion ya ha pasado
    //Pero si no tenemos dma activa, pues la dma esta modificando t_estados, y en una operacion de lectura/escritura larga,
    //este contador testados_desde_inicio_pulso_interrupcion puede tener un valor muy alto
    if (testados_desde_inicio_pulso_interrupcion>=cpu_duracion_pulso_interrupcion) {
        int hay_dma=0;

        //Soporte DMA ZXUNO
        if (MACHINE_IS_ZXUNO && zxuno_dma_disabled.v==0) hay_dma=1;

        //Soporte Datagear/TBBlue DMA
        if (datagear_dma_emulation.v && datagear_dma_is_disabled.v==0) hay_dma=1;

        if (MACHINE_IS_TSCONF) hay_dma=1;

        if (!hay_dma) {
            //printf("interrupt timeout. t-states since interrupt triggered: %d\n",testados_desde_inicio_pulso_interrupcion);
            interrupcion_maskable_generada.v=0;
        }
    }



    //justo despues de EI no debe generar interrupcion (incluso aunque antes del EI ya estuvieran habilitadas las interrupciones)
    //tampoco se puede generar en medio de un refetch de prefijo DD o FD
    //e interrupcion nmi tiene prioridad
    if (interrupcion_maskable_generada.v && byte_leido_core_spectrum!=251 && !core_refetch) {

        //printf ("Lanzada interrupcion spectrum normal\n");

        debug_anota_retorno_step_maskable();
        //Tratar interrupciones maskable
        interrupcion_maskable_generada.v=0;

        interrupcion_si_despues_lda_ir();

        //Aunque parece que rzx deberia saltar aqui al siguiente frame, lo hacemos solo cuando es necesario (cuando las lecturas en un frame exceden el frame)
        //if (rzx_reproduciendo) {
            //rzx_next_frame_recording();
        //}

        
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


void core_spectrum_handle_interrupts_pentagon(void)
{
    if (!disparada_int_pentagon) {
        
        int linea=t_estados/screen_testados_linea;
        if (linea==319) {
            //en el Spectrum la INT comienza en el scanline 248, 0T
            //Pero en Pentagon la interrupción debe dispararse en el scanline 239 (contando desde 0), y 320 pixel clocks (o 160 T estados) tras comenzar dicho scanline
            //A los 160 estados
            int t_est_linea=t_estados % screen_testados_linea;
            if (t_est_linea>=pentagon_inicio_interrupt) {
                //printf ("Int Pentagon\n");
                //printf ("scanline %d t_estados %d\n",t_estados/screen_testados_linea,t_estados);			

                disparada_int_pentagon=1;
                if (iff1.v==1) {
                    //printf ("Generated pentagon interrupt\n");
                    //printf ("scanline %d t_estados %d\n",t_estados/screen_testados_linea,t_estados);
                    interrupcion_maskable_generada.v=1;		

                    testados_desde_inicio_pulso_interrupcion=0;


                    //Si la anterior instruccion ha tardado 32 ciclos o mas
                    /*if (duracion_ultimo_opcode>=cpu_duracion_pulso_interrupcion) {
                        debug_printf (VERBOSE_PARANOID,"Losing last interrupt because last opcode lasts 32 t-states or more");
                        interrupcion_maskable_generada.v=0;
                    }*/
                }
            }

        }
    }
}

void core_spectrum_ciclo_fetch(void)
{

	TIMESENSOR_ENTRY_PRE(TIMESENSOR_ID_core_spectrum_store_rainbow_current_atributes);
	core_spectrum_store_rainbow_current_atributes();
	TIMESENSOR_ENTRY_POST(TIMESENSOR_ID_core_spectrum_store_rainbow_current_atributes);



#ifdef DEBUG_SECOND_TRAP_STDOUT

        //Para poder debugar rutina que imprima texto. Util para aventuras conversacionales
        //hay que definir este DEBUG_SECOND_TRAP_STDOUT manualmente en compileoptions.h despues de ejecutar el configure

	scr_stdout_debug_print_char_routine();

#endif

    if (MACHINE_IS_TSCONF) tsconf_handle_frame_interrupts();

    if (nmi_pending_pre_opcode) {
            //Dado que esto se activa despues de lanzar nmi y antes de leer opcode, aqui saltara cuando PC=66H
            //debug_printf (VERBOSE_DEBUG,"Handling nmi mapping pre opcode fetch at %04XH",reg_pc);
            nmi_handle_pending_prepost_fetch();
    }				


    int t_estados_antes_opcode=t_estados;
    core_refetch=0;

    //Modo normal
    if (diviface_enabled.v==0) {

                        contend_read( reg_pc, 4 );
        byte_leido_core_spectrum=fetch_opcode();



    }


    //Modo con diviface activado
    else {
        diviface_pre_opcode_fetch();
        contend_read( reg_pc, 4 );
        byte_leido_core_spectrum=fetch_opcode();
        diviface_post_opcode_fetch();
    }




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


    //Nota: agregar estos dos if de nmi_pending_pre_opcode y nmi_pending_post_opcode 
    //supone un 0.2 % de uso mas en mi iMac: pasa de usar 5.4% cpu a 5.6% cpu en --vo null y --ao null
    //Es muy poco...
    if (nmi_pending_post_opcode) {
        //Dado que esto se activa despues de lanzar nmi y leer opcode, aqui saltara cuando PC=67H
        //debug_printf (VERBOSE_DEBUG,"Handling nmi mapping post opcode fetch at %04XH",reg_pc);
        nmi_handle_pending_prepost_fetch(); 
    }				

    reg_r++;

    rzx_in_fetch_counter_til_next_int_counter++;


#ifdef EMULATE_SCF_CCF_UNDOC_FLAGS	
    //Guardar antes F
    scf_ccf_undoc_flags_before=Z80_FLAGS;
#endif

    z80_no_ejecutado_block_opcodes();
    codsinpr[byte_leido_core_spectrum]  () ;


#ifdef EMULATE_SCF_CCF_UNDOC_FLAGS	
    //Para saber si se ha modificado
    scf_ccf_undoc_flags_after_changed=(Z80_FLAGS  == scf_ccf_undoc_flags_before ? 0 : 1);
#endif				

    int delta_t_estados=t_estados-t_estados_antes_opcode;

    //if (!core_refetch) duracion_ultimo_opcode=delta_t_estados;
    //else duracion_ultimo_opcode +=delta_t_estados;

    testados_desde_inicio_pulso_interrupcion +=delta_t_estados;

    /*if (rzx_reproduciendo && rzx_in_fetch_counter_til_next_int) {
        if (rzx_in_fetch_counter_til_next_int_counter>=rzx_in_fetch_counter_til_next_int) {
            //Forzar final de frame
            //t_estados=screen_testados_total;
            printf ("Forzar final de frame\n");
            rzx_next_frame_recording();
        }
    }*/


    //Soporte interrupciones raster zxuno
    if (MACHINE_IS_ZXUNO || MACHINE_IS_TBBLUE) zxuno_tbblue_handle_raster_interrupts();

    //Soporte DMA ZXUNO
    if (MACHINE_IS_ZXUNO && zxuno_dma_disabled.v==0) zxuno_handle_dma();

    //Soporte Datagear/TBBlue DMA
    if (datagear_dma_emulation.v && datagear_dma_is_disabled.v==0) datagear_handle_dma(); 

    //Soporte TBBlue copper y otras...
    if (MACHINE_IS_TBBLUE) {
        //Si esta activo copper
        if (tbblue_force_disable_cooper.v==0) tbblue_copper_handle_next_opcode();


        if (tbblue_use_rtc_traps) {
            //Reloj RTC
            if (reg_pc==0x27a9 || reg_pc==0x27aa) {
            /*
                27A9 C9     RET
                27AA 37     SCF
                27AB C9     RET
            */						
                if (
                    peek_byte_no_time(reg_pc)==0xC9 &&
                    peek_byte_no_time(reg_pc+1)==0x37 &&
                    peek_byte_no_time(reg_pc+2)==0xC9 
                )
                tbblue_trap_return_rtc();
            }

        }

    
    }

}

//bucle principal de ejecucion de la cpu de spectrum
void cpu_core_loop_spectrum(void)
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
    if (plus3dos_traps.v) traps_plus3dos();


    //Gestionar autoload
    gestionar_autoload_spectrum();


    if (tap_load_detect()) {
        //si estamos en pausa, no hacer nada
        if (!tape_pause) {
            audio_playing.v=0;

            draw_tape_text();

            tap_load();
            all_interlace_scr_refresca_pantalla();

            //audio_playing.v=1;
            timer_reset();
        }

        else {
            core_spectrum_store_rainbow_current_atributes();
            //generamos nada. como si fuera un NOP
            contend_read( reg_pc, 4 );

        }
    }

    else if (tap_save_detect()) {
        audio_playing.v=0;

        draw_tape_text();

        tap_save();
        //audio_playing.v=1;
        timer_reset();
    }


    else {
        if (esperando_tiempo_final_t_estados.v==0) {
            TIMESENSOR_ENTRY_PRE(TIMESENSOR_ID_core_spectrum_ciclo_fetch);
            //printf("antes ciclo fetch PC=%XH\n",reg_pc);
            core_spectrum_ciclo_fetch();
            //printf("despues ciclo fetch PC=%XH\n",reg_pc);
            TIMESENSOR_ENTRY_POST(TIMESENSOR_ID_core_spectrum_ciclo_fetch);
        }

        //else {
        //    printf("esperando final t estados PC=%XH\n",reg_pc);
        //}

    }



    //En pentagon, disparar interrupcion antes del final de frame
    if (MACHINE_IS_PENTAGON) {
        core_spectrum_handle_interrupts_pentagon();
    }


    //A final de cada scanline 
    if ( (t_estados/screen_testados_linea)>t_scanline  ) {
        TIMESENSOR_ENTRY_PRE(TIMESENSOR_ID_core_spectrum_fin_scanline);
        core_spectrum_fin_scanline();			
        TIMESENSOR_ENTRY_POST(TIMESENSOR_ID_core_spectrum_fin_scanline);
    }
    

    //Ya hemos leido duracion ultimo opcode. Resetearla a 0 si no hay que hacer refetch
    //if (!core_refetch) duracion_ultimo_opcode=0;		



    if (esperando_tiempo_final_t_estados.v) {
        timer_pause_waiting_end_frame();
    }



    //Interrupcion de 1/50s. mapa teclas activas y joystick
    if (interrupcion_fifty_generada.v) {
        interrupcion_fifty_generada.v=0;

        //y de momento actualizamos tablas de teclado segun tecla leida
        //printf ("Actualizamos tablas teclado %d pc=%XH\n", contador_segundo,reg_pc);
        TIMESENSOR_ENTRY_PRE(TIMESENSOR_ID_scr_actualiza_tablas_teclado);
        scr_actualiza_tablas_teclado();
        TIMESENSOR_ENTRY_POST(TIMESENSOR_ID_scr_actualiza_tablas_teclado);


        //lectura de joystick
        TIMESENSOR_ENTRY_PRE(TIMESENSOR_ID_realjoystick_main);
        realjoystick_main();
        TIMESENSOR_ENTRY_POST(TIMESENSOR_ID_realjoystick_main);



    }


    //Interrupcion de procesador y marca final de frame
    if (interrupcion_timer_generada.v) {
        //printf ("Generada interrupcion timer %d pc=%XH\n",contador_segundo,reg_pc);
        interrupcion_timer_generada.v=0;
        esperando_tiempo_final_t_estados.v=0;
        interlaced_numero_frame++;
        //printf ("%d\n",interlaced_numero_frame);

        //Para calcular lo que se tarda en ejecutar todo un frame
        timer_get_elapsed_core_frame_pre();


    }


    //Interrupcion de cpu. gestion im0/1/2. Esto se hace al final de cada frame en spectrum o al cambio de bit6 de R en zx80/81
    if (interrupcion_maskable_generada.v || interrupcion_non_maskable_generada.v) {
        TIMESENSOR_ENTRY_PRE(TIMESENSOR_ID_core_spectrum_handle_interrupts);
        //printf ("Generada interrupcion cpu %d pc=%XH\n",contador_segundo,reg_pc);
        core_spectrum_handle_interrupts();
        TIMESENSOR_ENTRY_POST(TIMESENSOR_ID_core_spectrum_handle_interrupts);
    }
    //Fin gestion interrupciones


    //Aplicar snapshot pendiente de ZRCP y ZENG envio snapshots. Despues de haber gestionado interrupciones
    if (core_end_frame_check_zrcp_zeng_snap.v) {
        core_end_frame_check_zrcp_zeng_snap.v=0;
        check_pending_zrcp_put_snapshot();
        zeng_send_snapshot_if_needed();			
    }



    debug_get_t_stados_parcial_post();

}
