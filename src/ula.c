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
#include <string.h>


#include "ula.h"
#include "cpu.h"
#include "debug.h"
#include "screen.h"
#include "contend.h"
#include "zxvision.h"
#include "multiface.h"
#include "betadisk.h"
#include "settings.h"
#include "divmmc.h"
#include "hilow_datadrive.h"
#include "samram.h"
#include "hilow_barbanegra.h"
#include "transtape.h"
#include "mhpokeador.h"
#include "specmate.h"
#include "phoenix.h"
#include "defcon.h"
#include "ramjet.h"


//#define ZESARUX_ZXI_PORT_REGISTER 0xCF3B
//#define ZESARIX_ZXI_PORT_DATA     0xDF3B

z80_byte zesarux_zxi_last_register=0;

z80_byte zesarux_zxi_registers_array[256];

//valor que la ula deja en el bus de datos. habitualmente 255 (o sea todo a alta impedancia)
z80_byte ula_databus_value=255;

//ultimo valor enviado al border, tal cual
z80_byte out_254_original_value;

//ultimo valor enviado al border, teniendo en cuenta mascara de inves
z80_byte out_254;

//a 1 indica teclado issue2 (bit 6 a 1). Sino, issue 3
z80_bit keyboard_issue2;

//ultimo atributo leido por la ULA
z80_byte last_ula_attribute=255;
//ultimo byte de pixel leido por la ULA
z80_byte last_ula_pixel=255;


z80_byte contador_parpadeo=16;
z80_bit estado_parpadeo;

//No permitir cambio parpadeo
z80_bit disable_change_flash={0};


z80_bit ula_late_timings={0};

z80_bit ula_im2_slow={0};

z80_bit pentagon_timing={0};

//Puerto de Pentagon y ZX Evo
//Bit 0: activar modo 16C
//Bit XXX: Control de acceso a celdas nvram
z80_byte puerto_eff7=0;


//Si se pulsan mas de dos teclas en diferentes columnas, en spectrum, se leen mas teclas.
//El tipico caps+b+v representa caps+b+v+space
z80_bit keyboard_matrix_error={0};


//Poder desactivar paginado de rom y ram
z80_bit ula_disabled_ram_paging={0};
z80_bit ula_disabled_rom_paging={0};


z80_bit recreated_zx_keyboard_support={0};

z80_bit recreated_zx_keyboard_pressed_caps={0};


int nmi_pending_pre_opcode=0;
int nmi_pending_post_opcode=0;

//Dinamic SD1. Alterar bit 5 a 0. Realmente pone cualquier puerto que no modifique explicitamente este bit, a 0
//A la práctica se usa con Camelot Warriors y el puerto FEH
z80_bit dinamic_sd1={0};

void ula_pentagon_timing_common(void)
{

        //Recalcular algunos valores cacheados
        recalcular_get_total_ancho_rainbow();
        recalcular_get_total_alto_rainbow();

        screen_set_video_params_indices();
        inicializa_tabla_contend();

        init_rainbow();
        init_cache_putpixel();

	//Parchecillo temporal, dado que el footer se desplaza una linea de caracteres hacia abajo al activar pentagon
	menu_init_footer();

}

//Activa timings de pentagon pero NO activa contended memory. Se hace para compatibilidad con funciones de zuxno
void ula_disable_pentagon_timing(void)
{
                pentagon_timing.v=0;

                set_machine_params();

	ula_pentagon_timing_common();
}


//Activa timings de pentagon pero NO desactiva contended memory. Se hace para compatibilidad con funciones de zuxno
void ula_enable_pentagon_timing_no_common(void)
{
	pentagon_timing.v=1;

                        screen_invisible_borde_superior=16;
                        screen_borde_superior=64;
                        screen_total_borde_inferior=48;

                        //los timings son realmente estos pero entonces necesitariamos mas tamanyo de ventana de ancho
                        /*screen_total_borde_izquierdo=64;
                        screen_total_borde_derecho=64;
                        screen_invisible_borde_derecho=64;*/

                        //dejamos estos que es el tamanyo normal
                        screen_total_borde_izquierdo=48;
                        screen_total_borde_derecho=48;
                        screen_invisible_borde_derecho=96;

                        screen_testados_linea=224;


}


