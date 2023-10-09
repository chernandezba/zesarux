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

#ifndef UTILS_H
#define UTILS_H

#ifdef __FreeBSD__
#include <limits.h>
#endif

#include "cpu.h"
#include "compileoptions.h"
#include "zxvision.h"
#include "expression_parser.h"

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef MINGW
#include <stdlib.h>
#define PATH_MAX MAX_PATH
#define NAME_MAX MAX_PATH
#endif



extern void util_get_file_extension(char *filename,char *extension);
extern void util_get_file_no_directory(char *filename,char *file_no_dir);
extern void util_get_file_without_extension(char *filename,char *filename_without_extension);
extern void util_get_complete_path(char *dir,char *name,char *fullpath);


extern int util_compare_file_extension(char *filename,char *extension_compare);

extern void util_get_dir(char *ruta,char *directorio);

extern void set_symshift(void);

extern void clear_symshift(void);

extern void open_sharedfile(char *archivo,FILE **f);

#define MAX_COMPILE_INFO_LENGTH 4096

//2000 parametros
#define MAX_PARAMETERS_CONFIG_FILE 8000

//256 kb
#define MAX_SIZE_CONFIG_FILE 1048576

extern void get_compile_info(char *s);

extern void show_compile_info(void);

extern FILE *ptr_input_file_keyboard;
extern char input_file_keyboard_name_buffer[];
extern char *input_file_keyboard_name;
extern z80_bit input_file_keyboard_inserted;
extern int input_file_keyboard_delay;
extern int input_file_keyboard_delay_counter;
extern z80_bit input_file_keyboard_pending_next;
extern unsigned char input_file_keyboard_last_key;
extern z80_bit input_file_keyboard_is_pause;
extern z80_bit input_file_keyboard_send_pause;
extern z80_bit input_file_keyboard_turbo;


extern void insert_input_file_keyboard(void);
extern void eject_input_file_keyboard(void);
extern int input_file_keyboard_init(void);

extern void reset_keyboard_ports(void);
extern void ascii_to_keyboard_port(unsigned tecla);
extern void ascii_to_keyboard_port_set_clear(unsigned tecla,int pressrelease);
extern void input_file_keyboard_get_key(void);
extern void input_file_keyboard_close(void);


#ifdef EMULATE_CPU_STATS
	extern unsigned int stats_codsinpr[];
	extern unsigned int stats_codpred[];
	extern unsigned int stats_codprcb[];
	extern unsigned int stats_codprdd[];
	extern unsigned int stats_codprfd[];
	extern unsigned int stats_codprddcb[];
	extern unsigned int stats_codprfdcb[];

	extern void util_stats_increment_counter(unsigned int *stats_array,int index);
	extern unsigned int util_stats_get_counter(unsigned int *stats_array,int index);
	extern void util_stats_set_counter(unsigned int *stats_array,int index,unsigned int value);
	extern void util_stats_init(void);
	extern int util_stats_find_max_counter(unsigned int *stats_array);
	extern unsigned int util_stats_sum_all_counters(void);

#endif

struct x_tabla_teclado
{
        z80_byte *puerto;
        z80_byte mascara;
};

extern struct x_tabla_teclado tabla_teclado_numeros[];
extern struct x_tabla_teclado tabla_teclado_letras[];
extern void convert_numeros_letras_puerto_teclado(z80_byte tecla,int pressrelease);

extern struct x_tabla_teclado ql_tabla_teclado_letras[];
extern struct x_tabla_teclado ql_tabla_teclado_numeros[];

extern int get_rom_size(int machine);
extern int get_ram_size(void);
extern void configfile_parse(void);
extern char *configfile_argv[];
extern int configfile_argc;

extern z80_bit debug_parse_config_file;

//valores teclas en modo raw
#define RAWKEY_Escape 0x01
#define RAWKEY_minus 0x0c
#define RAWKEY_equal 0x0d
#define RAWKEY_BackSpace 0x0e
#define RAWKEY_Tab 0x0f
#define RAWKEY_bracket_left 0x1a
#define RAWKEY_bracket_right 0x1b
#define RAWKEY_Return 0x1c
#define RAWKEY_Control_L 0x1d
#define RAWKEY_semicolon 0x27
#define RAWKEY_apostrophe 0x28
#define RAWKEY_grave 0x29
#define RAWKEY_Shift_L 0x2a
#define RAWKEY_backslash 0x2b
#define RAWKEY_comma 0x33
#define RAWKEY_period 0x34
#define RAWKEY_slash 0x35
#define RAWKEY_Keypad_Divide 0x35
#define RAWKEY_Shift_R 0x36

#define RAWKEY_Keypad_Multiply 0x37

#define RAWKEY_Alt_L 0x38
#define RAWKEY_Space 0x39

#define RAWKEY_Caps_Lock 0x3a

