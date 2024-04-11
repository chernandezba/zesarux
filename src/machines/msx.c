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

#include "msx.h"
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

z80_byte *msx_vram_memory=NULL;

z80_byte msx_ppi_register_a;
//bit 0-1:slot segmento 0 (0000h-3fffh)
//bit 2-3:slot segmento 1 (4000h-7fffh)
//bit 4-5:slot segmento 2 (8000h-bfffh)
//bit 6-7:slot segmento 3 (C000h-ffffh)


z80_byte msx_ppi_register_b;
z80_byte msx_ppi_register_c;


//Aunque solo son 10 filas, metemos array de 16 pues es el maximo valor de indice seleccionable por el PPI
z80_byte msx_keyboard_table[16]={
255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255
};


//slots asignados, y sus 4 segmentos
//tipos: rom, ram, vacio
int msx_memory_slots[4][4];

z80_bit msx_cartridge_inserted={0};

const char *msx_string_memory_type_rom="ROM";
const char *msx_string_memory_type_ram="RAM";
const char *msx_string_memory_type_empty="EMPTY";


int msx_mapper_type=MSX_MAPPER_TYPE_NONE;

//bancos para el mapper
//Por ejemplo posiciones 0 y 1 de ascii_16kb es para el segmento 4000h y el 8000h
int msx_mapper_rom_cartridge_pages[4];

z80_byte *msx_assigned_memory_mapper=NULL;
int msx_cartridge_size=0;

char *msx_get_string_memory_type(int tipo)
{


    switch (tipo) {

        case MSX_SLOT_MEMORY_TYPE_ROM:
            return (char *)msx_string_memory_type_rom;
        break;

        case MSX_SLOT_MEMORY_TYPE_RAM:
            return (char *)msx_string_memory_type_ram;
        break;

        default:
            return (char *)msx_string_memory_type_empty;
        break;

    }
}


