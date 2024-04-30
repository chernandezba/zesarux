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
#include <stdlib.h>
#include <string.h>

#include "svi.h"
#include "vdp_9918a.h"
#include "cpu.h"
#include "debug.h"
#include "ay38912.h"
#include "tape.h"
#include "screen.h"
#include "audio.h"
#include "operaciones.h"
#include "joystick.h"
#include "settings.h"


// Info, soft etc:
// http://www.samdal.com/svsoftware.htm


z80_byte *svi_vram_memory=NULL;

z80_byte svi_ppi_register_a;
//bit 0-1:slot segmento 0 (0000h-3fffh)
//bit 2-3:slot segmento 1 (4000h-7fffh)
//bit 4-5:slot segmento 2 (8000h-bfffh)
//bit 6-7:slot segmento 3 (C000h-ffffh)


z80_byte svi_ppi_register_b;
z80_byte svi_ppi_register_c;


//Aunque solo son 10 filas, metemos array de 16 pues es el maximo valor de indice seleccionable por el PPI

z80_byte svi_keyboard_table[16]={
255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255
};



//slots asignados, y sus 4 segmentos
//tipos: rom, ram, vacio
//int svi_memory_slots[4][4];


z80_bit svi_cartridge_inserted={0};


const char *svi_string_memory_type_rom="ROM";
const char *svi_string_memory_type_ram="RAM";
const char *svi_string_memory_type_empty="EMPTY";

char *svi_get_string_memory_type(int tipo)
{


    switch (tipo) {

        case SVI_SLOT_MEMORY_TYPE_ROM:
            return (char *)svi_string_memory_type_rom;
        break;

        case SVI_SLOT_MEMORY_TYPE_RAM:
            return (char *)svi_string_memory_type_ram;
        break;

        default:
            return (char *)svi_string_memory_type_empty;
        break;

    }
}


int svi_return_offset_ram_page(int ram_number,z80_int direccion)
{

    //Total:  3 ROMS de 32 kb, 5 RAMS de 32 kb, en SVI328.
    //En 318, solo 1 pagina de 16 kb ram?

    int offset=32768*ram_number;

    if (MACHINE_IS_SVI_318) {
        offset=0; //solo una pagina de RAM, y de 16kb
        direccion &=16383;
        //TODO: parece que el basic muestra 30 kb aun con esto
    }

    //saltar las 3 roms
    offset +=3*32768;

    offset +=(direccion & 32767);

    //if (offset>=16384+3*32768)  printf ("offset: %d\n",offset-3*32768);

    return offset;
}

int svi_return_offset_rom_page(int rom_number,z80_int direccion)
{

    //Total:  3 ROMS de 32 kb, 5 RAMS de 32 kb, en SVI328

    return 32768*rom_number+(direccion & 32767);

}