#define RAWKEY_F1 0x3b
#define RAWKEY_F2 0x3c
#define RAWKEY_F3 0x3d
#define RAWKEY_F4 0x3e
#define RAWKEY_F5 0x3f
#define RAWKEY_F6 0x40
#define RAWKEY_F7 0x41

#define RAWKEY_F8 0x42
#define RAWKEY_F9 0x43
#define RAWKEY_F10 0x44

#define RAWKEY_Num_Lock 0x45

#define RAWKEY_Scroll_Lock 0x46

//Estas home,up,left... hasta pgdown son las generadas en el teclado numerico
#define RAWKEY_Keypad_Home 0x47
#define RAWKEY_Keypad_Up 0x48
#define RAWKEY_Keypad_Page_Up 0x49
#define RAWKEY_Keypad_Subtract 0x4a
#define RAWKEY_Keypad_Left 0x4b
#define RAWKEY_Keypad_5 = 0x4c,
#define RAWKEY_Keypad_Right 0x4d
#define RAWKEY_Keypad_Add 0x4e
#define RAWKEY_Keypad_End 0x4f
#define RAWKEY_Keypad_Down 0x50
#define RAWKEY_Keypad_Page_Down 0x51
#define RAWKEY_Keypad_0 0x52
#define RAWKEY_Keypad_Period 0x53

//Tecla a la izquierda de la Z
#define RAWKEY_leftz 0x56


/*
47 (Keypad-7/Home), 48 (Keypad-8/Up), 49 (Keypad-9/PgUp)
4a (Keypad--)
4b (Keypad-4/Left), 4c (Keypad-5), 4d (Keypad-6/Right), 4e (Keypad-+)
4f (Keypad-1/End), 50 (Keypad-2/Down), 51 (Keypad-3/PgDn)
52 (Keypad-0/Ins), 53 (Keypad-./Del)

*/

//En raspberry:

#define RAWKEY_RPI_Home 0x66
#define RAWKEY_RPI_Up 0x67
#define RAWKEY_RPI_Left 0x69
#define RAWKEY_RPI_Right 0x6a
#define RAWKEY_RPI_End 0x6b
#define RAWKEY_RPI_Down 0x6c


//#define RAWKEY_Control_R 0xe01d
//#define RAWKEY_Alt_R 0xe038



extern int quickload(char *nombre);
extern int quickload_valid_extension(char *nombre);
extern void insert_tape_cmdline(char *s);
extern void insert_snap_cmdline(char *s);

extern int si_existe_archivo(char *nombre);

extern long long int get_file_size(char *nombre);

extern long long int get_size_human_friendly(long long int tamanyo,char *sufijo);

extern int lee_archivo(char *nombre,char *buffer,int max_longitud);

extern int util_get_configfile_name(char *configfile);

//extern void util_print_second_overlay(char *texto, int x, int y);

//valores usados en funcion util_set_reset_mouse
enum util_mouse_buttons
{
	UTIL_MOUSE_LEFT_BUTTON,
	UTIL_MOUSE_RIGHT_BUTTON
};


extern void util_set_reset_mouse(enum util_mouse_buttons boton,int pressrelease);

//valores usados en funcion util_set_reset_key
enum util_teclas
{
	UTIL_KEY_NONE=0,  //None se usa en teclado chloe

	UTIL_KEY_SPACE=128,  //128 en adelante para no entrar en conflicto con teclas ascii <128
	UTIL_KEY_ENTER,
    UTIL_KEY_HOME,
    UTIL_KEY_END,
    UTIL_KEY_DEL,
	UTIL_KEY_SHIFT_L,
	UTIL_KEY_SHIFT_R,
	UTIL_KEY_CAPS_SHIFT,
	UTIL_KEY_ALT_L,
	UTIL_KEY_ALT_R,
	UTIL_KEY_CONTROL_L,
	UTIL_KEY_CONTROL_R,
	UTIL_KEY_BACKSPACE,
	UTIL_KEY_FIRE,
	UTIL_KEY_LEFT,
	UTIL_KEY_RIGHT,
	UTIL_KEY_DOWN,
	UTIL_KEY_UP,
	UTIL_KEY_TAB,
	UTIL_KEY_CAPS_LOCK,
	UTIL_KEY_COMMA,
	UTIL_KEY_PERIOD,
	UTIL_KEY_F1,
	UTIL_KEY_F2,
	UTIL_KEY_F3,
	UTIL_KEY_F4,
	UTIL_KEY_F5,
	UTIL_KEY_F6,
	UTIL_KEY_F7,
	UTIL_KEY_F8,
	UTIL_KEY_F9,
	UTIL_KEY_F10,
	UTIL_KEY_F11,
	UTIL_KEY_F12,
	UTIL_KEY_F13,
	UTIL_KEY_F14,
	UTIL_KEY_F15,
	UTIL_KEY_ESC,
	UTIL_KEY_PAGE_UP,
	UTIL_KEY_PAGE_DOWN,
	UTIL_KEY_KP_PLUS,
    UTIL_KEY_KP_NUMLOCK,
    UTIL_KEY_KP_DIVIDE,
    UTIL_KEY_KP_MULTIPLY,
    UTIL_KEY_KP_MINUS,
	UTIL_KEY_KP0,
	UTIL_KEY_KP1,
	UTIL_KEY_KP2,
	UTIL_KEY_KP3,
	UTIL_KEY_KP4,
	UTIL_KEY_KP5,
	UTIL_KEY_KP6,
	UTIL_KEY_KP7,
	UTIL_KEY_KP8,
	UTIL_KEY_KP9,
	UTIL_KEY_KP_COMMA,
	UTIL_KEY_KP_ENTER,
	UTIL_KEY_WINKEY_L,
    UTIL_KEY_WINKEY_R,