//Retorna direccion de memoria donde esta mapeada la ram y su tipo
z80_byte *msx_return_segment_address(z80_int direccion,int *tipo)
{
    int segmento=direccion/16384;

    z80_byte slot=msx_ppi_register_a;

    /*
    ppi_register_a

    bits:
    76  54  32  10
    |   |   |   |
    |   |   |   ------ Slot for page 0
    |   |   |
    |   |   ---------- Slot for page 1
    |   |
    |   -------------- Slot for page 2
    |
    ------------------ Slot for page 3


    */


    int rotar=segmento*2;

    if (segmento>0) {
        slot=slot>>rotar;
    }

    slot &=3;


    *tipo=msx_memory_slots[slot][segmento];

    //if (msx_mapper_type && slot==1) printf("hay mapper. slot=%d tipo=%d\n",slot,*tipo);

    int offset=((slot*4)+segmento)*16384;

    //if (slot>0) printf ("direccion %6d segmento %d slot %d offset %d\n",direccion,segmento,slot,offset);

    if (msx_mapper_type!=MSX_MAPPER_TYPE_NONE && (*tipo)==MSX_SLOT_MEMORY_TYPE_ROM) {
        //Hay un mapper
        //printf("hay mapper\n");

        if (slot==1) {

            //Ascii 16kb
            if (msx_mapper_type==MSX_MAPPER_TYPE_ASCII_16KB) {
                if (segmento==1 || segmento==2) {

                    int indice_bloque_16kb=segmento-1;
                    int bloque_cartucho_16kb=msx_mapper_rom_cartridge_pages[indice_bloque_16kb];

                    //limitar
                    int total_bloques_cartucho=msx_cartridge_size/16384;
                    bloque_cartucho_16kb=bloque_cartucho_16kb % total_bloques_cartucho;

                    //printf("Retornando dir %04XH de memory mapper de bloque %d\n",direccion,bloque_cartucho);
                    return &msx_assigned_memory_mapper[bloque_cartucho_16kb*16384+(direccion&16383)];
                }
            }

            //Ascii 8kb
            if (msx_mapper_type==MSX_MAPPER_TYPE_ASCII_8KB) {
                if (segmento==1 || segmento==2) {

                    int segmento_8kb=(direccion-16384)/8192;

                    int indice_bloque_8kb=segmento_8kb;
                    int bloque_cartucho_8kb=msx_mapper_rom_cartridge_pages[indice_bloque_8kb];

                    //limitar
                    int total_bloques_cartucho=msx_cartridge_size/8192;
                    bloque_cartucho_8kb=bloque_cartucho_8kb % total_bloques_cartucho;

                    //printf("Retornando dir %04XH de memory mapper de bloque %d\n",direccion,bloque_cartucho);
                    return &msx_assigned_memory_mapper[bloque_cartucho_8kb*8192+(direccion&8191)];
                }
            }

            //Konami megarom without scc
            if (msx_mapper_type==MSX_MAPPER_TYPE_KONAMI_MEGAROM_WITHOUT_SCC) {
                if (segmento==1 || segmento==2) {

                    int segmento_8kb=(direccion-16384)/8192;

                    int indice_bloque_8kb=segmento_8kb;
                    int bloque_cartucho_8kb=msx_mapper_rom_cartridge_pages[indice_bloque_8kb];

                    //limitar
                    int total_bloques_cartucho=msx_cartridge_size/8192;
                    bloque_cartucho_8kb=bloque_cartucho_8kb % total_bloques_cartucho;

                    //printf("Retornando dir %04XH de memory mapper de bloque %d\n",direccion,bloque_cartucho);
                    return &msx_assigned_memory_mapper[bloque_cartucho_8kb*8192+(direccion&8191)];
                }
            }

            //R-Type
            if (msx_mapper_type==MSX_MAPPER_TYPE_RTYPE) {
                if (segmento==1 || segmento==2) {

                    int indice_bloque_16kb=segmento-1;
                    int bloque_cartucho_16kb=msx_mapper_rom_cartridge_pages[indice_bloque_16kb];

                    if (indice_bloque_16kb==0) bloque_cartucho_16kb=0x0F;

                    //limitar
                    int total_bloques_cartucho=msx_cartridge_size/16384;
                    bloque_cartucho_16kb=bloque_cartucho_16kb % total_bloques_cartucho;

                    //printf("Retornando dir %04XH de memory mapper de bloque %d\n",direccion,bloque_cartucho);
                    return &msx_assigned_memory_mapper[bloque_cartucho_16kb*16384+(direccion&16383)];
                }
            }

        }
    }


    return &memoria_spectrum[offset+(direccion&16383)];



}


void msx_init_memory_tables(void)
{

    //inicio con todos los slots vacios
    int slot,segment;
    for (slot=0;slot<4;slot++) {
        for (segment=0;segment<4;segment++) {
            msx_memory_slots[slot][segment]=MSX_SLOT_MEMORY_TYPE_EMPTY;
        }

    }


    //De momento meter 32 kb rom, 32 kb ram
    msx_memory_slots[0][0]=MSX_SLOT_MEMORY_TYPE_ROM;
    msx_memory_slots[0][1]=MSX_SLOT_MEMORY_TYPE_ROM;


    msx_memory_slots[2][0]=MSX_SLOT_MEMORY_TYPE_RAM;
    msx_memory_slots[2][1]=MSX_SLOT_MEMORY_TYPE_RAM;
    msx_memory_slots[2][2]=MSX_SLOT_MEMORY_TYPE_RAM;
    msx_memory_slots[2][3]=MSX_SLOT_MEMORY_TYPE_RAM;






}


void msx_reset(void)
{
    //Mapear inicialmente todo a slot 0
    msx_ppi_register_a=0;

    //Resetear vram
    int i;

    for (i=0;i<16384;i++) msx_vram_memory[i]=0;

    //Indices comunes a todos los mappers: ascii 16kb, ascii 8kb, etc
    msx_mapper_rom_cartridge_pages[0]=0;
    msx_mapper_rom_cartridge_pages[1]=1;
    msx_mapper_rom_cartridge_pages[2]=2;
    msx_mapper_rom_cartridge_pages[3]=3;

}

void msx_out_port_vdp_data(z80_byte value)
{
    vdp_9918a_out_vram_data(msx_vram_memory,value);
}