void ula_enable_pentagon_timing(void)
{

	ula_enable_pentagon_timing_no_common();
	 ula_pentagon_timing_common();
}


z80_byte zesarux_zxi_read_last_register(void)
{
  return zesarux_zxi_last_register;
}

void zesarux_zxi_write_last_register(z80_byte value)
{
  zesarux_zxi_last_register=value;
}

void zesarux_zxi_write_register_value(z80_byte value)
{

  switch (zesarux_zxi_last_register) {
    case 0:
      //Bit 0.
      if (MACHINE_IS_INVES) {  //Solo lanzamos linea de debug, no hacemos accion, ese bit se leerá donde corresponda de la funcion peek de inves
        if ((value &1)==1) {
			    debug_printf (VERBOSE_DEBUG,"Show Inves Low RAM");
		    }
		    else {
			    debug_printf (VERBOSE_DEBUG,"Hide Inves Low RAM (normal situation)");
		    }
      }

    break;

    case 1:
      //HARDWARE_DEBUG_ASCII
        printf ("%c",(value>=32 && value<=127 ? value : '?')  );
        fflush(stdout);
    break;

    case 2:
      //HARDWARE_DEBUG_NUMBER
        printf ("%d",value);
			  fflush(stdout);
    break;

    case 3:
	/*
* Reg 3: ZEsarUX control register
Bit 0: Set to 1 to exit emulator
Bit 1-7: Unused


	*/
	if (value&1) {
		debug_printf(VERBOSE_INFO,"Exiting emulator because of a ZEsarUX ZXI port exit emulator operation");
		end_emulator_autosave_snapshot();
	}
    break;

    case 6:
      //HARDWARE_DEBUG_BYTE_FILE
        if (zesarux_zxi_hardware_debug_file[0]==0) debug_printf(VERBOSE_ERR,"HARDWARE_DEBUG_BYTE_FILE unconfigured");
        else {
            FILE *ptr_destino;
            ptr_destino=fopen(zesarux_zxi_hardware_debug_file,"ab");

            if (ptr_destino==NULL) {
                debug_printf (VERBOSE_ERR,"Error opening HARDWARE_DEBUG_BYTE_FILE");
                return;
            }

            //Escribir 1 byte
            fwrite(&value,1,1,ptr_destino);
            fclose(ptr_destino);

        }
    break;

  }

  zesarux_zxi_registers_array[zesarux_zxi_last_register]=value;

}


z80_byte zesarux_zxi_read_string_register(char *s,z80_byte register_number)
{
	z80_byte register_value;
	int longitud_string;

	//Si indice esta fuera de rango de string (posicion longitud+1) resetear a 0
	//Indice siempre apunta a siguiente posicion a leer
	register_value=zesarux_zxi_registers_array[register_number]; 

	longitud_string=strlen(s);  
	//Ejemplo String longitud 3: ABC. Ultima posicion valida: 3 (caracter 0)
	//Si se ha retornado toda la string, tendra valor posicion 4
	//Esto tambien evita que el usuario meta el indice en posicion mayor a la longitud de la string

	if (register_value>longitud_string) register_value=0;

	//Caracter a retornar
	z80_byte caracter_retorno=s[register_value];

	//Aumentamos indice
	register_value++;

	//Guardamos indice
	zesarux_zxi_registers_array[register_number]=register_value;

	return caracter_retorno;
}

z80_byte zesarux_zxi_read_register_value(void)
{

/*
* Reg 4: ZEsarUX version number

Used to get ZEsarUX version number string. You must write first to this register (with value 0) to reset index string to position 0 (you could even write any value to change the index)
Then read this register to get the string, every read will get a character (the index is incremented every read), finishing the string with character 0. When it reaches the end, the index string is reset to the beginning.
Index is reset to 0 every reset

* Reg 5: ZEsarUX build number

Used to get ZEsarUX build number string. Same behaviour as "Reg 4: ZEsarUX version number": write value 0 here to reset string index.
Every read will get a character, finishing the string with character 0. When it reaches the end, the index string is reset to t
he beginning.
Index is reset to 0 every reset
*/


//EMULATOR_VERSION
//BUILDNUMBER


	//Casos especiales
	switch (zesarux_zxi_last_register) {
		case 4:
			return zesarux_zxi_read_string_register(EMULATOR_VERSION,4);
		break;

		case 5:
			return zesarux_zxi_read_string_register(BUILDNUMBER,5);
		break;

	}


  return zesarux_zxi_registers_array[zesarux_zxi_last_register];
}