	//Estos 5 son para enviar eventos de joystick mediante ZENG
	UTIL_KEY_JOY_FIRE,
	UTIL_KEY_JOY_UP,
	UTIL_KEY_JOY_DOWN,
	UTIL_KEY_JOY_LEFT,
	UTIL_KEY_JOY_RIGHT,

    //Esta es para enviar mediante ZENG una orden de reset todas teclas. Usado en driver curses
    UTIL_KEY_RESET_ALL
};

//valores usados en funcion util_set_reset_key_z88_keymap
//son teclas que vienen de conversion de keymap
enum util_teclas_z88_keymap
{
        UTIL_KEY_Z88_MINUS,
	UTIL_KEY_Z88_EQUAL,
	UTIL_KEY_Z88_BACKSLASH,
	UTIL_KEY_Z88_BRACKET_LEFT,
	UTIL_KEY_Z88_BRACKET_RIGHT,
	UTIL_KEY_Z88_SEMICOLON,
	UTIL_KEY_Z88_APOSTROPHE,
	UTIL_KEY_Z88_POUND,
	UTIL_KEY_Z88_COMMA,
	UTIL_KEY_Z88_PERIOD,
	UTIL_KEY_Z88_SLASH
};


//Usados en mapeo de keymap que deberia ser comun a todas maquinas (z88, cpc, chloe, sam)
//De momento solo usado en sam
enum util_teclas_common_keymap
{
        UTIL_KEY_COMMON_KEYMAP_MINUS,
        UTIL_KEY_COMMON_KEYMAP_EQUAL,
        UTIL_KEY_COMMON_KEYMAP_BACKSLASH,
        UTIL_KEY_COMMON_KEYMAP_BRACKET_LEFT,
        UTIL_KEY_COMMON_KEYMAP_BRACKET_RIGHT,
        UTIL_KEY_COMMON_KEYMAP_SEMICOLON,
        UTIL_KEY_COMMON_KEYMAP_APOSTROPHE,
        UTIL_KEY_COMMON_KEYMAP_POUND,
        UTIL_KEY_COMMON_KEYMAP_COMMA,
        UTIL_KEY_COMMON_KEYMAP_PERIOD,
        UTIL_KEY_COMMON_KEYMAP_SLASH,
        UTIL_KEY_COMMON_KEYMAP_LEFTZ
};

//valores usados en funcion util_set_reset_key_cpc_keymap
//son teclas que vienen de conversion de keymap
enum util_teclas_cpc_keymap
{
        UTIL_KEY_CPC_MINUS,
        UTIL_KEY_CPC_CIRCUNFLEJO,
	UTIL_KEY_CPC_ARROBA,
        UTIL_KEY_CPC_BRACKET_LEFT,
	UTIL_KEY_CPC_COLON,
        UTIL_KEY_CPC_SEMICOLON,
        UTIL_KEY_CPC_BRACKET_RIGHT,
        UTIL_KEY_CPC_COMMA,
        UTIL_KEY_CPC_PERIOD,
        UTIL_KEY_CPC_SLASH,
        UTIL_KEY_CPC_BACKSLASH
};


/*
enum util_teclas_msx_keymap
{
        UTIL_KEY_MSX_MINUS

};
*/

enum util_teclas_chloe_keymap
{
        UTIL_KEY_CHLOE_MINUS,
        UTIL_KEY_CHLOE_EQUAL,
        UTIL_KEY_CHLOE_BACKSLASH,
        UTIL_KEY_CHLOE_BRACKET_LEFT,
        UTIL_KEY_CHLOE_BRACKET_RIGHT,
        UTIL_KEY_CHLOE_SEMICOLON,
        UTIL_KEY_CHLOE_APOSTROPHE,
        UTIL_KEY_CHLOE_POUND,
        UTIL_KEY_CHLOE_COMMA,
        UTIL_KEY_CHLOE_PERIOD,
        UTIL_KEY_CHLOE_SLASH,
        UTIL_KEY_CHLOE_LEFTZ
};


