;    SPED - SPectrum Ensamblador Desensamblador / SuPer Ensamblador Desensamblador
;    Copyright (C) 1995 Cesar Hernandez Bano
;
;    This file is part of ZEsarUX.
;
;    SPED is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.


;Paging routines that go to address 23296
;       LST+
RUTPAG
       ORG  23296
RAMNOR PUSH AF
       PUSH BC
PAGNOR LD   A,17
RAMNO2 LD   BC,32765
       OUT  (C),A
       POP  BC
       POP  AF
       RET
RAMEXT PUSH AF
       PUSH BC
NUMPAG LD   A,16
       JR   RAMNO2
ILDIR  CALL RAMEXT
       LDIR
       JR   RAMNOR
ILDDR  CALL RAMEXT
       LDDR
       JR   RAMNOR
LOAD0  CALL RAMEXT
       INC  D
       EX   AF,AF'
       DEC  D
       DI
       CALL 1378
       JR   RAMNOR
SAVE   CALL RAMEXT
       CALL 1222
       JR   RAMNOR
ILDAHL CALL RAMEXT
ILDAH2 LD   A,(HL)
       JR   RAMNOR
ILDHLA CALL RAMEXT
ILDHL2 LD   (HL),A
       JR   RAMNOR
CDIL0  CALL RAMEXT
CDIL1  LD   A,B
       CP   D
       JR   NZ,CDIL2
       LD   A,C
       CP   E
       JR   NZ,CDIL2
       CALL RAMNOR
       JP   CADIL5
CDIL2  LD   A,HX
       CP   H
       JR   NZ,CDIL3
       LD   A,LX
       CP   L
       JR   NZ,CDIL3
       SCF
       JR   RAMNOR
CDIL3  INC  BC
CDIL4  LD   A,(HL)
       INC  HL
       CP   13
       JR   NZ,CDIL4
       JR   CDIL1
RAMETI PUSH AF
       PUSH BC
       LD   A,19
       JR   RAMNO2
ELDAHL CALL RAMETI
       JR   ILDAH2
ELDHLA CALL RAMETI
       JR   ILDHL2
RAMOBJ PUSH AF
       PUSH BC
       LD   A,20
       JR   RAMNO2
OLDAHL CALL RAMOBJ
       JR   ILDAH2
OLDHLA CALL RAMOBJ
       JR   ILDHL2
DRST8  SET  4,(IY-17)
DVUELV LD   (DREGSP),SP
       LD   SP,23552
       CALL RAMNOR
       JP   DVUEL1
DPOPR1 CALL RAMOBJ
       LD   SP,(DREGSP)
       RET
DBYSAL DEFS 7
DREGSP DEFW 0
DERR8  DEFW DRST8