void nmi_handle_pending_prepost_fetch(void)
{

    nmi_pending_pre_opcode=0;
    nmi_pending_post_opcode=0;

    if (multiface_enabled.v) {
		multiface_map_memory();
        multiface_lockout=0;
	}

    if (betadisk_enabled.v) {
        betadisk_active.v=1;
    }

    if (hilow_enabled.v) {
        hilow_nmi();
    }

    if (hilow_bbn_enabled.v) {
        hilow_bbn_nmi();
    }

    if (transtape_enabled.v) {
        transtape_nmi();
    }  

    if (mhpokeador_enabled.v) {
        mhpokeador_nmi();
    }  

    if (specmate_enabled.v) {
        specmate_nmi();
    }

    if (phoenix_enabled.v) {
        phoenix_nmi();
    }

    if (defcon_enabled.v) {
        defcon_nmi();
    }

    if (ramjet_enabled.v) {
        ramjet_nmi();
    }                    

}

void generate_nmi(void)
{
	interrupcion_non_maskable_generada.v=1;
    //nmi_pending_post_opcode=1;

    if (samram_enabled.v) {
        samram_nmi();
    }

}

void generate_nmi_multiface_tbblue(void)
{
    //hacer que no salte mapeo de divmmc
    //if (divmmc_diviface_enabled.v) divmmc_diviface_disable();

	interrupcion_non_maskable_generada.v=1;

   
}

/*
void old_generate_nmi_prepare_fetch(void)
{
    //Vamos a suponer que lo normal es que salte en 66h, o sea, con pre_opcode

    nmi_pending_pre_opcode=1;
    
    if (MACHINE_IS_TBBLUE && multiface_enabled.v && multiface_type==MULTIFACE_TYPE_THREE) {
        //Pero en tbblue, salta con post. Entonces no se esta comportando como un mf3 realmente
        nmi_pending_post_opcode=1;
        nmi_pending_pre_opcode=0;
    }

}
*/

void generate_nmi_prepare_fetch(void)
{
    //Vamos a suponer que lo normal es que salte en 67h, o sea, con post_opcode

    nmi_pending_post_opcode=1;
    
    if (!MACHINE_IS_TBBLUE && multiface_enabled.v && multiface_type==MULTIFACE_TYPE_THREE) {
        //Pero en mf3 (no en tbblue), salta con pre
        nmi_pending_pre_opcode=1;
        nmi_pending_post_opcode=0;
    }

    //prueba betadisk. todo indica por la rom que hace pre, aunque luego igualmente al lanzar la nmi, peta
    if (betadisk_enabled.v) {
        nmi_pending_pre_opcode=1;
        nmi_pending_post_opcode=0;        
    }

    //hilow. todo indica que lo hace con pre
    if (hilow_enabled.v) {
        nmi_pending_pre_opcode=1;
        nmi_pending_post_opcode=0;        
    }


    if (hilow_bbn_enabled.v) {
        nmi_pending_pre_opcode=1;
        nmi_pending_post_opcode=0;
    }

    if (transtape_enabled.v) {
        nmi_pending_pre_opcode=1;
        nmi_pending_post_opcode=0;
    } 

    if (mhpokeador_enabled.v) {
        nmi_pending_pre_opcode=1;
        nmi_pending_post_opcode=0;
    }       

    if (specmate_enabled.v) {
        nmi_pending_pre_opcode=1;
        nmi_pending_post_opcode=0;
    }

    if (phoenix_enabled.v) {
        nmi_pending_pre_opcode=1;
        nmi_pending_post_opcode=0;
    }

    if (defcon_enabled.v) {
        nmi_pending_pre_opcode=1;
        nmi_pending_post_opcode=0;
    }

    if (ramjet_enabled.v) {
        nmi_pending_pre_opcode=1;
        nmi_pending_post_opcode=0;
    }                  

}