extern void util_set_reset_key(enum util_teclas tecla,int pressrelease);
extern void util_set_reset_key_z88_keymap(enum util_teclas_z88_keymap tecla,int pressrelease);
extern void util_set_reset_key_cpc_keymap(enum util_teclas_cpc_keymap tecla,int pressrelease);
extern void util_set_reset_key_chloe_keymap(enum util_teclas_chloe_keymap tecla,int pressrelease);
extern void util_set_reset_key_common_keymap(enum util_teclas_common_keymap tecla,int pressrelease);



extern unsigned int parse_string_to_number(char *texto);
extern unsigned int parse_string_to_number_get_type(char *texto,enum token_parser_formato *tipo_valor);

//#define TMPDIR_BASE "/tmp/zesarux"

extern char *get_tmpdir_base(void);

extern int convert_smp_to_rwa(char *origen, char *destino);
extern int convert_smp_to_rwa_tmpdir(char *origen, char *destino);

extern int convert_wav_to_rwa(char *origen, char *destino);
extern int convert_wav_to_rwa_tmpdir(char *origen, char *destino);

extern int convert_rwa_to_wav(char *origen, char *destino);

extern int convert_tzx_to_rwa(char *origen, char *destino);
extern int convert_tzx_to_rwa_tmpdir(char *origen, char *destino);
//extern int convert_tzx_to_wav(char *origen, char *destino);

extern int convert_wav_to_raw_tmpdir(char *origen, char *destino);

extern int convert_o_to_rwa(char *origen, char *destino);
extern int convert_o_to_rwa_tmpdir(char *origen, char *destino);

extern int convert_p_to_rwa(char *origen, char *destino);
extern int convert_p_to_rwa_tmpdir(char *origen, char *destino);

extern int convert_tap_to_rwa_tmpdir(char *origen, char *destino);
extern int convert_tap_to_rwa(char *origen, char *destino);
//extern int convert_tap_to_wav(char *origen, char *destino);

extern int convert_any_to_wav(char *origen, char *destino);

extern int convert_hdf_to_raw(char *origen, char *destino);

extern z80_bit quickload_guessing_tzx_type;

extern int load_binary_file(char *binary_file_load,int valor_leido_direccion,int valor_leido_longitud);
extern int save_binary_file(char *filename,int valor_leido_direccion,int valor_leido_longitud);
extern void parse_customfile_options(void);
extern void parse_custom_file_config(char *archivo);

extern int set_machine_type_by_name(char *machine_name);
extern void open_sharedfile_write(char *archivo,FILE **f);
extern int scandir_mingw(const char *dir, struct dirent ***namelist,
              int (*filter)(const struct dirent *),
              int (*compar)(const struct dirent **, const struct dirent **));

extern void customconfig_help(void);

extern char letra_mayuscula(char c);

extern char letra_minuscula(char c);

extern void string_a_minusculas(char *origen, char *destino);

extern void string_a_mayusculas(char *origen, char *destino);

extern int si_ruta_absoluta(char *ruta);

extern int get_file_type(char *nombre);

extern char external_tool_sox[];
//extern char external_tool_unzip[];
extern char external_tool_gunzip[];
extern char external_tool_tar[];
extern char external_tool_unrar[];

extern void convert_relative_to_absolute(char *relative_path,char *final_path);
extern void convert_realtape_to_po(char *filename, char *archivo_destino, char *texto_info_output,int si_load);

//Mas de 48kb de pokes no tiene sentido
#define MAX_LINEAS_POK_FILE 49152

#define MAX_LENGTH_LINE_POKE_FILE 90

struct s_pokfile
{
        int indice_accion;
        char texto[MAX_LENGTH_LINE_POKE_FILE+1];
	z80_byte banco;
	z80_int direccion;
	z80_byte valor;
	z80_byte valor_orig;
};


extern int util_parse_pok_file(char *file,struct s_pokfile **tabla_pokes);

extern int util_poke(z80_byte banco,z80_int direccion,z80_byte valor);

extern int util_busca_archivo_nocase(char *archivo,char *directorio,char *nombreencontrado);

extern void set_peek_byte_function_spoolturbo(void);
extern void reset_peek_byte_function_spoolturbo(void);


extern void set_poke_byte_function_writerom(void);
extern void reset_poke_byte_function_writerom(void);

/*
Lista fabricantes
Amstrad
Ascii Corp
Cambridge Computers
Chloe Corporation
Coleco Industries
Investronica
Jupiter Cantab
Mario Prato
Microdigital Eletronica
Miles Gordon Technology
NedoPC
New Horizons
Pentagon
Science of Cambridge
Sega
Sinclair Research
Spectravideo Intl
Timex Computers
Timex Sinclair
TS Labs
VTrucco/FB Labs
ZXUno Team
*/