//Retorna direccion de memoria donde esta mapeada la ram y su tipo
z80_byte *svi_return_segment_address(z80_int direccion,int *tipo)
{


    //TODO: si el usuario cambia el numero de chips ay a 2 o 3, esto puede fallar
    //o puede fallar al cargar snapshot zsf
    //Mapeo de memoria sale del registro 15 del ay chip (realmente es el port B del ay chip)
    z80_byte page_config=ay_3_8912_registros[ay_chip_selected][15];


    //temp_prin_page_config++; if ((temp_prin_page_config % 5000) ==0  ) printf ("page config: %02XH\n",page_config);


    int offset_segment_low=svi_return_offset_rom_page(0,direccion);
    int offset_segment_high=svi_return_offset_ram_page(0,direccion);

    int tipo_low=SVI_SLOT_MEMORY_TYPE_ROM;
    int tipo_high=SVI_SLOT_MEMORY_TYPE_RAM;


    if (page_config!=0xFF) {


        //Ver bits activos
        if ((page_config & 1)==0) {
            offset_segment_low=svi_return_offset_rom_page(1,direccion);
        }

        if ((page_config & 2)==0) {
            offset_segment_low=svi_return_offset_ram_page(1,direccion);
            tipo_low=SVI_SLOT_MEMORY_TYPE_RAM;
            //if ((temp_prin_page_config % 5000) ==0  ) printf ("Ram 1 en segmento bajo\n");
        }

        if ((page_config & 4)==0) {
            offset_segment_high=svi_return_offset_ram_page(2,direccion);
        }

        if ((page_config & 8)==0) {
            tipo_low=SVI_SLOT_MEMORY_TYPE_RAM;
            offset_segment_low=svi_return_offset_ram_page(3,direccion);
        }

        if ((page_config & 16)==0) {
            offset_segment_high=svi_return_offset_ram_page(4,direccion);
        }

        //TODO bits 5,6,7
        // 5: Cap Lamp (Keyboard Cap Lock)
        // 6: ROM 2 enable (1=Disable, 0=Enable)
        // 7: ROM 3 enable (1=Disable, 0=Enable)

    }


    if (direccion<32768) {
        *tipo=tipo_low;
        return &memoria_spectrum[offset_segment_low];
    }
    else {
        *tipo=tipo_high;
        return &memoria_spectrum[offset_segment_high];
    }


/*
PSG Port B Output

Bit Name    Description
1   /CART   Memory bank 11, ROM 0000-7FFF (Cartridge /CCS1, /CCS2)  -> Rom 1 para mi (32 kb)
2   /BK21   Memory bank 21, RAM 0000-7FFF                           -> Ram 1 para mi (32 kb)
3   /BK22   Memory bank 22, RAM 8000-FFFF                           -> Ram 2 para mi (32 kb)
4   /BK31   Memory bank 31, RAM 0000-7FFF                           -> Ram 3 para mi (32 kb)

5   /BK32   Memory bank 32, RAM 8000-FFFF                           -> Ram 4 para mi (32 kb)
6   CAPS    Caps-Lock diod
7   /ROMEN0 Memory bank 12, ROM 8000-BFFF* (Cartridge /CCS3)        -> Rom 2.0 para mi (16 kb)
8   /ROMEN1 Memory bank 12, ROM C000-FFFF* (Cartridge /CCS4)        -> Rom 2.1 para mi (16 kb)

Total:  3 ROMS de 32 kb, 5 RAMS de 32 kb,

 The /CART signal must be active for any effect,
  then all banks of RAM are disabled.

  Por defecto: bank01 rom basic, bank02 ram

  Bancos:


  FFFF      BANK 02 RAM     |      BANK 12 CARTRIDGE ROM    |   BANK 22 RAM     |       BANK 32 RAM
  8000


  7FFF      BANK 01 ROM     |      BANK 11 CARTRIDGE ROM    |   BANK 21 RAM     |       BANK 31 RAM
  0000
  */


}

//Retorna el banco en el slot y pagina de SVI. Son slots fijos
//Por defecto guarda un texto mas corto en buffer_mem_type. Si long_buffer_memory_type!=NULL, mete una descripcion mas larga
//para algunos bancos
void svi_get_string_memory_slot(char *buffer_mem_type,char *long_buffer_memory_type, z80_byte slot,z80_byte segment)
{
    		switch (slot) {
						case 0:
							if (segment==0) {
								strcpy(buffer_mem_type,"ROM01");
							}
							else {
								strcpy(buffer_mem_type,"RAM02");
							}
						break;

						case 1:
							if (segment==0) {
								strcpy(buffer_mem_type,"CRO11");
							}
							else {
								strcpy(buffer_mem_type,"CRO12");
							}
						break;

						case 2:
							if (segment==0) {
								strcpy(buffer_mem_type,"RAM21");
							}
							else {
								strcpy(buffer_mem_type,"RAM22");
							}
						break;

						default:
							if (segment==0) {
								strcpy(buffer_mem_type,"RAM31");
							}
							else {
								strcpy(buffer_mem_type,"RAM32");
							}
						break;

					}

    if (long_buffer_memory_type!=NULL) {
        if (slot==1) {
            if (segment==0) {
                strcpy(long_buffer_memory_type,"Cartridge ROM 11");
            }
            else {
                strcpy(long_buffer_memory_type,"Cartridge ROM 12");
            }
        }
        else {
            strcpy(long_buffer_memory_type,buffer_mem_type);
        }
    }
}


void svi_init_memory_tables(void)
{

    //inicio con todos los slots vacios
    /*
    int slot,segment;
    for (slot=0;slot<4;slot++) {
        for (segment=0;segment<4;segment++) {
            svi_memory_slots[slot][segment]=SVI_SLOT_MEMORY_TYPE_EMPTY;
        }

    }


    //De momento meter 32 kb rom, 32 kb ram
    svi_memory_slots[0][0]=SVI_SLOT_MEMORY_TYPE_ROM;
    svi_memory_slots[0][1]=SVI_SLOT_MEMORY_TYPE_ROM;


    svi_memory_slots[2][0]=SVI_SLOT_MEMORY_TYPE_RAM;
    svi_memory_slots[2][1]=SVI_SLOT_MEMORY_TYPE_RAM;
    svi_memory_slots[2][2]=SVI_SLOT_MEMORY_TYPE_RAM;
    svi_memory_slots[2][3]=SVI_SLOT_MEMORY_TYPE_RAM;

    */




}