z80_byte msx_in_port_vdp_data(void)
{
    return vdp_9918a_in_vram_data(msx_vram_memory);
}



z80_byte msx_in_port_vdp_status(void)
{
    return vdp_9918a_in_vdp_status();
}

void msx_out_port_vdp_command_status(z80_byte value)
{
    vdp_9918a_out_command_status(value);
}


void msx_out_port_ppi(z80_byte puerto_l,z80_byte value)
{
    //printf ("Out port ppi. Port %02XH value %02XH\n",puerto_l,value);

    //int slot,segment;

    switch (puerto_l) {
        case 0xA8:
            msx_ppi_register_a=value;
            //printf ("Out port ppi. Port %02XH value %02XH\n",puerto_l,value);

    //temporal mostrar mapeos

/*
    for (slot=0;slot<4;slot++) {
        for (segment=0;segment<4;segment++) {
            printf ("%d %d : %d\n",slot,segment,msx_memory_slots[slot][segment]);
        }
    }
*/

        break;

        case 0xA9:
            msx_ppi_register_b=value;
        break;


        case 0xAA:
            msx_ppi_register_c=value;

            //Curiosidad: Chase HQ utiliza esto
            //printf ("Posible beep: %d\n",value&128);

			set_value_beeper_on_array(da_amplitud_speaker_msx() );


        break;
    }
}

z80_byte msx_in_port_ppi(z80_byte puerto_l)
{
    //printf ("In port ppi. Port %02XH\n",puerto_l);

    //z80_byte valor;

    switch (puerto_l) {

        case 0xA8:
            return msx_ppi_register_a;
        break;

        case 0xA9:
            //Leer registro B (filas teclado)
            //que fila? msx_ppi_register_c

            //si estamos en el menu, no devolver tecla
            if (zxvision_key_not_sent_emulated_mach() ) return 255;

            z80_byte fila=msx_ppi_register_c & 0x0F;

            z80_byte valor_fila=msx_keyboard_table[fila];

            //Si row8, donde estan los cursores, y tipo joystick es msx, entonces no retornamos cursores
            //row 8 derecha abajo arriba izquierda	DEL	INS	HOME	SPACE

            if (fila==8 && joystick_emulation==JOYSTICK_MSX) valor_fila |=0xF0;


            return valor_fila;

        break;

        case 0xAA:

        /*
AAh	is the port to access the register that control the keyboard CAP LED,
two signals to data recorder and a matrix row (use the port C of PPI).

bits 0~3 = Row number of specified keyboard matrix to read via port B
bit 4 = Data recorder motor (reset to turn on)
bit 5 = Set to write on tape
bit 6 = Keyboard LED CAPS (reset to turn on)
bit 7 = 1, then 0 shortly thereafter to make a clicking sound (used for the keyboard).
        */



            //Devolver lo mismo que se ha escrito
            return msx_ppi_register_c;
        break;

    }

    return 255; //temp
}


void msx_out_port_psg(z80_byte puerto_l,z80_byte value)
{
    //printf ("Out port psg. Port %02XH value %02XH\n",puerto_l,value);


        //Registro
        if (puerto_l==0xA0) {
                        activa_ay_chip_si_conviene();
                        if (ay_chip_present.v==1) out_port_ay(65533,value);
                }
        //Datos
        if (puerto_l==0xA1) {
                        activa_ay_chip_si_conviene();
                        if (ay_chip_present.v==1) {

                            if (ay_3_8912_registro_sel[ay_chip_selected]==14 || ay_3_8912_registro_sel[ay_chip_selected]==15) {

                                //de momento registros 14 y 15 nada
                            }
                            else out_port_ay(49149,value);

                        }
        }

}


void msx_alloc_vram_memory(void)
{
    if (msx_vram_memory==NULL) {
        msx_vram_memory=malloc(16384);
        if (msx_vram_memory==NULL) cpu_panic("Cannot allocate memory for msx vram");
    }
}


z80_byte msx_read_vram_byte(z80_int address)
{
    //Siempre leer limitando a 16 kb
    return msx_vram_memory[address & 16383];
}

