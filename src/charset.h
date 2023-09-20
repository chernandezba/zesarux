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

#ifndef CHARSET_H
#define CHARSET_H

extern unsigned char char_set_zx81_no_ascii[];

extern unsigned char *char_set;
extern unsigned char char_set_spectrum[];
extern unsigned char char_set_cpc[];
extern unsigned char char_set_z88[];
extern unsigned char char_set_sam[];
extern unsigned char char_set_mansoftware[];
extern unsigned char char_set_ql[];
extern unsigned char char_set_retromac[];
extern unsigned char char_set_amigaos[];
extern unsigned char char_set_ataritos[];
extern unsigned char char_set_msx[];
extern unsigned char char_set_beos[];
extern unsigned char char_set_dos[];
extern unsigned char char_set_templeos[];
extern unsigned char char_set_customfile[];

extern char char_set_customfile_path[];

#define MAX_CHARSET_NAME 32

struct s_charset_list {
    char nombre[MAX_CHARSET_NAME];
    unsigned char *puntero;
};

extern struct s_charset_list charset_list[];
extern int user_charset;

extern void charset_retorna_nombres(void);

extern int get_charset_id_by_name(char *nombre);

extern void set_user_charset(void);

#define MAX_CHARSET_GRAPHIC 168

//Tamanyo de cada array para charset en menu, asi podemos saber si por error he metido mas elementos
//(aunque el compilador no me avisa cuando hay menos elementos, que seria deseable)
#define TOTAL_ASCII_CHARSET_ELEMENTS ((MAX_CHARSET_GRAPHIC-32+1)*8)


extern char *charset_icons_text[];

//El ancho efectivo del caracter. Se escribe con 1 espacio de mas
#define CHARSET_ICONS_ANCHO 3
#define CHARSET_ICONS_ALTO 5


#endif