void svi_reset(void)
{
    //Mapear inicialmente todo a slot 0
    ay_3_8912_registros[ay_chip_selected][15]=0xFF;

    //Resetear vram
    int i;

    for (i=0;i<16384;i++) svi_vram_memory[i]=0;

}

void svi_out_port_vdp_data(z80_byte value)
{
    //printf ("out port vdp data %02XH char: %c\n",value,
    //(value>=32 && value<=126 ? value : '?') );



    vdp_9918a_out_vram_data(svi_vram_memory,value);
}


z80_byte svi_in_port_vdp_data(void)
{
    return vdp_9918a_in_vram_data(svi_vram_memory);
}



z80_byte svi_in_port_vdp_status(void)
{
    return vdp_9918a_in_vdp_status();
}

void svi_out_port_vdp_command_status(z80_byte value)
{
    //printf ("out port vdp command %02XH char: %c\n",value,
    //(value>=32 && value<=126 ? value : '?') );
    vdp_9918a_out_command_status(value);
}


void svi_out_port_ppi(z80_byte puerto_l,z80_byte value)
{
    //printf ("Out port ppi. Port %02XH value %02XH\n",puerto_l,value);

    //int slot,segment;

    switch (puerto_l) {
        case 0x98:
            svi_ppi_register_a=value;
            //printf ("Out port ppi. Port %02XH value %02XH\n",puerto_l,value);

    //temporal mostrar mapeos

/*
    for (slot=0;slot<4;slot++) {
        for (segment=0;segment<4;segment++) {
            printf ("%d %d : %d\n",slot,segment,svi_memory_slots[slot][segment]);
        }
    }
*/

        break;

        case 0x99:
            svi_ppi_register_b=value;
        break;


        case 0x96:
            svi_ppi_register_c=value;


                //printf ("Posible beep: %d\n",value&128);

			set_value_beeper_on_array(da_amplitud_speaker_svi() );


        break;
    }
}

z80_byte svi_in_port_ppi(z80_byte puerto_l)
{
    //printf ("In port ppi. Port %02XH\n",puerto_l);

    z80_byte valor;

    switch (puerto_l) {

        case 0x98:
            valor=255;

            //Esta parte es algo diferente de MSX. El boton del joystick se lee desde aqui. Bits 4 y 5

            if (joystick_emulation==JOYSTICK_SVI) {
                //si estamos con menu abierto, no retornar nada
                if (!zxvision_key_not_sent_emulated_mach() ) {

                    if ((puerto_especial_joystick&16)) valor &=(255-16);  //fire1
                    //TODO: boton 2
                }
            }

            //Bit 6 y 7 es:
            //6: Cassete ready (0=Ready, 1=not ready)
            //7: Cassette read data
            //TODO Bit 6

            int leer_cinta_real=0;

            if (realtape_inserted.v && realtape_playing.v) leer_cinta_real=1;

            if (audio_can_record_input()) {
                if (audio_is_recording_input) {
                    leer_cinta_real=1;
                }
            }

            if (leer_cinta_real) {

                //marcar bit cassete ready
                valor &=(255-64);

				//printf ("%d ",realtape_last_value);
					if (realtape_get_current_bit_playing() ) { //-50
							valor=valor|128;
							//printf ("1 \n");
							//valor=255;
					}
					else {
							valor=(valor & (255-128));
							//printf ("0 \n");
							//valor=0;
					}
			}


            return valor;
        break;

        case 0x99:
            //Leer registro B (filas teclado)
            //que fila? svi_ppi_register_c

            //si estamos en el menu, no devolver tecla
            if (zxvision_key_not_sent_emulated_mach() ) return 255;

            z80_byte fila=svi_ppi_register_c & 0x0F;

            valor=svi_keyboard_table[fila];

            //Si leyendo filas de cursor y joystick habilitado tipo svi, no devolver cursores
            if (joystick_emulation==JOYSTICK_SVI) {
                if (fila>=5 && fila<=8) {
                    //cursores en bit 7
                    valor |=128;
                }
            }

            return valor;

        break;

        case 0x96:
        //printf ("read tape??\n");


		/*valor=0;
                if (realtape_inserted.v && realtape_playing.v) {
                        if (realtape_last_value>=realtape_volumen) {
                                valor=valor|128;
                                printf ("1 ");
                        }
                        else {
                                valor=(valor & (255-128));
                                printf ("0 ");
                        }
                }
		return valor;*/



            //Devolver lo mismo que se ha escrito? TODO revisar esto
            return svi_ppi_register_c;
        break;

    }

    return 255; //temp
}