/*
void old_old_generate_nmi_prepare_fetch(void)
{
    nmi_pending_post_opcode=1;

    if (multiface_enabled.v && multiface_type==MULTIFACE_TYPE_THREE) {
        nmi_pending_post_opcode=0;
        nmi_pending_pre_opcode=1;
    }

    //Betadisk tambien hace en pre??
    if (betadisk_enabled.v) {
        nmi_pending_post_opcode=0;
        nmi_pending_pre_opcode=1;        
    }
}
*/

//Convertir tecla leida del recreated en tecla real y en si es un press (1) o un release(0)
/*
http://zedcode.blogspot.com.es/2016/07/notes-on-recreated-zx-spectrum.html
*/

char recreated_key_table_minus[]="1234567890qwe";
char recreated_key_table_mayus[]="rtyuiopasdfgh";


void recreated_zx_spectrum_keyboard_convert(int tecla, enum util_teclas *tecla_final, int *pressrelease)
{
/*
Key    Push+Release
1    ab
2    cd
3    ef
4    gh
5    ij
6    kl
7    mn
8    op
9    qr
0    st
Q    uv
W    wx
E    yz
R    AB
T    CD
Y    EF
U    GH
I    IJ
O    KL
P    MN
A    OP
S    QR
D    ST
F    UV
G    WX
H    YZ
J    01
K    23
L    45
ENTER    67
CAP SHIFT    89
Z    <>   (es ,. pero con mayusculas)
X    -=
C    []
V    ;:
B    ,.
N    /?  (? es / con mayusculas)
M    {}   (Es [] pero con mayusculas)       See note [6]
SYMBOL SHIFT    !$   (es 14 pero con mayusculas)     See Note [6]
BREAK SPACE        %^ (es 56 pero con mayusculas)


So when key 1 is pressed, we get an ‘a’ and when released we get a ‘b’.

Que pasa con zxcvbnm symbol y space? Parece que generan diferentes pulsaciones segun el keyboard mapping del pc
En caso de cocoa, leo el teclado en modo raw y no deberia afectar. 
En framebuffer, tampoco deberia afectar
En XWindow, sí que afecta la localizacion. soluciones: leer en raw? O usar el keymapping setting que uso para Z88 por ejemplo?
En SDL también le afecta la localización

*/

/*
NOTA: Esta funcion es sensible los valores de teclas. Si algun "UTIL_KEY_**" de estos llega a tener valor>31, puede que al redefinir
una tecla (ascii>=32) llegue a pensar que se trata de algun simbolo UTIL_KEY
Por lo que hay que tener cuidado de no usar aqui valores de UTIL_KEY que sean mayores de 31
*/

    //printf ("convirtiendo tecla %d (\"%c\") shift=%d de recreated\n",tecla,tecla,recreated_zx_keyboard_pressed_caps.v);

    if (recreated_zx_keyboard_pressed_caps.v) {
        if (tecla>='a' && tecla<='z')  tecla-=32;
        else {
            //Otros cambios con mayusculas
            switch (tecla)
            {
                case ',':
                    tecla='<';
                break;

                case '.':
                    tecla='>';
                break;

                case '/':
                    tecla='?';
                break;

                case '[':
                    tecla='{';
                break;

                case ']':
                    tecla='}';
                break;

                case ';':
                    tecla=':';
                break;

/*
SYMBOL SHIFT    !$   (es 14 pero con mayusculas)     See Note [6]
BREAK SPACE        %^ (es 56 pero con mayusculas)
*/
                case '1':
                    tecla='!';
                break;

                case '4':
                    tecla='$';
                break;

                case '5':
                    tecla='%';
                break;

                case '6':
                    tecla='^';
                break;

            }
        }
    }

    //Desde la a-z y A-Z tenemos una tabla
    //char recreated_key_table_minus[]="1234567890QWE";
    //char recreated_key_table_mayus[]="RTYUIOPASDFGH";
    if (tecla>='a' && tecla<='z') {
        tecla -='a'; 
        //Par es press, impar es release
        if (tecla&1) *pressrelease=0;
        else *pressrelease=1;

        tecla /=2;
        //retornar tecla
        *tecla_final=recreated_key_table_minus[tecla];
        return;
    }

    if (tecla>='A' && tecla<='Z') {
        tecla -='A'; 
        //Par es press, impar es release
        if (tecla&1) *pressrelease=0;
        else *pressrelease=1;

        tecla /=2;
        //retornar tecla
        *tecla_final=recreated_key_table_mayus[tecla];
        return;
    }

    //Resto de teclas
    switch (tecla) {
        case '0':
            *pressrelease=1;
            *tecla_final='j';
        break;

        case '1':
            *pressrelease=0;
            *tecla_final='j';
        break;

        case '2':
            *pressrelease=1;
            *tecla_final='k';
        break;

        case '3':
            *pressrelease=0;
            *tecla_final='k';
        break;

        case '4':
            *pressrelease=1;
            *tecla_final='l';
        break;

        case '5':
            *pressrelease=0;
            *tecla_final='l';
        break;

        case '6':
            *pressrelease=1;
            *tecla_final=UTIL_KEY_ENTER;
        break;

        case '7':
            *pressrelease=0;
            *tecla_final=UTIL_KEY_ENTER;
        break;

        case '8':
            //printf ("Pulsada 8\n");
            *pressrelease=1;
            *tecla_final=UTIL_KEY_CAPS_SHIFT;  //Para caps shift spectrum
        break;

        case '9':
            *pressrelease=0;
            *tecla_final=UTIL_KEY_CAPS_SHIFT;
        break;

        /*
        Z    <>
X    -=
C    []
V    ;:

        */

        case '<':
            *pressrelease=1;
            *tecla_final='z';
        break;

        case '>':
            *pressrelease=0;
            *tecla_final='z';
        break;


        case '-':
            *pressrelease=1;
            *tecla_final='x';
        break;

        case '=':
            *pressrelease=0;
            *tecla_final='x';
        break;


        case '[':
            *pressrelease=1;
            *tecla_final='c';
        break;

        case ']':
            *pressrelease=0;
            *tecla_final='c';
        break;

        case ';':
            *pressrelease=1;
            *tecla_final='v';
        break;

        case ':':
            *pressrelease=0;
            *tecla_final='v';
        break;

/*
B    ,.
N    /?
M    {}            See note [6]


*/

        case ',':
            *pressrelease=1;
            *tecla_final='b';
        break;

        case '.':
            *pressrelease=0;
            *tecla_final='b';
        break;

        case '/':
            *pressrelease=1;
            *tecla_final='n';
        break;

        case '?':
            *pressrelease=0;
            *tecla_final='n';
        break;

        case '{':
            *pressrelease=1;
            *tecla_final='m';
        break;

        case '}':
            *pressrelease=0;
            *tecla_final='m';
        break;
/*
SYMBOL SHIFT    !$        See Note [6]
BREAK SPACE        %^
*/

        case '!':
            *pressrelease=1;
            *tecla_final=UTIL_KEY_CONTROL_L;
        break;

        case '$':
            *pressrelease=0;
            *tecla_final=UTIL_KEY_CONTROL_L;
        break;

        case '%':
            *pressrelease=1;
            *tecla_final=32;
        break;

        case '^':
            *pressrelease=0;
            *tecla_final=32;
        break;        

        default:
            //Valores sin alterar
            //printf ("tecla sin alterar %c\n",tecla);
            *tecla_final=0;
        break;

    }

    //printf ("fin de funcion. tecla final: %c pressrelease: %d\n",*tecla_final,*pressrelease);


}


z80_byte get_ula_databus_value(void)
{

    z80_byte valor=ula_databus_value;

    //Para funciones que leen directamente con esta funcion y no idle_bus_port_atribute,
    //tenemos que aplicar mascara SD1 si esta activo
    if (dinamic_sd1.v) valor=valor & (255-32);

    return valor;
}