#define TOTAL_FABRICANTES 22


#define FABRICANTE_AMSTRAD                      0
#define FABRICANTE_ASCII_CORP                   1
#define FABRICANTE_CAMBRIDGE_COMPUTERS          2
#define FABRICANTE_CHLOE_CORPORATION            3
#define FABRICANTE_COLECO_INDUSTRIES            4
#define FABRICANTE_INVESTRONICA                 5
#define FABRICANTE_JUPITER_CANTAB               6
#define FABRICANTE_MARIOPRATO                   7
#define FABRICANTE_MICRODIGITAL_ELECTRONICA     8
#define FABRICANTE_MILES_GORDON                 9
#define FABRICANTE_NEDOPC                       10
#define FABRICANTE_NEW_HORIZONS                 11
#define FABRICANTE_PENTAGON                     12
#define FABRICANTE_SCIENCE_OF_CAMBRIDGE         13
#define FABRICANTE_SEGA                         14
#define FABRICANTE_SINCLAIR                     15
#define FABRICANTE_SPECTRAVIDEO_INTERNATIONAL   16
#define FABRICANTE_TIMEX_COMPUTERS              17
#define FABRICANTE_TIMEX_SINCLAIR               18
#define FABRICANTE_TSLABS                       19
#define FABRICANTE_TBBLUE                       20
#define FABRICANTE_ZXUNO_TEAM                   21



extern char *array_fabricantes[];
extern char *array_fabricantes_hotkey[];
extern char array_fabricantes_hotkey_letra[];

extern int *return_maquinas_fabricante(int fabricante);

extern int return_fabricante_maquina(int maquina);

extern int return_machine_position(int *array_maquinas,int id_maquina);


struct s_tecla_redefinida
{
        z80_byte tecla_original;   //Tecla a redefinir. Si no, vale 0
        z80_byte tecla_redefinida; //Tecla resultante. Si no, da igual su valor
};

typedef struct s_tecla_redefinida tecla_redefinida;


//Tabla para subzonas. Se usa en array y un elemento de 0,0,"" indica el ultimo elemento
struct s_subzone_info
{
	int inicio;
	int fin;
	char nombre[33];
};

typedef struct s_subzone_info subzone_info;


//Tabla para guardar configuracion de geometria de ventanas
struct s_saved_config_window_geometry
{
	char nombre[MAX_NAME_WINDOW_GEOMETRY];
	int x,y,ancho,alto,is_minimized,is_maximized,width_before_max_min_imize,height_before_max_min_imize;
};

typedef struct s_saved_config_window_geometry saved_config_window_geometry;

extern saved_config_window_geometry saved_config_window_geometry_array[];

extern int total_config_window_geometry;

//extern int legacy_util_find_window_geometry(char *nombre,int *x,int *y,int *ancho,int *alto,int *is_minimized,int *width_before_max_min_imize,int *height_before_max_min_imize);
extern int util_find_window_geometry(char *nombre,int *x,int *y,int *ancho,int *alto,int *is_minimized,int *is_maximized,int *width_before_max_min_imize,int *height_before_max_min_imize);

extern int util_add_window_geometry(char *nombre,int x,int y,int ancho,int alto,int is_minimized,int is_maximized,int width_before_max_min_imize,int height_before_max_min_imize);

extern void util_add_window_geometry_compact(zxvision_window *ventana);

extern void util_clear_all_windows_geometry(void);


#define MAX_TECLAS_REDEFINIDAS 10
extern tecla_redefinida lista_teclas_redefinidas[];


extern void clear_lista_teclas_redefinidas(void);

extern int util_add_redefinir_tecla(z80_byte p_tecla_original, z80_byte p_tecla_redefinida);

extern void clear_lista_teclas_redefinidas(void);

extern void util_set_reset_key_continue(enum util_teclas tecla,int pressrelease);

extern void util_set_reset_key_continue_after_zeng(enum util_teclas tecla,int pressrelease);

extern void convert_numeros_letras_puerto_teclado_continue(z80_byte tecla,int pressrelease);



extern int util_tape_tap_get_info(z80_byte *tape,char *texto,int origin_tap);

#define DEFAULT_ZESARUX_CONFIG_FILE ".zesaruxrc"

extern char *customconfigfile;

extern void get_machine_config_name_by_number(char *machine_name,int machine_number);

extern int util_write_configfile(void);

extern int util_create_sample_configfile(int additional);

extern z80_bit save_configuration_file_on_exit;

extern z80_byte peek_byte_z80_moto(unsigned int address);
extern void poke_byte_z80_moto(unsigned int address,z80_byte valor);

extern unsigned int get_ql_pc(void);

extern unsigned int adjust_address_space_cpu(unsigned int direccion);

extern unsigned int get_pc_register(void);