void svi_out_port_psg(z80_byte puerto_l,z80_byte value)
{
    //printf ("Out port psg. Port %02XH value %02XH\n",puerto_l,value);


        //Registro
        if (puerto_l==0x88) {
                        activa_ay_chip_si_conviene();
                        if (ay_chip_present.v==1) out_port_ay(65533,value);
                }
        //Datos
        if (puerto_l==0x8c) {
                        activa_ay_chip_si_conviene();
                        if (ay_chip_present.v==1) {

                            /*if (ay_3_8912_registro_sel[ay_chip_selected]==14 || ay_3_8912_registro_sel[ay_chip_selected]==15) {

                                //de momento registros 14 y 15 nada
                                if (ay_3_8912_registro_sel[ay_chip_selected]==15) {
                                    printf ("Out port AY register 15: %02XH\n",value);
                                }
                            }*/

                                //if (ay_3_8912_registro_sel[ay_chip_selected]==15) {
                                //    printf ("Out port AY register 15: %02XH\n",value);
                                //}

                            out_port_ay(49149,value);

                        }
        }

}


void svi_alloc_vram_memory(void)
{
    if (svi_vram_memory==NULL) {
        svi_vram_memory=malloc(16384);
        if (svi_vram_memory==NULL) cpu_panic("Cannot allocate memory for svi vram");
    }
}


z80_byte svi_read_vram_byte(z80_int address)
{
    //Siempre leer limitando a 16 kb
    return svi_vram_memory[address & 16383];
}



void svi_insert_rom_cartridge(char *filename)
{

	debug_printf(VERBOSE_INFO,"Inserting svi rom cartridge %s",filename);

    if (!si_existe_archivo(filename)) {
        debug_printf(VERBOSE_ERR,"File %s not found",filename);
        return;
    }

    long tamanyo_archivo=get_file_size(filename);

    if (tamanyo_archivo!=2048 && tamanyo_archivo!=4096 && tamanyo_archivo!=8192 && tamanyo_archivo!=16384 && tamanyo_archivo!=32768) {
        debug_printf(VERBOSE_ERR,"Only 2k, 4k, 8k, 16k and 32k rom cartridges are allowed");
        return;
    }

        FILE *ptr_cartridge;
        ptr_cartridge=fopen(filename,"rb");

        if (!ptr_cartridge) {
		debug_printf (VERBOSE_ERR,"Unable to open cartridge file %s",filename);
                return;
        }



	//Leer cada bloque de 16 kb si conviene. Esto permite tambien cargar cartucho de 8kb como si fuera de 16kb

	//int bloque;

    //int salir=0;

    //int bloques_totales=0;

    //int leidos=fread(&memoria_spectrum[svi_return_offset_rom_page(1,0)],1,32768,ptr_cartridge);

    fread(&memoria_spectrum[svi_return_offset_rom_page(1,0)],1,32768,ptr_cartridge);







        fclose(ptr_cartridge);


        if (noautoload.v==0) {
                debug_printf (VERBOSE_INFO,"Reset cpu due to autoload");
                reset_cpu();
        }

    svi_cartridge_inserted.v=1;

}


void svi_empty_romcartridge_space(void)
{



    //Mapear inicialmente todo a slot 0
    ay_3_8912_registros[ay_chip_selected][15]=0xFF;


    //Limpiar 32 kb de espacio rom de cartucho


    int direccion;

    for (direccion=0;direccion<32768;direccion++) {
        int offset_segment=svi_return_offset_rom_page(1,direccion);

        memoria_spectrum[offset_segment]=0;

    }

    svi_cartridge_inserted.v=0;

    //Y cargar la rom normal
    rom_load(NULL);





}





