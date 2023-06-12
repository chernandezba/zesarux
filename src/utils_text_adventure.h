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

#ifndef UTILS_TEXT_ADVENTURE_H
#define UTILS_TEXT_ADVENTURE_H


#include "cpu.h"




#define DAAD_PARSER_BREAKPOINT_PC_SPECTRUM 0x617c

#define DAAD_PARSER_BREAKPOINT_PC_CPC 0x09b4 
//0x14df

#define DAAD_PARSER_CONDACT_BREAKPOINT 220

#define PAWS_LONGITUD_PALABRAS 5
#define QUILL_LONGITUD_PALABRAS 4

#define MENU_DEBUG_NUMBER_FLAGS_OBJECTS 7

#define TEXT_ADVENTURE_MAX_LOCATIONS 256

#define MAX_ALLOWED_TEXT_ADVENTURE_LOCATION_LENGTH 255

extern void util_add_text_adventure_kdb(char *texto);

extern void util_clear_text_adventure_kdb(void);

extern int util_paws_dump_vocabulary(int *p_quillversion);

extern char *quillversions_strings[];

extern int util_gac_dump_dictonary(int *p_gacversion);

extern char *gacversions_strings[];

extern int util_unpawsetc_dump_words(char *mensaje);


extern char *util_unpaws_get_parser_name(void);
extern char *util_undaad_unpaws_ungac_get_parser_name(void);
extern int util_paws_is_in_parser(void);

extern z80_int util_daad_get_start_pointers(void);
extern void util_daad_get_language_parser(char *texto);
extern int util_daad_detect(void);
extern int util_textadv_detect_paws_quill(void);
extern z80_int util_daad_get_start_vocabulary(void);
extern int util_daad_dump_vocabulary(int tipo,char *texto,int max_string);

extern int util_daad_get_limit_objects(void);
extern int util_daad_get_limit_flags(void);

extern int util_undaad_unpaws_is_quill(void);


extern z80_byte util_daad_get_flag_value(z80_byte index);

extern void util_daad_locate_word(z80_byte numero_palabra_buscar,z80_byte tipo_palabra_buscar,char *texto_destino);
extern void util_daad_paws_locate_word(z80_byte numero_palabra_buscar,z80_byte tipo_palabra_buscar,char *texto_destino);
extern int util_paws_dump_vocabulary_tostring(int tipo,char *texto,int max_string);
extern void util_paws_locate_word(z80_byte numero_palabra_buscar,z80_byte tipo_palabra_buscar,char *texto_destino);

extern z80_byte util_daad_get_object_value(z80_byte index);

extern void util_daad_put_flag_value(z80_byte index,z80_byte value);
extern void util_daad_put_object_value(z80_byte index,z80_byte value);

extern z80_int util_daad_get_start_objects_names(void);
extern z80_int util_daad_get_num_objects_description(void);

extern void util_daad_get_object_description(z80_byte index,char *texto);



extern int util_daad_is_in_parser(void);

extern void util_daad_get_message_table_lookup(z80_byte index,z80_int table_dir,char *texto,int limite_mensajes);

extern void util_daad_get_compressed_message(z80_byte index,char *texto);

extern z80_int util_daad_get_num_user_messages(void);
extern z80_int util_daad_get_num_sys_messages(void);
extern z80_int util_daad_get_num_locat_messages(void);
extern z80_int util_daad_get_total_graphics(void);

extern void util_daad_get_user_message(z80_byte index,char *texto);
extern void util_daad_get_sys_message(z80_byte index,char *texto);
extern void util_daad_get_locat_message(z80_byte index,char *texto);

extern int util_daad_condact_uses_message(void);

extern void util_daad_get_condact_message(char *buffer);

extern z80_int util_daad_get_graphics_attr(z80_byte location,int *ink,int *paper,int *is_picture);
extern int util_daad_has_graphics(void);
extern z80_int util_gac_get_graphics_location(int location,int *location_id);
extern void util_gac_get_graphics_size(int location,int *location_commands,int *location_size);
extern z80_int util_gac_daad_get_total_graphics(void);
extern int util_gac_detect(void);
extern int util_gac_get_index_location_by_id(int location_id);

extern z80_int util_daad_get_start_graphics(void);
extern z80_int util_daad_get_start_graphics_attr(void);
extern void util_unpaws_get_maintop_mainattr(z80_int *final_maintop,z80_int *final_mainattr,int *final_quillversion);
extern z80_int util_daad_get_graphics_location(z80_byte location);