extern void convert_signed_unsigned(char *origen, unsigned char *destino,int longitud);

extern int uncompress_gz(char *origen,char *destino);

extern char *util_strcasestr(char *string, char *string_a_buscar);

extern void util_sprintf_address_hex(menu_z80_moto_int p,char *string_address);

extern int si_menu_mouse_activado(void);

//extern long long int parse_string_to_long_number(char *texto);

extern int util_parse_commands_argvc(char *texto, char *parm_argv[], int maximo);

extern int util_parse_commands_argvc_comillas(char *texto, char *parm_argv[], int maximo);

extern int get_machine_id_by_name(char *machine_name);
extern char **get_machine_icon_by_name(char *machine_name);
extern void get_machine_list_whitespace(void);

extern void util_truncate_file(char *filename);

extern int util_write_pbm_file(char *archivo, int ancho, int alto, int ppb, z80_byte *source);
extern int util_write_sprite_c_file(char *archivo, int ancho, int alto, int ppb, z80_byte *source);

extern int get_file_type_from_stat(struct stat *f);

extern int get_file_type_from_name(char *nombre);

extern int file_is_directory(char *nombre);

extern int get_file_date_from_name(char *nombre,int *hora,int *minuto,int *segundo,int *dia,int *mes,int *anyo);

extern int get_file_date_from_stat(struct stat *buf_stat,int *hora,int *minuto,int *segundo,int *dia,int *mes,int *anyo);

#define MACHINE_MAX_MEMORY_ZONES 1000

//Maximo 30 caracteres de nombre de zona
#define MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT 30

extern z80_byte *machine_get_memory_zone_pointer(int zone, int address);
extern unsigned int machine_get_memory_zone_attrib(int zone, int *readwrite);
extern void machine_get_memory_zone_name(int zone, char *name);
extern int machine_get_next_available_memory_zone(int zone);
extern void machine_get_memory_subzone_name(int zone, int machine_id, int address, char *name);
extern subzone_info *machine_get_memory_subzone_array(int zone, int machine_id);

extern void util_delete(char *filename);

extern z80_long_int util_crc32_calculation(z80_long_int crc, z80_byte *buf, size_t len);


extern int util_return_ceros_byte(z80_byte valor);

extern void util_byte_to_binary(z80_byte value,char *texto);

extern void util_copy_file(char *source_file, char *destination_file);

extern void util_set_reset_key_convert_recreated_yesno(enum util_teclas tecla,int pressrelease,int convertrecreated);

extern void convert_numeros_letras_puerto_teclado_continue_after_recreated(z80_byte tecla,int pressrelease);

extern int util_load_editionnamegame(void);
extern int find_sharedfile(char *archivo,char *ruta_final);
extern int si_existe_editionnamegame(char *nombre_final);

extern int util_extract_mdv(char *mdvname, char *dest_dir);
extern int util_extract_hdf(char *hdfname, char *dest_dir);

extern void util_save_file(z80_byte *origin, long long int tamanyo_origen, char *destination_file);
extern int util_load_file_bytes(z80_byte *taperead,char *filename,int total_leer);

extern void util_string_replace_char(char *s,char orig,char dest);

extern int util_add_string_newline(char *destination,char *text_to_add);

extern void util_binary_to_hex(z80_byte *origen, char *destino, int longitud_max, int longitud);

extern void util_binary_to_ascii(z80_byte *origen, char *destino, int longitud_max, int longitud);

extern void util_tape_get_info_tapeblock(z80_byte *tape,z80_byte flag,z80_int longitud,char *texto);

extern void util_ascii_to_binary(int valor_origen,char *destino,int longitud_max);

//extern void util_file_save(char *filename,z80_byte *puntero, long long int tamanyo);

extern z80_64bit util_get_seconds(void);

extern int util_get_byte_repetitions(z80_byte *memoria,int longitud,z80_byte *byte_repetido);

extern int util_compress_data_repetitions(z80_byte *origen,z80_byte *destino,int longitud,z80_byte magic_byte);

extern int util_uncompress_data_repetitions(z80_byte *origen,z80_byte *destino,int longitud,z80_byte magic_byte);

extern char emulator_tmpdir_set_by_user[];

extern void util_spectrumscreen_get_xy(z80_int dir,int *xdest,int *ydest);

extern int util_if_open_just_menu(void);

extern int util_if_open_just_menu_times;

extern unsigned int util_if_open_just_menu_counter;

extern unsigned int util_if_open_just_menu_initial_counter;
extern void util_convert_scr_sprite(z80_byte *origen,z80_byte *destino);

extern int util_get_pixel_color_scr(z80_byte *scrfile,int x,int y);

extern int util_convert_sna_to_scr(char *filename,char *archivo_destino);

extern int util_convert_sp_to_scr(char *filename,char *archivo_destino);

extern int util_convert_z80_to_scr(char *filename,char *archivo_destino);