//Refresco pantalla sin rainbow
void scr_refresca_pantalla_y_border_svi_no_rainbow(void)
{



    if (border_enabled.v && vdp_9918a_force_disable_layer_border.v==0) {
            //ver si hay que refrescar border
            if (modificado_border.v)
            {
                    vdp_9918a_refresca_border();
                    modificado_border.v=0;
            }

    }


    if (vdp_9918a_force_disable_layer_ula.v==0) {

        //Capa activada. Pero tiene reveal?

        if (vdp_9918a_reveal_layer_ula.v) {
            //En ese caso, poner fondo tramado
            int x,y;
            for (y=0;y<192;y++) {
                for (x=0;x<256;x++) {
                    int posx=x&1;
			        int posy=y&1;
                    int si_blanco_negro=posx ^ posy;
                    int color=si_blanco_negro*15;
                    scr_putpixel_zoom(x,y,  VDP_9918_INDEX_FIRST_COLOR+color);
                }
            }
        }
        else {
            vdp_9918a_render_ula_no_rainbow(svi_vram_memory);
        }
    }

    else {
        //En ese caso, poner fondo negro
        int x,y;
        for (y=0;y<192;y++) {
            for (x=0;x<256;x++) {
                scr_putpixel_zoom(x,y,  VDP_9918_INDEX_FIRST_COLOR+0);
            }
        }
    }



    if (vdp_9918a_force_disable_layer_sprites.v==0) {
        vdp_9918a_render_sprites_no_rainbow(svi_vram_memory);
    }




}


void scr_refresca_pantalla_y_border_svi(void)
{
    if (rainbow_enabled.v) {
        vdp_9918a_scr_refresca_pantalla_y_border_rainbow();
    }
    else {
        scr_refresca_pantalla_y_border_svi_no_rainbow();
    }
}

int da_amplitud_speaker_svi(void)
{
                                if (svi_ppi_register_c & 128) return amplitud_speaker_actual_svi;
                                else return -amplitud_speaker_actual_svi;
}






//Almacenaje temporal de render de la linea actual
z80_int svi_scanline_buffer[512];


void screen_store_scanline_rainbow_svi_border_and_display(void)
{

    screen_store_scanline_rainbow_vdp_9918a_border_and_display(svi_scanline_buffer,svi_vram_memory);


}



int svi_cas_load_detect(void)
{
    if (!tapefile) return 0;

    if ( (tape_loadsave_inserted & TAPE_LOAD_INSERTED)==0) return 0;


    //Si es el JP de la ROM
    if (reg_pc==0x69 && peek_byte_no_time(0x69)==195) return 1;
    //O a donde salta ese JP
    if (reg_pc==0x203a && peek_byte_no_time(0x203a)==0xE5) return 1;

    //Si es el JP de la ROM
    if (reg_pc==0x6C && peek_byte_no_time(0x69)==195) return 1;
    //O a donde salta ese JP
    if (reg_pc==0x2016 && peek_byte_no_time(0x2016)==0xD5) return 1;




    return 0;
}



//#define SVI_CAS_HEADER_LENGTH 16


//16
//z80_byte svi_cabecera_firma[SVI_CAS_HEADER_LENGTH] = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x7f };


//int primera_vez=0;