void msx_insert_rom_load_mapper_common(char *filename,long tamanyo_archivo)
{
        FILE *ptr_cartridge;
        ptr_cartridge=fopen(filename,"rb");

        if (!ptr_cartridge) {
		debug_printf (VERBOSE_ERR,"Unable to open cartridge file %s",filename);
                return;
        }

    //Vamos a asignar memoria para almacenar ese cartucho
    if (msx_assigned_memory_mapper!=NULL) free (msx_assigned_memory_mapper);

    msx_assigned_memory_mapper=util_malloc(tamanyo_archivo,"Can not assign memory for rom cartridge");


	fread(msx_assigned_memory_mapper,1,tamanyo_archivo,ptr_cartridge);

    fclose(ptr_cartridge);

    msx_memory_slots[1][1]=MSX_SLOT_MEMORY_TYPE_ROM;
    msx_memory_slots[1][2]=MSX_SLOT_MEMORY_TYPE_ROM;



        if (noautoload.v==0) {
                debug_printf (VERBOSE_INFO,"Reset cpu due to autoload");
                reset_cpu();
        }

    msx_cartridge_inserted.v=1;

}



void msx_insert_rom_cartridge_no_mapper(char *filename,long tamanyo_archivo)
{

    printf("Loading rom cartridge with no mapper\n");
    msx_mapper_type=MSX_MAPPER_TYPE_NONE;

        FILE *ptr_cartridge;
        ptr_cartridge=fopen(filename,"rb");

        if (!ptr_cartridge) {
		debug_printf (VERBOSE_ERR,"Unable to open cartridge file %s",filename);
                return;
        }



	//Leer cada bloque de 16 kb si conviene. Esto permite tambien cargar cartucho de 8kb como si fuera de 16kb

	int bloque;

    int salir=0;

    int bloques_totales=0;


    int bloque_inicial=1;

/*
If you go to 48K, use the extra 16K in page 0, not in page 3.
Ejemplo de cartucho de 48K: Trucho Demo
*/

    if (tamanyo_archivo==49152) {
        bloque_inicial=0;
    }

    int offset_inicial=bloque_inicial*16384;

	for (bloque=0;bloque<3 && !salir;bloque++) {
        /*
        The ROM Header

A ROM needs a header to be auto-executed by the system when the MSX is initialized.

After finding the RAM and initializing the system variables, the MSX looks for the ROM headers in all the slots
on the memory pages 4000h-7FFFh and 8000h-FFFh. The search is done in ascending order.
When a primary Slot is expanded, the search is done in the corresponding secondary Slots before going to the next Primary Slot.
When the system finds a header, it selects the ROM slot only on the memory page corresponding to the address specified in INIT then, runs the program in ROM at the same address. (In short, it makes an inter-slot call.)

        */
        int offset=65536+bloque*16384;
		int leidos=fread(&memoria_spectrum[offset+offset_inicial],1,16384,ptr_cartridge);
        if (leidos==16384) {
            msx_memory_slots[1][bloque_inicial+bloque]=MSX_SLOT_MEMORY_TYPE_ROM;
            debug_printf (VERBOSE_INFO,"Loaded 16kb bytes of rom at slot 1 block %d",bloque);

            bloques_totales++;


        }
        else {
            salir=1;
        }

	}

    if (bloques_totales==1) {
            //Copiar en los otros 3 segmentos

            //Antes, si es un bloque de 8kb, copiar 8kb bajos en parte alta
            if (tamanyo_archivo==8192) {
                memcpy(&memoria_spectrum[65536+16384+8192],&memoria_spectrum[65536+16384],8192);
            }

            memcpy(&memoria_spectrum[65536],&memoria_spectrum[65536+16384],16384);
            memcpy(&memoria_spectrum[65536+32768],&memoria_spectrum[65536+16384],16384);
            memcpy(&memoria_spectrum[65536+49152],&memoria_spectrum[65536+16384],16384);

            msx_memory_slots[1][0]=MSX_SLOT_MEMORY_TYPE_ROM;
            msx_memory_slots[1][2]=MSX_SLOT_MEMORY_TYPE_ROM;
            msx_memory_slots[1][3]=MSX_SLOT_MEMORY_TYPE_ROM;
    }



    //int i;


        fclose(ptr_cartridge);


        if (noautoload.v==0) {
                debug_printf (VERBOSE_INFO,"Reset cpu due to autoload");
                reset_cpu();
        }

    msx_cartridge_inserted.v=1;

}

