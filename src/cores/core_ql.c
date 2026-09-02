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


//Según cálculos a ojo tendríamos: (y digo a ojo por la velocidad comparativa, viendo que vaya "mas o menos igual" que un QL real
//y basándonos en 312 scanlines:
//223 ciclos por scanline
//69582 ciclos en un frame de video -> aprox 3479100 Hz = 3.48 MHz
//El QL debido a:
//La penalización del bus de 8 bits del 68008.
//La contención de la RAM interna con el ZX8301.
//Rinde aproximadamente el 41 % del rendimiento de un 68000 a 7.5 Mhz, o, inversamente, un 68000 a 7,5 MHz sería unas 2,43 veces más rápido.

/*
| Situación                                 | Equivalencia aproximada |
|---                                        |---:|
| Código con bastante cálculo interno       | 5–7 MHz de 68000 |
| Carga mixta típica                        | 4–5 MHz de 68000 |
| Código muy dependiente de RAM interna     | 3–4 MHz de 68000 |
| Tu estimación                             | ≈3,08 MHz, o 41 % |


Una equivalencia global más prudente podría estar alrededor de 4–4,6 MHz de 68000, dejando los 3 MHz como un caso particularmente castigado.

El calculo a ojo consiste en calcular cuantos frames de video tarda desde el arranque del QL hasta aparecer el menu,
esto son 175 frames

*/


//Contadores para estos cálculos temporales. No se usan realmente
static int ciclos_por_frame_calculados=0;
static int ciclos_por_scanline_calculados=0;



//bucle principal de ejecucion de la cpu de jupiter ace
void cpu_core_loop_ql(void)
{

    debug_get_t_stados_parcial_pre();


    timer_check_interrupt();


    if (chardetect_detect_char_enabled.v) chardetect_detect_char();
    if (chardetect_printchar_enabled.v) chardetect_printchar();


    ql_rom_traps();



    if (esperando_tiempo_final_t_estados.v==0) {



#ifdef EMULATE_CPU_STATS
        util_stats_increment_counter(stats_codsinpr,byte_leido_core_ql);
#endif




	//Ejecutar opcode
#ifdef EMULATE_VISUALMEM
        set_visualmemopcodebuffer(get_pc_register() % (ql_mem_limit+1) ); //Le hago el modulo porque a veces se sale de limite
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

        //Le pedimos 1 ciclo y el contador final nos indica cuanto ha tardado
        int ciclos_ultimo_opcode=1-m68k_cycles_remaining();
        //printf("Ciclos ultima instruccion: %d\n",ciclos_ultimo_opcode);

        ciclos_por_frame_calculados +=ciclos_ultimo_opcode;
        ciclos_por_scanline_calculados +=ciclos_ultimo_opcode;


        //old
        //t_estados +=4;

        //Simplemente incrementamos los t-estados un valor inventado, aunque luego al final parece ser parecido a la realidad
        t_estados+=ciclos_ultimo_opcode;


    }




    //Esto representa final de scanline

    //normalmente
    if ( (t_estados/screen_testados_linea)>t_scanline  ) {

        //printf("Ciclos por scanline: %d\n",ciclos_por_scanline_calculados);
        ciclos_por_scanline_calculados=0;

        t_scanline++;

        //Envio sonido

        audio_valor_enviar_sonido_izquierdo=audio_valor_enviar_sonido_derecho=0;

        audio_valor_enviar_sonido_izquierdo +=da_output_ay_izquierdo();
        audio_valor_enviar_sonido_derecho +=da_output_ay_derecho();

        if (audio_nagra_effect.v) {
            audio_apply_nagra_effect();
            audio_apply_nagra_effect_next();
        }

        audio_valor_enviar_sonido_izquierdo +=ql_audio_da_output();
        audio_valor_enviar_sonido_derecho +=ql_audio_da_output();


        //Ajustar volumen
        if (audiovolume!=100) {
            audio_valor_enviar_sonido_izquierdo=audio_adjust_volume(audio_valor_enviar_sonido_izquierdo);
            audio_valor_enviar_sonido_derecho=audio_adjust_volume(audio_valor_enviar_sonido_derecho);
        }


        audio_send_stereo_sample(audio_valor_enviar_sonido_izquierdo,audio_valor_enviar_sonido_derecho);


        ql_audio_next_cycle();

        ay_chip_siguiente_ciclo();

        ql_qimi_handle_irq(t_scanline);

        //se supone que hemos ejecutado todas las instrucciones posibles de toda la pantalla. refrescar pantalla y
        //esperar para ver si se ha generado una interrupcion 1/50

        //Final de frame

        if (t_estados>=screen_testados_total) {
            //printf("Ciclos total en un frame: %d Total en 1 segundo: %d\n",ciclos_por_frame_calculados,ciclos_por_frame_calculados*50);
            //printf("Total frames: %d\n",ql_total_frames);
            ql_total_frames++;
            ciclos_por_frame_calculados=0;

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



            //SYNC/frame. hace parpadear pantalla
            ql_pc_intr |=8;

            //No estoy seguro si esto son las interrupciones que genera el timer o no
            //Esto acaba generando llamadas a leer PC_INTR		Interrupt register
            m68k_set_irq(2);


            //ql_qimi_handle_irq();

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

        scr_actualiza_tablas_teclado();

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

    //Aplicar snapshot pendiente de ZRCP y ZENG envio snapshots. Despues de haber gestionado interrupciones
    if (core_end_frame_check_zrcp_zeng_snap.v) {
        core_end_frame_check_zrcp_zeng_snap.v=0;
        check_pending_zrcp_put_snapshot();
        zeng_send_snapshot_if_needed();

        zeng_online_client_end_frame_from_core_functions();
    }


    debug_get_t_stados_parcial_post();


}
