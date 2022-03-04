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

#ifndef AUTOSELECTOPTIONS_H
#define AUTOSELECTOPTIONS_H


extern void set_snaptape_fileoptions(char *filename);
extern void set_snaptape_filemachine(char *filename);

extern int tape_options_set_first_message_counter;
extern void delete_tape_options_set_first_message(void);
extern int tape_options_set_second_message_counter;
extern void delete_tape_options_set_second_message(void);
extern int tape_options_set_first_message_counter;
extern int tape_options_set_second_message_counter;
 
extern char *mostrar_footer_game_name;
extern void autoselect_options_put_footer(void);
extern char *mostrar_footer_second_message;

extern char mostrar_footer_first_message[];
extern char mostrar_footer_first_message_mostrado[];

extern int indice_first_message_mostrado;

extern int indice_second_message_mostrado;

extern void tape_options_corta_a_32(char *s);
extern void put_footer_first_message(char *mensaje);


#define AUTOSELECTOPTIONS_MAX_FOOTER_LENGTH 255




#endif