//Buscar cabecera en cinta. En vez de complicarlo tanto com msx, buscar solo byte 7f, que va despues de la secuencia 0x55
/*
una cinta .cas tiene el contenido:

00000000  55 55 55 55 55 55 55 55  55 55 55 55 55 55 55 55  |UUUUUUUUUUUUUUUU|
00000010  7f d3 d3 d3 d3 d3 d3 d3  d3 d3 d3 41 4c 49 45 4e  |...........ALIEN|
00000020  38 ff 00 55 55 55 55 55  55 55 55 55 55 55 55 55  |8..UUUUUUUUUUUUU|
00000030  55 55 55 7f 18 80 0a 00  9f 3a bd 0f 0f 2c 12 2c  |UUU......:...,.,|
00000040  12 3a d8 2c 2c 11 3a c5  2c 11 00 40 80 14 00 91  |.:.,,.:.,..@....|
00000050  3a 91 22 20 20 20 20 20  20 20 20 20 41 4c 49 45  |:."         ALIE|
00000060  4e 20 38 20 69 73 20 6c  6f 61 64 69 6e 67 20 2e  |N 8 is loading .|

Podria buscar esos 16 bytes 55H y luego el 0x7f, pero no creo que valga la pena complicarse tanto. Busco el 7F y listo
Si al final de cada bloque alguien mete un 0x7f en vez de los 16 bytes 55H, esto funcionaria mal, se posicionaria despues del 7F, o
sea en el primer 55H

Nota: ver funciones similares de MSX, ahí si que se busca una cabecera de varios bytes

*/
void svi_cas_lookup_header(void)
{

    int leidos;

    //Nos quedamos con la posicion actual
    //long posicion_cas=ftell(ptr_mycinta);

    //Leemos SVI_CAS_HEADER_LENGTH bytes
    z80_byte buffer_lectura;

    while (1) {



        debug_printf (VERBOSE_DEBUG,"Searching for CAS header");
        leidos=fread(&buffer_lectura,1,1,ptr_mycinta);
        //printf ("posible byte cabecera: %02XH\n",buffer_lectura[0]);

        if (buffer_lectura==0x7f) {
                //printf ("Encontrada cabecera\n\n");
                //sleep(1);


                //OK
                //Z80_FLAGS &=(255-FLAG_C);

                //No tengo claro que los flags vayan asi
                Z80_FLAGS |= FLAG_C;


                return;
        }

        //Error
        //Z80_FLAGS |= FLAG_C;




        if (leidos<1) {
            //Expulsar cinta
            tape_loadsave_inserted = tape_loadsave_inserted & (255 - TAPE_LOAD_INSERTED);

            debug_printf (VERBOSE_INFO,"Ejecting CAS tape");

            return;
        }

    }

return;






}



//int temporal_print_byte;

void svi_cas_read_byte(void)
{

    debug_printf (VERBOSE_PARANOID,"Reading CAS byte");


    //Leemos 1 byte
    z80_byte byte_leido;

    int leidos=fread(&byte_leido,1,1,ptr_mycinta);

    if (leidos==1) {
        reg_a=byte_leido;
        //printf ("%02X ",reg_a);


        //temporal_print_byte++;
        //if ( (temporal_print_byte % 1024) == 0) printf ("\n");

            //Quitar carry y volver
            Z80_FLAGS &=(255-FLAG_C);
            return;

    }


    //Error. devolver carry
    Z80_FLAGS |= FLAG_C;


    //Expulsar cinta
    tape_loadsave_inserted = tape_loadsave_inserted & (255 - TAPE_LOAD_INSERTED);

}


void svi_cas_load(void)
{

    /*

SVI:

6C - CASIN  (es un JP a 2016H)
69 - CSRDON (es un JP a 203aH)



    */

    if (reg_pc==0x69 || reg_pc==0x203a) {
        debug_printf (VERBOSE_DEBUG,"Searching for CAS header");
        //Buscar cabecera
        svi_cas_lookup_header();

        //RET
        reg_pc=pop_valor();

        //printf ("volver de buscar cabecera a direccion %04XH\n",reg_pc);
        return;
    }

    if (reg_pc==0x6C || reg_pc==0x2016) {
        //printf ("cargar byte\n");
        //Cargar byte
        svi_cas_read_byte();

        //RET
        reg_pc=pop_valor();

        return;
    }

    //Aqui no se deberia llegar nunca
    //Pero hacemos reg_pc++ y que continue la fiesta
    reg_pc++;

}

z80_byte svi_read_psg(void)
{

        //Registro 14 del ay chip realmente es el port A del ay chip
		if ( (ay_3_8912_registro_sel[ay_chip_selected] & 15) ==14) {


			z80_byte valor=255;


            //Si esta activada emulacion de joystick msx
            if (joystick_emulation==JOYSTICK_SVI) {
                //si estamos con menu abierto, no retornar nada
                if (!zxvision_key_not_sent_emulated_mach() ) {
                        /*
                        0	Input joystick pin 1	0 = up
                        1	Input joystick pin 2	0 = down
                        2	Input joystick pin 3	0 = left
                        3	Input joystick pin 4	0 = right
                        Bits 4,5,6,7 son del segundo joystick. El fire va por el registro svi_ppi_register_a
                        */
                    //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right

                    if ((puerto_especial_joystick&8)) valor &=(255-1);  //up
                    if ((puerto_especial_joystick&4)) valor &=(255-2);  //down
                    if ((puerto_especial_joystick&2)) valor &=(255-4);   //left
                    if ((puerto_especial_joystick&1)) valor &=(255-8);   //right

                }
            }

			return valor;
		}


		else {
			return in_port_ay(0xFF);
		}
}