extern int util_convert_p_to_scr(char *filename,char *archivo_destino);

extern int util_convert_zsf_to_scr(char *filename,char *archivo_destino);

extern int util_convert_any_to_scr(char *filename,char *archivo_destino);

extern int util_get_absolute(int valor);

extern int util_get_sign(int valor);

extern int input_file_keyboard_is_playing(void);

extern z80_bit input_file_keyboard_playing;

extern void convert_to_rwa_common_tmp(char *origen, char *destino);

extern void util_tape_get_name_header(z80_byte *tape,char *texto);

extern int util_extract_tap(char *filename,char *tempdir,char *tzxfile,int tzx_turbo_rg);

extern int util_extract_ddh(char *filename,char *tempdir);

extern int util_extract_p(char *filename,char *tempdir);

extern int util_extract_o(char *filename,char *tempdir);

extern int util_extract_tzx(char *filename,char *tempdir,char *tapfile);

extern int util_extract_pzx(char *filename,char *tempdir,char *tapfile);

extern int util_extract_trd(char *filename,char *tempdir);

extern void util_file_append(char *filename,z80_byte *puntero, int tamanyo);

extern int util_dsk_get_blocks_entry_file(z80_byte *dsk_file_memory,int longitud_dsk,z80_byte *bloques,int entrada_obtener);
extern void util_dsk_getsectors_block(z80_byte *dsk_file_memory,int longitud_dsk,int bloque,int *sector1,int *pista1,int *sector2,int *pista2,int incremento_pista);

extern int util_extract_dsk(char *filename,char *tempdir);

extern int util_extract_z88_card(char *filename,char *tempdir);

extern int file_is_z88_basic(char *filename);

extern void util_save_game_config(char *filename);


extern int util_is_digit(char c);
extern int util_is_letter(char c);

extern int util_get_available_drives(char *texto);

extern int get_cpu_frequency(void);


extern void util_clear_final_spaces(char *orig,char *destination);

extern int util_concat_string(char *original,char *string_to_add,int limite);

extern void snapshot_get_date_time_string_human(char *texto);
extern void snapshot_get_date_time_string(char *texto);


#define MEMORY_ZONE_NUM_FILE_ZONE 16
#define MEMORY_ZONE_NUM_TBBLUE_COPPER 17
#define MEMORY_ZONE_NUM_TIMEX_EX 18
#define MEMORY_ZONE_NUM_TIMEX_DOCK 19

#define MEMORY_ZONE_NUM_DAAD_CONDACTS 20
#define MEMORY_ZONE_NUM_PAWS_CONDACTS 21
#define MEMORY_ZONE_DEBUG 22
#define MEMORY_ZONE_IFROM 23
#define MEMORY_ZONE_MSX_VRAM 24
#define MEMORY_ZONE_MSX_ALL_MEM 25
#define MEMORY_ZONE_COLECO_VRAM 26
#define MEMORY_ZONE_SG1000_VRAM 27

#define MEMORY_ZONE_SVI_VRAM 28
#define MEMORY_ZONE_SVI_ALL_MEM 29

#define MEMORY_ZONE_SAMRAM 30

#define MEMORY_ZONE_NUM_TBBLUE_SPRITES 31

#define MEMORY_ZONE_SMS_VRAM 32

#define MEMORY_ZONE_HILOW_ROM 33
#define MEMORY_ZONE_HILOW_RAM 34
#define MEMORY_ZONE_HILOW_DEVICE 35
#define MEMORY_ZONE_HILOW_CONVERT_READ 36

#define MEMORY_ZONE_HILOW_BARBANEGRA_ROM 37
#define MEMORY_ZONE_HILOW_BARBANEGRA_RAM 38
#define MEMORY_ZONE_TRANSTAPE_ROM 39
#define MEMORY_ZONE_TRANSTAPE_RAM 40
#define MEMORY_ZONE_MHPOKEADOR_RAM 41
#define MEMORY_ZONE_SPECMATE_ROM 42
#define MEMORY_ZONE_PHOENIX_ROM 43
#define MEMORY_ZONE_DEFCON_ROM 44
#define MEMORY_ZONE_RAMJET_ROM 45
#define MEMORY_ZONE_INTERFACE007_ROM 46
#define MEMORY_ZONE_DINAMID3_ROM 47
#define MEMORY_ZONE_DSK_SECTOR 48




extern void util_str_add_char(char *texto,int posicion,char letra);
extern void util_str_del_char(char *texto,int posicion);
extern int get_file_lines(char *filename);
extern void util_fill_string_character(char *buffer_linea,int longitud,z80_byte caracter);

extern char util_printable_char(char c);

extern char *util_read_line(char *origen,char *destino,int size_orig,int max_size_dest,int *leidos);
extern void util_normalize_name(char *texto);

