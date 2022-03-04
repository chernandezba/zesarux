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

#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include "cpu.h"


extern char *asm_return_op_ops(char *origen,char *opcode,char *primer_op,char *segundo_op);

extern int assemble_opcode(int direccion_destino,char *texto,z80_byte *destino);

//maximo tamanyo que un opcode puede generar al ensamblar
#define MAX_DESTINO_ENSAMBLADO 255

#endif