void msx_insert_rom_cartridge(char *filename)
{

	debug_printf(VERBOSE_INFO,"Inserting msx rom cartridge %s",filename);

    //De momento quitar toda info de cartucho
    msx_empty_romcartridge_space();

    if (!si_existe_archivo(filename)) {
        debug_printf(VERBOSE_ERR,"File %s not found",filename);
        return;
    }

    long tamanyo_archivo=get_file_size(filename);

    msx_cartridge_size=tamanyo_archivo;

    //si cartucho>= 64kb activamos automapper ascii 16kb
    if (tamanyo_archivo>=65536) {
        printf("Loading cartridge using memory mapper. Assuming defaults ascii 16kb\n");

        msx_insert_rom_load_mapper_common(filename,tamanyo_archivo);

        //Aqui habria que hacer autodeteccion del mapper. De momento asumimos ese
        msx_mapper_type=MSX_MAPPER_TYPE_ASCII_16KB;
        return;
    }

    if (tamanyo_archivo!=8192 && tamanyo_archivo!=16384 && tamanyo_archivo!=32768 && tamanyo_archivo!=49152) {
        debug_printf(VERBOSE_ERR,"If rom cartridge is smaller than 64kb, only 8k, 16k, 32k and 48k rom cartridges are allowed");
        return;
    }

    msx_insert_rom_cartridge_no_mapper(filename,tamanyo_archivo);


}


void msx_empty_romcartridge_space(void)
{

    int i;
    for (i=0;i<4;i++) {
        msx_memory_slots[1][i]=MSX_SLOT_MEMORY_TYPE_EMPTY;
    }

    msx_cartridge_inserted.v=0;
    msx_mapper_type=MSX_MAPPER_TYPE_NONE;

}





//Refresco pantalla sin rainbow
void scr_refresca_pantalla_y_border_msx_no_rainbow(void)
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
            vdp_9918a_render_ula_no_rainbow(msx_vram_memory);
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
        vdp_9918a_render_sprites_no_rainbow(msx_vram_memory);
    }




}


void scr_refresca_pantalla_y_border_msx(void)
{
    if (rainbow_enabled.v) {
        vdp_9918a_scr_refresca_pantalla_y_border_rainbow();
    }
    else {
        scr_refresca_pantalla_y_border_msx_no_rainbow();
    }
}

int da_amplitud_speaker_msx(void)
{
                                if (msx_ppi_register_c & 128) return amplitud_speaker_actual_msx;
                                //else return -amplitud_speaker_actual_msx;
                                else return 0;
}






//Almacenaje temporal de render de la linea actual
z80_int msx_scanline_buffer[512];


void screen_store_scanline_rainbow_msx_border_and_display(void)
{

    screen_store_scanline_rainbow_vdp_9918a_border_and_display(msx_scanline_buffer,msx_vram_memory);


}




int tape_block_cas_open(void)
{

		ptr_mycinta=fopen(tapefile,"rb");

                if (!ptr_mycinta)
                {
                        debug_printf(VERBOSE_ERR,"Unable to open cas input file %s",tapefile);


			tapefile=0;
                        return 1;
                }

		return 0;

}

int msx_cas_load_detect(void)
{
    if (!tapefile) return 0;

    if ( (tape_loadsave_inserted & TAPE_LOAD_INSERTED)==0) return 0;

    if (reg_pc==0xE1 || reg_pc==0xE4) return 1;

    return 0;
}

z80_byte msx_cabecera_firma[8] = { 0x1F,0xA6,0xDE,0xBA,0xCC,0x13,0x7D,0x74 };

