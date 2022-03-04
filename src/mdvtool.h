/*
    mdvtool.h: Tool from MIST emulator
    (c) 2015 by Till Harbaum
    https://github.com/mist-devel

    Copyright (c) 2015 Till Harbaum <till@harbaum.org>

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

#ifndef MDV_H
#define MDV_H

#define MDVTOOL_MAX_SECTORS 255

typedef struct {
  unsigned char preamble[12];   // 10*0, 2*ff
  unsigned char ff;  
  unsigned char snum;  
  char name[10];
  unsigned short rnd;
  unsigned short csum;
} __attribute__((packed)) hdr_t;

typedef struct {
  unsigned char bh_preamble[12];   // 10*0, 2*ff
  unsigned char file;
  unsigned char block;
  unsigned short bh_csum;

  unsigned char data_preamble[8];   // 6*0, 2*ff
  unsigned char data[512];
  unsigned short data_csum;

  unsigned char extra_byte[120];
} __attribute__((packed)) sector_t;

typedef struct {
  unsigned int length;
  unsigned char access;
  unsigned char type;        // 0=regular, 1=exec
  unsigned int info[2];
  unsigned short name_len;   // file name length
  char name[36];
  unsigned int last_update;
  unsigned int version;
  unsigned int last_backup;
} __attribute__((packed)) file_t;

typedef struct {
  hdr_t hdr;
  sector_t sec;
} mdv_entry_t;

typedef struct {
  int long_id;   // signature
  int extra_id;  // extra signature
  file_t file;
} __attribute__((packed)) zip_extra_qdos_t;

typedef struct {
  char flags;
  int last_modified;
  int last_access;
  int creation;
} __attribute__((packed)) zip_extra_ur_t;

extern int main_mdvtool(int argc, char **argv);

#endif // MDV_H