extern int util_download_file(char *hostname,char *url,char *archivo,int use_ssl,int estimated_maximum_size,char *ssl_sni_host_name);
extern void util_normalize_query_http(char *orig,char *dest);

extern int util_extract_scl(char *sclname, char *dest_dir);
extern int util_extract_zip(char *zipname, char *dest_dir);

extern void util_realtape_browser(char *filename, char *texto_browser,int maxima_longitud_texto,char *tap_output,
                                    long *array_block_positions,int max_array_block_positions,int *codigo_retorno);

extern void util_get_host_url(char *url, char *host);
extern void util_get_url_no_host(char *url, char *url_no_host);
extern int util_url_is_https(char *url);

extern char util_return_valid_ascii_char(char c);

extern int convert_pzx_to_rwa_tmpdir(char *origen, char *destino);
extern int convert_pzx_to_rwa(char *origen, char *destino);

#define PZX_CURRENT_MAJOR_VERSION 1
#define PZX_CURRENT_MINOR_VERSION 0

extern int convert_scr_to_tap(char *origen, char *destino);
extern int convert_scr_to_txt(char *origen, char *destino);

extern z80_byte get_memory_checksum_spectrum(z80_byte crc,z80_byte *origen,int longitud);

extern void util_store_value_little_endian(z80_byte *destination,z80_int value);

extern z80_int util_get_value_little_endian(z80_byte *origin);

extern void util_get_home_dir(char *homedir);

extern void util_write_screen_bmp(char *archivo);

extern void util_bmp_load_palette(z80_byte *mem,int indice_inicio_color);

//extern int util_bmp_load_palette_changed_palette;
extern int util_bmp_load_palette_changed_palette_primary;
extern int util_bmp_load_palette_changed_palette_second;

extern z80_byte *util_load_bmp_file(char *archivo,int id_paleta);

extern void util_rotate_file(char *filename,int archivos);

extern int util_convert_utf_charset(char *origen,z80_byte *final,int longitud_texto);

extern const char *spectrum_colour_names[];

extern void util_write_long_value(z80_byte *destino,unsigned int valor);
extern unsigned int util_read_long_value(z80_byte *origen);

extern int util_get_input_file_keyboard_ms(void);

extern char fatfs_disk_zero_path[];

extern int util_path_is_prefix_mmc_fatfs(char *dir);
extern int util_path_is_windows_with_drive(char *dir);
extern int util_path_is_mmc_fatfs(char *dir);

#define MAX_COPY_FILES_TO_MMC 10
extern int util_copy_files_to_mmc_addlist(char *source, char *destination);
extern void util_copy_files_to_mmc_doit(void);

extern z80_byte util_get_byte_protect(z80_byte *memoria,int total_size,int offset);
extern void util_memcpy_protect_origin(z80_byte *destino,z80_byte *memoria,int total_size,int offset,int total_copiar);
extern int util_abs(int v);
extern int util_sign(int v);
extern int util_get_cosine(int degrees);
extern int util_get_sine(int degrees);
extern int util_compare_bytes_address(menu_z80_moto_int dir,int *lista,int total_items);
extern z80_64bit util_sqrt(z80_64bit number,int *result_type);
extern int util_get_acosine(int cosine);

extern void util_print_minutes_seconds(int segundos_totales, char *texto);

extern void toggle_flash_state(void);

extern void *util_malloc(int total,char *mensaje_panic);
extern void *util_malloc_fill(int total,char *mensaje_panic,z80_byte value);
extern void *util_malloc_max_texto_generic_message(char *mensaje_panic);
extern void *util_malloc_max_texto_browser(void);

extern int util_random_noise;
extern void util_generate_random_noise(int pressrelease);

extern void util_trunc_name_right(char *texto,int max_length,int char_buffer_size);

extern void util_drag_drop_file(char *filepath);

extern int util_extract_preview_file_expandable(char *nombre,char *tmpdir);
extern void util_extract_preview_file_simple(char *nombre,char *tmpdir,char *tmpfile_scr,int file_size);

extern int util_get_extract_preview_type_file(char *nombre,long long int file_size);
extern void util_normalize_file_name_for_temp_dir(char *nombre);

extern int util_if_filesystem_plusidedos(z80_byte *memoria,int total_size);
extern int util_if_filesystem_fat16(z80_byte *memoria,int total_size);

extern int util_parity(z80_byte value);

extern void util_get_emulator_version_number(char *buffer);

extern char util_byte_to_hex_nibble(z80_byte valor);
extern z80_byte util_hex_nibble_to_byte(char letra);

//Contando el NULL del final
#define UTIL_SCANF_HISTORY_MAX_LINES 11

extern void util_scanf_history_insert(char **textos_historial,char *texto);
extern int util_scanf_history_get_total_lines(char **textos_historial);

extern void util_get_operating_system_release(char *destino,int maximo);



#endif