void msx_cas_lookup_header(void)
{

    while (1) {
        debug_printf (VERBOSE_DEBUG,"Searching for CAS header");

        //Nos quedamos con la posicion actual
        long posicion_cas=ftell(ptr_mycinta);

        //Leemos 8 bytes
        z80_byte buffer_lectura[8];

        int leidos=fread(buffer_lectura,1,8,ptr_mycinta);

        if (leidos==8) {
            //Ver si se ha leido la firma de cabecera
            if (!memcmp(buffer_lectura,msx_cabecera_firma,8)) {
                //Quitar carry y volver
                Z80_FLAGS &=(255-FLAG_C);
                debug_printf (VERBOSE_DEBUG,"Cas header found");
                return;
            }
        }


        //Error. Saltar 1 byte, devolver carry
        Z80_FLAGS |= FLAG_C;

        if (leidos<8) {
            //Expulsar cinta
            tape_loadsave_inserted = tape_loadsave_inserted & (255 - TAPE_LOAD_INSERTED);

            debug_printf (VERBOSE_INFO,"Ejecting CAS tape");
            return;
        }

        else {
            posicion_cas++;
            fseek(ptr_mycinta, posicion_cas, SEEK_SET);
        }

        debug_printf (VERBOSE_DEBUG,"Cas header not found");
    }
}



void msx_cas_read_byte(void)
{

    debug_printf (VERBOSE_PARANOID,"Reading CAS byte");


    //Leemos 1 byte
    z80_byte byte_leido;

    int leidos=fread(&byte_leido,1,1,ptr_mycinta);

    if (leidos==1) {
        reg_a=byte_leido;
            //Quitar carry y volver
            Z80_FLAGS &=(255-FLAG_C);
            return;

    }


    //Error. devolver carry
    Z80_FLAGS |= FLAG_C;


    //Expulsar cinta
    tape_loadsave_inserted = tape_loadsave_inserted & (255 - TAPE_LOAD_INSERTED);

}


void msx_cas_load(void)
{

    /*
TAPION (00E1H)		*1
  Function:	reads the header block after turning the cassette motor ON.
  Input:	none
  Output:	if failed, the CY flag is set
  Registers:	all


TAPIN (00E4H)		*1
  Function:	reads data from the tape
  Input:	none
  Output:	A for data. If failed, the CY flag is set.
  Registers:	all


The cas format is the result of bypassing the following BIOS calls:

00E1 - TAPION
00E4 - TAPIN
00EA - TAPOON
00ED - TAPOUT

If you call the TAPION function, the BIOS will read from tape untill it has
found a header, and all of the header is read. The TAPOUT function will
output a header. In the cas format the header is encoded to these 8 bytes:

1F A6 DE BA CC 13 7D 74

These bytes have to be at a position that can be divided by 8; e.g. 0000,
0008, 0010 etc. If not, the byte sequence is not recognised as a header.



There are 4 types of data that can be stored on a tape;

* binary files (bload)
* basic files (cload)
* ascii files (load)
* custom data (to be loaded using the bios)

Data is stored on the tape in blocks, each block is preceeded by a header (the 1f a6 de .. block). The purpose of this header (the pieeeeeeeeep), is to sync for decoding the fsk data.

Binary files (bload) consist out of two blocks; a binary header block, and a binary data block. The binary header block is specified by 10 times 0xD0, followed by 6 characters defining its filename. The block following this header, is the datablock, which also defines the begin,end and start address (0xFE,begin,end,start).

Basic files (cload) also consist out of two blocks; a basic header block, and a basic data block. The basic header block is specified by 10 times 0xD3, followed by 6 characters defining its filename. The block folowing this header is the datablock (tokenized basic data).

Ascii files (load) can consist out of multiple blocks. The first block is always the ascii header block, which is specified by 10 times 0xea, followed again by 6 characters defining the filename. After this, an unlimited number of blocks can follow. The last block can be identified by the EOF (0x1a) character.

Custom blocks are all blocks that don't fit in the 3 specified above.

You might want to look at the casdir.c code, which implemented the above.
    */

    if (reg_pc==0xE1) {
        //Buscar cabecera
        msx_cas_lookup_header();

        //RET
        reg_pc=pop_valor();
        return;
    }

    if (reg_pc==0xE4) {
        //Cargar byte
        msx_cas_read_byte();

        //RET
        reg_pc=pop_valor();

        return;
    }

    //Aqui no se deberia llegar nunca
    //Pero hacemos reg_pc++ y que continue la fiesta
    reg_pc++;

}