extern z80_byte daad_peek(z80_int dir);
extern void daad_poke(z80_int dir,z80_byte value);
extern z80_int util_daad_get_pc_parser(void);
extern z80_int util_paws_get_pc_parser(void); 
extern void util_daad_get_version_daad(int *official_version,int *version_pointers);

extern void util_unpaws_daad_get_version_string(char *texto);

extern void debug_get_daad_breakpoint_string(char *texto);

extern void debug_get_daad_step_breakpoint_string(char *texto);

extern void debug_get_daad_runto_parse_string(char *texto);

extern z80_byte chardetect_convert_daad_accents(z80_byte c);
extern z80_byte chardetect_convert_paws_accents(z80_byte c);

struct s_daad_paws_contacts {
  int parametros;
  char nombre[10];
};

//Estructura para guardar la parte derecha de la vista de daad, si muestra flag o objeto y cual

struct s_debug_daad_flag_object {
	int tipo; //0=flag, 1=object
	z80_byte indice; //cual
};

extern void menu_debug_daad_check_init_flagobject(void);

extern void menu_debug_daad_string_flagobject(z80_byte num_linea,char *destino);

extern struct s_debug_daad_flag_object debug_daad_flag_object[];

extern struct s_daad_paws_contacts paws_contacts_array[];

extern struct s_daad_paws_contacts daad_contacts_array[];

extern void menu_debug_daad_init_flagobject(void);

extern void textadventure_generate_connections_table(void);

extern void init_textadventure_connections_table(void);

extern void textadventure_follow_connections(int follow_rooms_no_connections);

extern int textadventure_get_size_map(int mapa,int z,int *ancho,int *alto,int *min_x,int *max_x,int *min_y,int *max_y,int si_todos_mapas);


//define para una localidad, a donde puede ir
struct text_adventure_conn {
    //destino a donde va cada posicion. -1 si no permite ir
    int north,south,east,west,northwest,northeast,southwest,southeast,up,down;

    //destinos dudosos que llevan a una direccion que no deberia estar ahi
    int dudoso_north,dudoso_south,dudoso_east,dudoso_west,dudoso_northwest,dudoso_northeast,dudoso_southwest,dudoso_southeast,dudoso_up,dudoso_down;

    //mi posicion. Esto se recorre con la funcion recursiva
    int x,y,z;

    //dice si la hemos recorrido con la funcion recursiva
    int recorrida;

    //numero de mapa conectado entre si. cada mapa no conectado tiene diferente identificador. Validos desde el 1 en adelante
    int mapa;

    //dice si hemos entrado jugando
    int entrado_jugando;

    //si esta es una habitacion que se entra desde un destino dudoso
    int habitacion_dudosa;
};

extern struct text_adventure_conn text_adventure_connections_table[];

extern int util_textadventure_get_current_location(void);
extern int util_textadventure_get_current_location_flag(void);
extern int util_textdaventure_dump_connections(char *texto,int max_string);
extern void textadventure_debug_show_map(void);
extern int textadventure_room_has_exits(int i);
extern int util_textadventure_is_daad_quill_paws(void);
extern void init_textadventure_entrada_jugando(void);

extern z80_bit textadv_location_desc_enabled;
extern char textimage_filter_program[];
extern int textadv_location_total_conversions;
extern int max_textadv_location_desc_no_char_counter;
extern int max_textadv_location_desc_counter;
extern void textadv_location_add_char(z80_byte c);
extern int textimage_filter_program_check_spaces(void);
extern void textadv_location_desc_read_keyboard_port(void);
extern void textadv_location_desc_disable(void);
extern void textadv_location_desc_enable(void);
extern void textadv_location_timer_event(void);

//Tipo de deteccion de cambio de habitacion, aparte del cls
#define TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_CLS 0
#define TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_ROOM_NUMBER 1
#define TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_CLS_AND_ROOM_NUMBER 2

extern int textadv_location_additional_room_change_method;
extern char *textadv_location_additional_room_change_method_strings[];
extern int textadv_location_set_method_by_string(char *s);

extern int textadv_location_desc_last_image_generated_min;
extern void textadv_location_print_method_strings(void);
extern void textadv_location_change_method(void);


#endif