z80_byte msx_read_psg(void)
{
    	//if (puerto_l==0xA0 && ay_3_8912_registro_sel[0]==14) {

		//printf ("reading from psg\n");


		if ( (ay_3_8912_registro_sel[ay_chip_selected] & 15) ==14) {
			//printf ("Read AY register 14. read tape / joystick\n");

			//Bit 7 cinta
			/*
PSG I/O port A (r#14) – read-only
Bit	Description	Comment
0	Input joystick pin 1	0 = up
1	Input joystick pin 2	0 = down
2	Input joystick pin 3	0 = left
3	Input joystick pin 4	0 = right
4	Input joystick pin 6	0 = trigger A
5	Input joystick pin 7	0 = trigger B
6	Japanese keyboard layout bit	1 = JIS, 0 = ANSI/AIUEO/50on
7	Cassette input signal
			*/
			z80_byte valor=255;
			if (realtape_inserted.v && realtape_playing.v) {
				//printf ("(%d) ",realtape_last_value);

					//margen de ceros. Reduccion ruido
					if (msx_loading_noise_reduction.v) {
						if (realtape_last_value>=-1 && realtape_last_value<=1) realtape_last_value=-1;
					}

					//Cambiar color border si conviene
					z80_byte border_reg=vdp_9918a_registers[7];

					border_reg &=0xF0;

					if (realtape_get_current_bit_playing()) {
							valor=valor|128;
							//printf ("1 ");
							//valor=255;


							//Cambiar color border
							border_reg |=4; //azul



					}
					else {
							valor=(valor & (255-128));
							//printf ("0 ");
							//valor=0;

							//Cambiar color border
							border_reg |=8; //naranja

					}

					if (msx_loading_stripes.v) vdp_9918a_registers[7]=border_reg;




					//temp_conta_lee_puerto_msx1_no_time++; if ((temp_conta_lee_puerto_msx1_no_time % 1000)==0) printf ("\n");
			}

            //Si esta activada emulacion de joystick msx
            if (joystick_emulation==JOYSTICK_MSX) {
                //si estamos con menu abierto, no retornar nada
                if (!zxvision_key_not_sent_emulated_mach() ) {
                        /*
                        0	Input joystick pin 1	0 = up
                        1	Input joystick pin 2	0 = down
                        2	Input joystick pin 3	0 = left
                        3	Input joystick pin 4	0 = right
                        4	Input joystick pin 6	0 = trigger A
                        5	Input joystick pin 7	0 = trigger B
                        */
                    //z80_byte puerto_especial_joystick=0; //Fire Up Down Left Right

                    if ((puerto_especial_joystick&8)) valor &=(255-1);  //up
                    if ((puerto_especial_joystick&4)) valor &=(255-2);  //down
                    if ((puerto_especial_joystick&2)) valor &=(255-4);   //left
                    if ((puerto_especial_joystick&1)) valor &=(255-8);   //right
                    if ((puerto_especial_joystick&16)) valor &=(255-16);  //fire1
                    //TODO: boton 2
                }
            }



			//printf ("%02XH ",valor);
			return valor;
		}

		//Registro 15 nada de momento
		/*
PSG I/O port B (r#15) – write/read
Bit	Description	Comment
0	Output joystick port 1, pin 6	Set to 1 to allow input
1	Output joystick port 1, pin 7	Set to 1 to allow input
2	Output joystick port 2, pin 6	Set to 1 to allow input
3	Output joystick port 2, pin 7	Set to 1 to allow input
4	Output joystick port 1, pin 8
5	Output joystick port 2, pin 8
6	Joystick input selection, for r#14 inputs	1 = port 2
7	Kana led control	1 = off
		*/
		else if ( (ay_3_8912_registro_sel[ay_chip_selected] & 15) ==15) {
			return 255;
		}

		else {
			return in_port_ay(0xFF);
		}

}