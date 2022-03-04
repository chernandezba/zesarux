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

        ORG  49152
	OUTPUT sped.bin



;
;SPED54  ,        ,       ~,
;   (C) CESAR HERNANDEZ BANO
;VERSION 1.3 EN


; V 1.0           (01/04/1995) - Preliminary version   - CESAR HERNANDEZ BANO
; V 1.1/SPED52    (11/07/1997) - Full working version  - CESAR HERNANDEZ BANO
; V 1.1 EN/SPED52 (17/08/2018) - English version       - TIM GILBERTS
; V 1.2 SPED53    (04/09/2018) - Bug fixed version     - CESAR HERNANDEZ BANO
; V 1.2 EN/SPED53 (10/09/2018) - Bug fixed version     - CESAR HERNANDEZ BANO
; V 1.3 EN/SPED54 (13/09/2018) - Bug fixed version     - CESAR HERNANDEZ BANO
;
;
;License: GNU GPL
;
FLAGS1 EQU  23582
FLAGS2 EQU  23583
OP11   EQU  23584
OP12   EQU  23585
OP13   EQU  23586
OP21   EQU  23587
OP22   EQU  23588
OP23   EQU  23589
AMAOP1 EQU  23590
AMAOP2 EQU  23591
FLAGS3 EQU  23592
FLAGS4 EQU  23593
       CALL COPPAG
       LD   SP,23552
       JP   INICIO
BUFORI EQU  65333
BUFDEB EQU  65130
ABUF   DEFS 34
CABOBJ DEFS 34
BUFEDI DEFS 32
CX     DEFB 0
CY     DEFB 0
TEXINI DEFW 23755
FINTEX DEFW 23755
PUNETI DEFW 0
FINNDX DEFW 0
DIRDBI DEFW 49152
LINEA  DEFW 1
LINP   DEFW 0
LINU   DEFW 0
DIRBLO DEFW 0
LONGBL DEFW 0
PUNDIR DEFW 16384
PUNORG DEFW 49152
DIRUP  DEFW 0
DIREDI DEFW 0
DIRENT DEFW 50000
PUNDES DEFW 49152
ERROR  DEFB 0
AOPE   DEFB 0
APASO  DEFB 0
AERRS  DEFW 0
ETINUM DEFW 0
BYTES  DEFW 0
ALINEA DEFW 0
ADIREC DEFW 0
ASIGDI DEFW 0
ANEMO  DEFW 0
ADIOP1 DEFW 0
ADIOP2 DEFW 0
AVAL1  DEFW 0
AVAL2  DEFW 0
ADIS   DEFB 0
ADES   DEFB 0
APREXY DEFB 0
DPREF  DEFB 0
DDIRIN DEFW 0
DRETOR DEFW 0
DREGR  DEFB 0
DBYBRK DEFS 3
DLONG  DEFB 0
DMEMO  DEFW 0
CABFUE DEFB 3
       DEFM "NOT-NAMED."
LONG0  DEFW 0
       DEFS 5
DIA    DEFB 0
MES    DEFB 0
ANYO   DEFB 0
LONG1  DEFW 0
LONG2  DEFW 0
RESERV DEFB 255,255,255,255
       DEFB 255,255,255,255,255
PUNTEX DEFW LONG0
LINCUR DEFW 0
ULTLIN DEFW 0
TEMP01 DEFW 0
TEMP02 DEFW 0
TEMP03 DEFW 0
OLDFL2 DEFB 0
DTIPIN DEFB 0
DNXTPC DEFW 0
BUFETI DEFS 32
LNETIN DEFW 0
BUFCAD DEFS 32
LNCAIN DEFW 0
EJEINI
INICI0 CALL COPPAG
       LD   SP,23552
       CALL PONREG
       CALL SLDIR
       CALL NOBUF
       CALL PONPRI
       LD   A,(OLDFL2)
       AND  %00001100
       LD   (FLAGS2),A
       XOR  A
       CALL PONTEX
       JP   MENU
COPPAG LD   HL,RUTPAG
       LD   DE,23296
       LD   BC,256
       LDIR
       RET
PONREG LD   HL,49152
       LD   DE,23755
       LD   BC,32768-23755
       LD   A,16
       EX   AF,AF'
       LD   A,19
       RET
PONPRI LD   HL,PRINT
       LD   (23739),HL
       CALL CLS
       LD   (IY-28),4
       LD   (IY-48),1
       SET  3,(IY+48)
       RES  1,(IY-17)
       LD   HL,DERR8
       LD   (23613),HL
       LD   HL,64000
       LD   (23651),HL
       LD   (23653),HL
       RES  4,(IY-17)
       JP   PONBOR
NOBUF  LD   (IY-27),32
       RET
INICIO CALL PONPRI
       CALL NOBUF
       CALL APUN
       CALL PRIMES
       DEFM "Welcome to @"
       CALL PRSPED
       CALL PRIMES
       DEFM ",#"
       DEFM "Assembler/Monitor"
       DEFM "/Disassembler#"
       DEFM "for Spectrum 128K"
       DEFM " / ZX Next.#@"
       CALL PRCPYR
       LD   A,13
       RST  16
       CALL INFECH
       JP   MENU
FIN    LD   A,(FLAGS2)
       LD   (OLDFL2),A
       CALL PRIMES
       DEFM "Return Address:@"
       LD   HL,INICI0
       CALL PRNUME
       LD   HL,32767
       LD   (23730),HL
       CALL PONREG
       LD   (PUNETI),HL
       LD   (FINNDX),HL
       EX   AF,AF'
       EX   DE,HL
       CALL SLDIR
       JP   4535
PRIMES POP  HL
PRIME1 LD   A,(HL)
       INC  HL
       CP   "#"
       JR   Z,PRIME3
       CP   "@"
       JR   Z,PRIME4
PRIME2 PUSH HL
       RST  16
       POP  HL
       JR   PRIME1
PRIME3 LD   A,13
       JR   PRIME2
PRIME4 JP   (HL)
CLS    LD   A,2
       CALL 5633
       LD   HL,16384
       LD   DE,16385
       LD   BC,6143
       LD   (HL),0
       LDIR
       INC  HL
       INC  DE
       LD   BC,767
       LD   A,4
       LD   (HL),A
       LD   (23693),A
       LD   (23624),A
       LDIR
       LD   (23682),BC
       RET
PRINT  RES  5,(IY-17)
       BIT  0,(IY-28)
       JR   Z,PRINT1
       CP   24
       JR   C,PRINT0
       LD   A,23
PRINT0 LD   (23683),A
       RES  0,(IY-28)
       SET  1,(IY-28)
       RET
PRINT1 BIT  1,(IY-28)
       JR   Z,PRINT2
       LD   (23682),A
       RES  1,(IY-28)
       RET
PRINT2 CP   8
       JR   NZ,PRINT3
       DEC  (IY+72)
       RET  P
       LD   (IY+72),31
       DEC  (IY+73)
       RET  P
       LD   (IY+73),0
       RET
PRINT3 CP   13
       JR   NZ,PRINT4
CMPL23 LD   (IY+72),0
       LD   A,(23683)
       CP   23
       JP   Z,SCROLL
       INC  (IY+73)
       RET
PRINT4 CP   22
       JR   NZ,PRINT5
       SET  0,(IY-28)
       RET
PRINT5 CP   23
       JR   NZ,PRINT6
       SET  1,(IY-28)
       RET
PRINT6 LD   L,A
       LD   H,0
       ADD  HL,HL
       ADD  HL,HL
       ADD  HL,HL
       LD   DE,15360
       ADD  HL,DE
       LD   A,(23683)
       LD   D,A
       RRCA
       RRCA
       RRCA
       AND  224
       OR   (IY+72)
       LD   E,A
       LD   A,D
       AND  24
       OR   64
       LD   D,A
       LD   B,8
PRINT7 LD   A,(HL)
       SLA  A
       OR   (HL)
       LD   (DE),A
       INC  D
       INC  HL
       DJNZ PRINT7
       LD   A,(23682)
       INC  A
       LD   (23682),A
       CP   32
       RET  NZ
       JR   CMPL23
SCROLL CALL SITEMY
       JR   NZ,SCROL1
       CALL ESPTEC
       CP   199
       JR   NZ,SCROL1
       SET  5,(IY-17)
SCROL1 JP   3582
SITEMY LD   A,254
       IN   A,(254)
       AND  1
       RET
EDIVA0 LD   A,13
       RST  16
EDIVAC CALL LIMBUF
EDITAR XOR  A
       LD   (CX),A
       LD   A,(23683)
       LD   (CY),A
EDITA1 LD   (IY+7),0
EDIT11 CALL PONCUR
       CALL PRIEST
EDITA2 LD   (IY-50),0
EDIT22 LD   A,(23560)
EDIT23 OR   A
       JR   Z,EDIT22
       CP   13
       JP   Z,ED13
       CP   8
       JR   Z,ED8
       CP   9
       JR   Z,ED9
       CP   10
       JP   Z,EDITA3
       CP   11
       JP   Z,EDITA3
       CP   6
       JR   Z,ED6
       CP   14
       JR   Z,ED14
       CP   7
       JP   Z,ED7
       CP   175
       JR   Z,ED175
       CP   12
       JP   Z,ED12
       CP   32
       JR   C,EDITA2
       CP   128
       JP   NC,EDITA3
       EX   AF,AF'
       LD   A,(CX)
       CP   31
       JR   Z,EDITA2
       EX   AF,AF'
       CALL BORCUR
       LD   HL,(CX)
       INC  L
       LD   (CX),HL
       DEC  L
       LD   (23682),HL
       LD   H,0
       LD   DE,BUFEDI
       ADD  HL,DE
       PUSH AF
       PUSH HL
       RST  16
       POP  HL
       POP  AF
       BIT  2,(IY-28)
       JP   NZ,EDITA4
       LD   (HL),A
       JR   EDITA1
ED8    LD   A,(CX)
       OR   A
       JR   Z,EDITA2
       CALL BORCUR
       DEC  A
       LD   (CX),A
       JP   EDITA1
ED9    LD   A,(CX)
       CP   31
       JP   Z,EDITA2
       CALL BORCUR
       INC  A
       LD   (CX),A
       JP   EDITA1
ED6    LD   A,(23658)
       XOR  8
       LD   (23658),A
       JP   EDITA1
ED14   LD   A,(23617)
       XOR  1
       LD   (23617),A
       CALL PRIEST
       JP   EDIT11
ED175  LD   A,(FLAGS1)
       XOR  4
       LD   (FLAGS1),A
       JP   EDITA1
ED13   CALL BORCUR
       LD   HL,(CX)
       LD   (23682),HL
       LD   A,13
       RST  16
       LD   HL,BUFEDI
       OR   A
       RET
ED12   LD   A,(CX)
       OR   A
       JP   Z,EDITA2
       CALL BORCUR
       LD   L,A
       DEC  A
       LD   (CX),A
       LD   H,0
       LD   DE,BUFEDI
       ADD  HL,DE
       LD   D,H
       LD   E,L
       DEC  DE
       PUSH HL
       LD   HL,BUFEDI+30
       OR   A
       SBC  HL,DE
       LD   B,H
       LD   C,L
       POP  HL
       LD   A,B
       OR   C
       JR   Z,ED120
       LDIR
ED120  LD   A,32
       LD   (DE),A
       JR   EDIT44
EDITA3 RES  0,(IY+7)
       BIT  3,(IY-28)
       JP   Z,EDITA2
       CALL BORCUR
       PUSH AF
       CALL PRIEST
       POP  AF
       SCF
       RET
EDITA4 EX   AF,AF'
       LD   A,(CX)
       CP   31
       JR   NZ,EDIT40
       EX   AF,AF'
       LD   (HL),A
       JP   EDITA1
EDIT40 EX   AF,AF'
       EX   DE,HL
       LD   HL,BUFEDI+31
       OR   A
       SBC  HL,DE
       LD   B,H
       LD   C,L
       LD   HL,BUFEDI+30
       LD   DE,BUFEDI+31
       LDDR
       LD   (DE),A
       LD   A,13
       LD   (BUFEDI+31),A
EDIT44 CALL PRILIN
EDIT43 JP   EDITA1
ED7    BIT  3,(IY-28)
       JP   Z,EDITA2
       CALL BORCUR
       LD   A,(CX)
       CP   7
       JR   NC,ED70
       LD   A,7
       JR   ED79
ED70   LD   A,12
ED79   LD   (CX),A
       JR   EDIT43
PRILIN LD   HL,(CX)
       LD   L,0
       LD   (23682),HL
       LD   HL,BUFEDI
       LD   B,31
PRILI1 LD   A,(HL)
       PUSH BC
       PUSH HL
       RST  16
       POP  HL
       POP  BC
       INC  HL
       DJNZ PRILI1
       RET
BORCUR PUSH AF
       LD   C,4
       CALL PRICUR
       POP  AF
       RET
PONCUR LD   C,132
PRICUR LD   HL,(CY)
       LD   H,0
       ADD  HL,HL
       ADD  HL,HL
       ADD  HL,HL
       ADD  HL,HL
       ADD  HL,HL
       LD   A,H
       OR   88
       LD   H,A
       LD   A,(CX)
       OR   L
       LD   L,A
       LD   (HL),C
       RET
PRIEST BIT  3,(IY-28)
       RET  Z
       LD   HL,5898
       LD   (23682),HL
       BIT  3,(IY+48)
       JR   Z,PRIES1
       CALL PRIMES
       DEFM "CAP @"
       JR   PRIES2
PRIES1 CALL PRIMES
       DEFM "low @"
PRIES2 BIT  2,(IY-28)
       JR   Z,PRIES3
       CALL PRIMES
       DEFM "INS @"
       JR   PRIES4
PRIES3 CALL PRIMES
       DEFM "OVR @"
PRIES4 BIT  0,(IY+7)
       JR   Z,PRIES5
       CALL PRIMES
       DEFM "EXT@"
       RET
PRIES5 CALL PRIMES
       DEFM "   @"
       RET
VAL0   RET  Z
       CP   ")"
       RET  Z
       CP   ","
       RET  Z
       CP   255
       RET
VAL    RES  4,(IY-28)
       LD   BC,0
       LD   A,(HL)
       CP   32
       CALL VAL0
       JR   Z,VALNO
       CP   13
       JR   Z,VALNO
VAL1   LD   A,(HL)
       CP   13
       CALL VAL0
       JR   Z,VALSI
       CP   32
       JR   Z,VALSI
       JR   C,VALNO
       CP   128
       JR   NC,VALNO
       CP   "+"
       JR   NZ,VAL2
       INC  HL
VAL2   CP   "-"
       JR   Z,VALRES
       CALL VALNUM
       JR   C,VALNO
       PUSH HL
       LD   H,B
       LD   L,C
       ADD  HL,DE
VAL3   LD   B,H
       LD   C,L
       POP  HL
       LD   A,(HL)
       CP   "+"
       JR   Z,VAL1
       CP   "-"
       JR   Z,VAL1
       CP   13
       CALL VAL0
       JR   Z,VALSI
       CP   32
       JR   NZ,VALNO
VALSI  LD   D,B
       LD   E,C
       OR   A
       RET
VALNO  LD   D,B
       LD   E,C
       SCF
       RET
VALRES INC  HL
       CALL VALNUM
       JR   C,VALNO
       PUSH HL
       LD   H,B
       LD   L,C
       OR   A
       SBC  HL,DE
       JR   VAL3
VALNUM PUSH BC
       LD   A,(HL)
       CP   "$"
       JR   NZ,VALN10
       LD   DE,(PUNDIR)
       JR   VALFI0
VALN10 CP   "!"
       JR   NZ,VALNU1
       LD   DE,(PUNDES)
       JR   VALFI0
VALNU1 CP   "#"
       JR   Z,VALHEX
       CP   "%"
       JR   Z,VALBIN
       CP   34
       JR   NZ,VALNU2
       INC  HL
       LD   E,(HL)
       INC  HL
       LD   A,(HL)
       CP   34
       JR   NZ,VALFIE
       LD   D,0
       JR   VALFI0
VALNU2 CP   48
       JR   C,VALFIE
       CP   58
       JR   C,VALDEC
       CALL ESETIQ
       JR   NC,VALETI
       JR   VALFIE
VALFI0 INC  HL
VALFIN POP  BC
       OR   A
       RET
VALFIE POP  BC
       SCF
       RET
VALBIN LD   DE,0
VALBI1 INC  HL
       LD   A,(HL)
       SUB  48
       JR   C,VALFIN
       CP   2
       JR   NC,VALFIN
       PUSH HL
       LD   H,D
       LD   L,E
       ADD  HL,HL
       LD   E,A
       LD   D,0
       ADD  HL,DE
       EX   DE,HL
       POP  HL
       JR   VALBI1
VALDEC LD   DE,0
       DEC  HL
VALDE1 INC  HL
       LD   A,(HL)
       SUB  48
       JR   C,VALFIN
       CP   10
       JR   NC,VALFIN
       PUSH HL
       LD   H,D
       LD   L,E
       ADD  HL,HL
       LD   D,H
       LD   E,L
       ADD  HL,HL
       ADD  HL,HL
       ADD  HL,DE
       LD   E,A
       LD   D,0
       ADD  HL,DE
       EX   DE,HL
       POP  HL
       JR   VALDE1
VALHEX LD   DE,0
VALHE1 INC  HL
       LD   A,(HL)
       SUB  48
       JR   C,VALFIN
       CP   23
       JR   NC,VALFIN
       CP   10
       JR   C,VALHE2
       SUB  17
       JR   C,VALFIN
       ADD  A,10
VALHE2 PUSH HL
       LD   H,D
       LD   L,E
       ADD  HL,HL
       ADD  HL,HL
       ADD  HL,HL
       ADD  HL,HL
       LD   E,A
       LD   D,0
       ADD  HL,DE
       EX   DE,HL
       POP  HL
       JR   VALHE1
VALETI CALL CALETI
       JP   NC,VALFIN
       SET  4,(IY-28)
       JP   VALFIE
INFECH CALL PRIMES
       DEFM "Enter Date"
       DEFM "(DD/MM/YYYY):@"
       CALL EDIVA0
       CALL VAL
       LD   A,E
       LD   (DIA),A
       INC  HL
       CALL VAL
       LD   A,E
       LD   (MES),A
       INC  HL
       CALL VAL
       LD   H,D
       LD   L,E
       LD   DE,1980
       OR   A
       SBC  HL,DE
       LD   A,L
       LD   (ANYO),A
       RET
MENU   LD   SP,23552
       RES  3,(IY-28)
       CALL PRIMES
       DEFM "#Command>#@"
       CALL EDIVAC
       LD   A,(BUFEDI+1)
       CP   ","
       JR   Z,MENU1
       CP   32
       JR   Z,MENU1
MENERR CALL MENER0
       JR   MENU
MENER0 CALL PRIMES
       DEFM "Invalid Command!"
       DEFM "#@"
       RET
MENU1  LD   A,13
       RST  16
       LD   DE,BUFEDI
       LD   A,(DE)
       LD   HL,BUFEDI+2
       LD   BC,28
       LDIR
       LD   C,A
       LD   HL,TAMENU
MENU2  LD   A,(HL)
       INC  HL
       CP   255
       JR   Z,MENERR
       CP   C
       JR   Z,MENU3
       INC  HL
       INC  HL
       JR   MENU2
MENU3  LD   E,(HL)
       INC  HL
       LD   H,(HL)
       LD   L,E
       CALL MEJPHL
       JR   MENU
MEJPHL JP   (HL)
TAMENU DEFM "A"
       DEFW AINICI
       DEFM "B"
       DEFW FIN
       DEFM "C"
       DEFW CALCUL
       DEFM "D"
       DEFW DINICI
       DEFM "E"
       DEFW EDICIO
       DEFM "F"
       DEFW INFECH
       DEFM "G"
       DEFW GOTEXT
       DEFM "H"
       DEFW HLPMNU
       DEFM "I"
       DEFW INFORM
       DEFM "L"
       DEFW LOAFUE
       DEFM "N"
       DEFW NUETEX
       DEFM "O"
       DEFW SALOBJ
       DEFM "S"
       DEFW SALFUE
       DEFM "T"
       DEFW VERTAB
       DEFM "Z"
       DEFW ZAP
       DEFB 255
HLPMNU XOR  A
       JP   HELP
PRSPED CALL PRIMES
       DEFM "SPED 1.3 EN@"
       RET
PRCPYR CALL PRIMES
       DEFM "#   ,   "
       DEFM "     ,       ~,#"
       DEFB 127
       DEFM " CESAR"
       DEFM " HERNANDEZ BANO#"
       DEFM "(04/1995,07/1997,"
       DEFM "09/2018)"
       DEFM "#English version"
       DEFM " by Tim Gilberts@"
       RET
PRNUME PUSH HL
       LD   A,32
       RST  16
       POP  HL
       JR   PRNUM
PRNUAE PUSH AF
       LD   A,32
       RST  16
       POP  AF
PRNUMA LD   L,A
       LD   H,0
PRNUM  LD   E,200
       CALL PRNU1
       RET
PRNU1  PUSH DE
       PUSH HL
       LD   BC,55536
       CALL 6442
       JP   6704
PRNUM0 LD   E,32
       CALL PRNU1
       RET
PRNU16 BIT  4,(IY-18)
       JR   NZ,PRHE16
       LD   E,48
       CALL PRNU1
       RET
PNUM16 BIT  4,(IY-18)
       JR   NZ,PRHE16
       JR   PRNUM
PNUM8  BIT  4,(IY-18)
       JR   Z,PRNUMA
PRHE8S PUSH AF
       LD   A,"#"
       RST  16
       POP  AF
       JR   PRHEX8
PRHE16 PUSH HL
       LD   A,"#"
       RST  16
       POP  HL
       PUSH HL
       LD   A,H
       CALL PRHEX8
       POP  HL
       LD   A,L
PRHEX8 PUSH AF
       AND  %11110000
       RRCA
       RRCA
       RRCA
       RRCA
       CALL PRHE81
       POP  AF
       AND  %1111
PRHE81 ADD  A,48
       CP   58
       JR   C,PRHE82
       ADD  A,7
PRHE82 RST  16
       RET
CALCUL LD   HL,BUFEDI
       CALL VAL
       EX   DE,HL
CALCU1 JP   NC,PRNUM
       BIT  4,(IY-28)
       JR   NZ,CALCU2
       CALL PRIMES
       DEFM "Syntax "
       DEFM "Error!#@"
       RET
CALCU2 LD   A,3
       JP   APRERR
VERTAB LD   HL,(PUNETI)
       LD   DE,49152
       OR   A
       SBC  HL,DE
       RET  Z
       EX   DE,HL
VERTA1 PUSH HL
VERTA2 CALL ELDAHL
       INC  HL
       CP   255
       JR   Z,VERTA3
       PUSH HL
       RST  16
       POP  HL
       JR   VERTA2
VERTA3 LD   A,32
       RST  16
       POP  HL
       PUSH HL
       LD   BC,7
       ADD  HL,BC
       CALL ELDAHL
       LD   E,A
       INC  HL
       CALL ELDAHL
       LD   D,A
       INC  HL
       CALL ELDAHL
       LD   C,A
       INC  HL
       CALL ELDAHL
       LD   B,A
       PUSH BC
       EX   DE,HL
       CALL PRNUM
       LD   A,13
       RST  16
       POP  BC
       POP  HL
       OR   A
       SBC  HL,BC
       RET  Z
       BIT  5,(IY-17)
       RET  NZ
       LD   H,B
       LD   L,C
       JR   VERTA1
NUMTEX LD   A,(IY-27)
       AND  %00001100
       RRCA
       RRCA
       RET
GOTEXT LD   HL,BUFEDI
       CALL VAL
       CALL PONLI1
       LD   A,D
       OR   A
       JR   NZ,PRERRB
       CALL NUMTEX
       CP   E
       JR   C,PRERRB
       LD   A,E
       CALL PONTEX
       JP   PRBLAC
PRERRB CALL PRIMES
       DEFM "Text block"
       DEFM " invalid!#@"
       RET
NUETEX CALL NUMTEX
       CP   2
       JR   NZ,NUETE2
       CALL PRIMES
       DEFM "No more"
       DEFM " blocks!#@"
       RET
NUETE2 INC  A
       RLCA
       RLCA
       LD   B,A
       LD   A,(IY-27)
       AND  %11110011
       OR   B
       LD   (IY-27),A
       RET
SALOBJ LD   HL,BUFEDI
       LD   DE,CABOBJ
       PUSH DE
       LD   A,3
       LD   (DE),A
       INC  DE
       LD   BC,10
       LDIR
       CALL CALLON
       EX   DE,HL
       LD   (HL),E
       INC  HL
       LD   (HL),D
       INC  HL
       LD   (HL),C
       INC  HL
       LD   (HL),B
       POP  IX
       PUSH DE
       XOR  A
       LD   DE,17
       SCF
       CALL SAVRA1
       CALL PAUSGR
       LD   A,20
       LD   (NUMPAG+1),A
       POP  DE
       LD   IX,(PUNORG)
       LD   A,255
       SCF
       CALL SAVE
       EI
       CALL ACTPAG
       JP   PONBOR
SAVRA1 CALL 1222
       EI
PONBOR PUSH AF
       XOR  A
       OUT  (254),A
       POP  AF
       RET
PAUSGR LD   B,20
PAUSG1 HALT
       DJNZ PAUSG1
       RET
CALLON LD   HL,(PUNDES)
       LD   BC,(PUNORG)
       OR   A
       SBC  HL,BC
       RET
ZAP    CALL PRIMES
       DEFM "Sure?@"
       CALL EDIVA0
       LD   A,(BUFEDI)
       CP   "Y"
       RET  NZ
       CALL PONLI1
ZAP2   LD   A,(FLAGS2)
       AND  %11110000
       LD   (FLAGS2),A
       JP   ZRTEX
PRESAV CALL SITEMY
       JR   NZ,PRESAF
       PUSH IX
       PUSH DE
       CALL PRIMES
       DEFM "Pause mode. DELET"
       DEFM "E to ignore#"
       DEFM "block#@"
       POP  DE
       POP  IX
       CALL ESPTEC
       CP   12
       JR   NZ,PRESAF
       CALL PRIMES
       DEFM "Block ignored.#@"
       RET
PRESAF LD   A,255
       JP   SAVE
SALFUE CALL ZRTEX0
       LD   HL,BUFEDI
       LD   DE,CABFUE
       PUSH DE
       INC  DE
       LD   BC,10
       LDIR
       POP  HL
       PUSH HL
       XOR  A
       LD   B,17
SALFU1 XOR  (HL)
       INC  HL
       DJNZ SALFU1
       LD   (HL),A
       POP  IX
       LD   DE,34
       XOR  A
       CALL SAVRA1
       CALL PAUSGR
       LD   IX,23755
       LD   DE,(LONG0)
       CALL PRESAV
       EI
       LD   HL,LONG1
       LD   A,22
       PUSH AF
       PUSH HL
SALFU2 CALL PAUSGR
       POP  HL
       LD   DE,RESERV
       LD   A,D
       CP   H
       JR   NZ,SALFU3
       LD   A,E
       CP   L
       JR   Z,SALFU0
SALFU3 LD   A,(HL)
       INC  HL
       OR   (HL)
       DEC  HL
       JR   Z,SALFU0
       POP  AF
       LD   (NUMPAG+1),A
       INC  A
       PUSH AF
       LD   E,(HL)
       INC  HL
       LD   D,(HL)
       INC  HL
       PUSH HL
       LD   IX,49152
       CALL PRESAV
       EI
       JR   SALFU2
SALFU0 CALL ZRTEX0
       CALL PONBOR
       JP   MENU
PRBLAC CALL PRIMES
       DEFM "#Text block in "
       DEFM "use:@"
       CALL BACT
       CALL PRNUAE
       LD   A,13
       RST  16
       RET
INFORM CALL PRSPED
       CALL PRIMES
       DEFM "#Todays Date: @"
       LD   HL,CABFUE+18
       CALL PRFECH
       LD   A,13
       RST  16
       LD   HL,CABFUE+1
       CALL PRNOMB
       CALL PRBLAC
       CALL PRIMES
       DEFM "Total blocks"
       DEFM " in use:@"
       CALL BTXTUS
       INC  A
       CALL PRNUAE
       CALL PRLBL
       LD   A,"0"
       RST  16
       LD   DE,(LONG0)
       LD   HL,41781
       CALL PRRES
       CALL BTXTUS
       OR   A
       JR   Z,INFOR2
       LD   B,A
       LD   C,"1"
       LD   HL,LONG1
INFOR1 PUSH HL
       PUSH BC
       CALL PRLBL
       POP  BC
       LD   A,C
       INC  C
       PUSH BC
       RST  16
       POP  BC
       POP  HL
       LD   E,(HL)
       INC  HL
       LD   D,(HL)
       INC  HL
       PUSH HL
       PUSH BC
       LD   HL,16384
       CALL PRRES
       POP  BC
       POP  HL
       DJNZ INFOR1
INFOR2 LD   A,13
       RST  16
       LD   A,13
       RST  16
INFOR3 CALL PRTAB
       CALL PRDEB
       LD   A,13
       RST  16
       CALL PRCOD
       CALL PRINEN
       LD   A,13
       RST  16
       RET
BOPRED LD   HL,0
       LD   (23682),HL
       LD   B,64
BOPRE1 PUSH BC
       LD   A,32
       RST  16
       POP  BC
       DJNZ BOPRE1
       LD   (IY+73),0
       RET
PRINEN CALL PRIMES
       DEFM "Start:@"
       LD   HL,(PUNORG)
       CALL PRNUME
       CALL PRIMES
       DEFM ", Execute:@"
       LD   HL,(DIRENT)
       JR   PRCOD3
PRCOD  CALL PRIMES
       DEFM "Object Code:@"
       CALL CALLON
PRCOD1 CALL PRNUBY
PRCOD2 LD   A,13
       RST  16
       RET
PRCOD3 CALL PRNUME
       JR   PRCOD2
PRTAB  CALL PRIMES
       DEFM "Symbol"
       DEFM " table:@"
       LD   BC,49152
       LD   HL,(FINNDX)
       OR   A
       SBC  HL,BC
       JR   PRCOD1
PRDEB  CALL BTXTUS
       CP   2
       RET  Z
       CALL PRIMES
       DEFM "Debug"
       DEFM " Table:@"
       LD   HL,(DIRDBI)
       LD   DE,49152
       OR   A
       SBC  HL,DE
       JR   PRCOD1
PRFECH PUSH HL
       LD   L,(HL)
       LD   H,0
       CALL PRNUM
       LD   A,"/"
       RST  16
       POP  HL
       INC  HL
       PUSH HL
       LD   L,(HL)
       LD   H,0
       CALL PRNUM
       LD   A,"/"
       RST  16
       POP  HL
       INC  HL
       LD   L,(HL)
       LD   H,0
       LD   DE,1980
       ADD  HL,DE
       CALL PRNUM
       JR   PRCOD2
PRNUBY CALL PRNUME
       CALL PRIMES
       DEFM " bytes@"
       RET
BACT   LD   A,(FLAGS2)
       AND  3
       RET
PRRES  PUSH DE
       PUSH HL
       LD   A,":"
       RST  16
       POP  HL
       CALL PRNUME
       LD   A,"-"
       RST  16
       POP  HL
       JP   PRNUM
PRLBL  CALL PRIMES
       DEFM "#Free in"
       DEFM " block @"
       RET
BTXTUS LD   A,(FLAGS2)
       RRCA
       RRCA
       AND  3
       RET
PRNOMB PUSH HL
       CALL PRIMES
       DEFM "Source code"
       DEFM " name: @"
       POP  HL
PRNOM0 LD   B,10
PRNOM1 LD   A,(HL)
       INC  HL
       PUSH BC
       PUSH HL
       RST  16
       POP  HL
       POP  BC
       DJNZ PRNOM1
       RET
ZRTXTS LD   HL,0
       LD   (LONG0),HL
       LD   (LONG1),HL
       LD   (LONG2),HL
ZRDBI  LD   HL,49152
       LD   (DIRDBI),HL
       RET
LOAFUE LD   A,(FLAGS2)
       AND  %11111100
       LD   (FLAGS2),A
       CALL ZRTXTS
       LD   IX,CABOBJ
       LD   DE,34
       CALL LOAD
       LD   HL,CABOBJ+1
       CALL PRNOMB
       CALL PRIMES
       DEFM "#Last "
       DEFM "build date:@"
       LD   HL,CABOBJ+18
       CALL PRFECH
       LD   IX,23755
       LD   DE,(CABOBJ+11)
       CALL LOAD2
       LD   HL,CABOBJ+21
       LD   BC,512
LOAFU1 LD   E,(HL)
       INC  HL
       LD   D,(HL)
       INC  HL
       LD   A,D
       OR   E
       JR   Z,LOAFIN
       INC  C
       LD   A,(FLAGS2)
       AND  %11111100
       OR   C
       LD   (FLAGS2),A
       CALL ACTPAG
       PUSH HL
       PUSH BC
       LD   IX,49152
       CALL LOAD2
       POP  BC
       POP  HL
       DJNZ LOAFU1
LOAFIN LD   A,(FLAGS2)
       AND  %11110000
       RLC  C
       RLC  C
       OR   C
       LD   (FLAGS2),A
       LD   HL,CABOBJ+1
       LD   DE,CABFUE+1
       LD   BC,12
       LDIR
       LD   HL,CABOBJ+21
       LD   DE,LONG1
       LD   BC,13
       LDIR
       CALL PONBOR
       XOR  A
       JP   PONTEX
LOAD2  LD   A,255
       SCF
       CALL LOAD0
       EI
       JR   NC,LOAERR
       RET
LOAD   XOR  A
       SCF
       INC  D
       EX   AF,AF'
       DEC  D
       DI
       CALL 1378
       EI
       RET  C
LOAERR CALL PRIMES
       DEFM "#Load ERROR! "
       DEFM "(Rewind Tape?)@"
       CALL PONBOR
       CALL ZAP2
       JP   MENU
ACTPAG CALL PAGTEX
       LD   (NUMPAG+1),A
       RET
PAGTEX LD   A,(FLAGS2)
       AND  3
       JR   NZ,PAGTE1
       LD   A,16
       RET
PAGTE1 ADD  A,21
       RET
SLDIR  LD   (NUMPAG+1),A
       PUSH AF
       CALL ILDAHL
       LD   LX,A
       POP  AF
       EX   AF,AF'
       LD   (NUMPAG+1),A
       EX   AF,AF'
       PUSH AF
       LD   A,LX
       EX   DE,HL
       CALL ILDHLA
       POP  AF
       EX   DE,HL
       INC  HL
       INC  DE
       DEC  BC
       PUSH AF
       LD   A,B
       OR   C
       JR   Z,SLDIR0
       POP  AF
       JR   SLDIR
SLDIR0 POP  AF
       JP   ACTPAG
PRNUL0 LD   HL,262
       LD   (23682),HL
PRNULI LD   HL,(LINCUR)
       JP   PRNUM0
PRMEMO LD   HL,283
       LD   (23682),HL
       LD   HL,0
       LD   DE,(FINTEX)
       OR   A
       SBC  HL,DE
       JP   PRNUM0
PRNUTE LD   HL,31
       LD   (23682),HL
       CALL BACT
       JP   PRNUMA
CADILI EX   DE,HL
       LD   BC,1
       LD   HL,(TEXINI)
       LD   IX,(FINTEX)
       CALL CADIL5
       RET  C
       JP   CDIL0
CADIL4 LD   IX,(FINTEX)
CADIL5 LD   A,HX
       CP   H
       JR   NZ,CADIL6
       LD   A,LX
       CP   L
       JR   NZ,CADIL6
       SCF
       RET
CADIL6 OR   A
       RET
PRLIN  RES  4,(IY-27)
       LD   BC,(DIREDI)
       LD   A,B
       OR   C
       JR   Z,PRLI00
       LD   H,B
       LD   L,C
       JR   PRLIN0
PRLI00 CALL CADILI
       JR   NC,PRLIN0
       SET  4,(IY-27)
       RET
PRLIN0 LD   C,0
PRLIN1 CALL ILDAHL
       INC  HL
       CP   13
       JR   Z,PRLIFI
       BIT  7,A
       JR   NZ,PRLIN2
PRLI11 INC  C
       PUSH BC
       PUSH HL
       CALL PRCAR
       POP  HL
       POP  BC
       LD   A,C
       CP   32
       JR   NZ,PRLIN1
       PUSH HL
       LD   HL,(TEMP01)
       DEC  HL
       LD   (TEMP01),HL
       DEC  C
       POP  HL
       JR   PRLIN1
PRLIN2 PUSH AF
       LD   A,C
       CP   7
       JR   NC,PRLIN3
       LD   B,7
       JR   PRLIN4
PRLIN3 CP   12
       JR   NC,PRLI33
       LD   B,12
PRLIN4 LD   A,C
       CP   B
       JR   Z,PRLIN5
       INC  C
       PUSH BC
       PUSH HL
       LD   A,32
       CALL PRCAR
       POP  HL
       POP  BC
       JR   PRLIN4
PRLI33 LD   B,C
       INC  B
       JR   PRLIN4
PRLIN5 POP  AF
       RES  7,A
       JR   PRLI11
PRLIFI LD   A,C
       CP   32
       JR   Z,PRLIF0
       INC  C
       PUSH BC
       PUSH HL
       LD   A,32
       CALL PRCAR
       POP  HL
       POP  BC
       JR   PRLIFI
PRLIF0 BIT  5,(IY-28)
       RET  Z
       LD   HL,(TEMP01)
       DEC  HL
       LD   (TEMP01),HL
       LD   A,13
       JP   PRCAR
PRCAR  JP   16
PONLIB PUSH AF
       LD   HL,BUFEDI
       LD   (TEMP01),HL
       LD   HL,PONCAR
       LD   (PRCAR+1),HL
       LD   HL,(LINCUR)
       SET  5,(IY-28)
       CALL PRLIN
       RES  5,(IY-28)
       LD   HL,16
       LD   (PRCAR+1),HL
       CALL PONCUR
       CALL PRIEST
       POP  AF
       JP   EDIT23
PONCAR LD   HL,(TEMP01)
       LD   (HL),A
       INC  HL
       LD   (TEMP01),HL
       RET
PRPAG  LD   HL,512
       LD   (23682),HL
       LD   HL,(DIRUP)
       LD   A,H
       OR   L
       JR   NZ,PRP_00
       LD   HL,(LINEA)
       CALL CADILI
       LD   (DIRUP),HL
       JR   C,PRPAG0
PRP_00 CALL PRLIN0
       LD   BC,(LINEA)
PRPAG2 LD   A,(23683)
       CP   23
       JR   Z,PRPAG9
       LD   DE,(FINTEX)
       CALL CODEHL
       JR   Z,PRPAG0
PRPAG3 PUSH BC
       CALL PRLIN0
       POP  BC
       INC  BC
       JR   PRPAG2
PRPAG9 LD   DE,(FINTEX)
       CALL CODEHL
       JR   Z,PRPAG0
PRPAG8 LD   BC,65535
PRPAG0 LD   (ULTLIN),BC
PRPA00 LD   A,(23683)
       CP   23
       RET  Z
       LD   B,32
PRPA01 PUSH BC
       LD   A,32
       RST  16
       POP  BC
       DJNZ PRPA01
       JR   PRPA00
PREDIC LD   A,32
       CALL PONCOL
       LD   HL,23264
       LD   DE,23265
       LD   BC,31
       LD   (HL),32
       LDIR
       LD   HL,0
       LD   (23682),HL
       CALL PRIMES
       DEFM "Current text:"
       DEFM " @"
       LD   HL,CABFUE+1
       CALL PRNOM0
PREDI2 CALL PRIMES
       DEFM "#Line:        "
       DEFM "  Free Memory:@"
       JP   PRIEST
BORLIN CALL CADILI
       RET  C
BORLI0 PUSH HL
       CALL DIFIL0
       POP  DE
       RET  C
       SBC  HL,DE
       LD   B,H
       LD   C,L
       EX   DE,HL
       JP   BORMEM
INSLIN CALL CADILI
INSLI0 LD   BC,0
       PUSH HL
       LD   HL,BUFEDI
INSL00 LD   A,(HL)
       INC  HL
       INC  BC
       CP   13
       JR   NZ,INSL00
       POP  HL
       PUSH HL
       PUSH BC
       CALL CREMEM
       LD   HL,BUFEDI
       POP  BC
       POP  DE
       CALL PAGTEX
       EX   AF,AF'
       LD   A,17
       JP   SLDIR
DIFILI CALL CADILI
       RET  C
DIFIL0 LD   BC,(FINTEX)
       LD   A,B
       CP   H
       JR   NZ,DIFIL1
       LD   A,C
       CP   L
       JR   NZ,DIFIL1
       SCF
       RET
DIFIL1 CALL ILDAHL
       INC  HL
       CP   13
       JR   NZ,DIFIL1
       OR   A
       RET
LINNU0 CALL LINNU9
       JP   INSLI0
LINNUE CALL LINNU9
       JP   INSLIN
LINNU9 LD   A,13
       LD   (BUFEDI),A
       RET
CONVLI LD   HL,BUFEDI+31
       LD   B,31
CONVL0 DEC  HL
       LD   A,(HL)
       CP   32
       JR   NZ,CONVL1
       DJNZ CONVL0
       JR   CONVL2
CONVL1 INC  HL
CONVL2 LD   (HL),13
       LD   HL,BUFEDI
       LD   DE,BUFEDI
       XOR  A
       PUSH AF
       LD   A,(HL)
       CP   ";"
       JR   NZ,CONVL3
       POP  AF
       RET
CONVL3 LD   A,(HL)
       CP   13
       JR   Z,CONVFI
       CP   32
       JR   Z,CONVL4
       LDI
       JR   CONVL3
CONVL4 POP  AF
       CP   2
       JR   NZ,CONVL5
       PUSH AF
       LDI
       JR   CONVL3
CONVL5 INC  A
       PUSH AF
CONVL6 LD   A,(HL)
       INC  HL
       CP   32
       JR   Z,CONVL6
       DEC  HL
       SET  7,A
       LD   (HL),A
       JR   CONVL3
CONVFI POP  AF
       LDI
       RET
EDICIO CALL CLS
       SET  3,(IY-28)
       LD   HL,(LINEA)
       LD   (LINCUR),HL
       LD   HL,512
       LD   (CX),HL
       CALL ZRDBI
       CALL ZRLIN
       LD   (DIRUP),HL
       LD   (DIREDI),HL
       LD   (LNETIN),HL
       LD   (LNCAIN),HL
EDICI0 CALL PREDIC
       CALL PRMEMO
       CALL PRNUTE
EDICI1 CALL PRPAG
EDICI2 CALL PRNUL0
EDICI3 CALL PONCUR
       LD   (IY-50),0
EDICI4 LD   A,(23560)
EDIC44 OR   A
       JR   Z,EDICI4
       CP   8
       JR   Z,EDIC8
       CP   9
       JP   Z,EDIC9
       CP   10
       JP   Z,EDIC10
       CP   11
       JP   Z,EDIC11
       CP   14
       JR   Z,EDIC14
       CP   175
       JR   Z,EDICIN
       CP   184
       JP   Z,EDICI1
       CP   199
       JP   Z,EDCSQ
       CP   128
       JP   NC,EDICSP
       CALL PONLIB
EDIC45 JP   C,EDICES
       CALL ACTLIN
       LD   HL,(LINCUR)
       INC  HL
       LD   (LINCUR),HL
       LD   HL,(DIREDI)
       CALL DIFIL0
       LD   (DIREDI),HL
       CALL LINNU0
       XOR  A
       LD   (CX),A
       LD   A,(CY)
       CP   22
       JR   NZ,EDIC46
       CALL POSILI
       JR   EDIC47
EDIC46 CALL BORCUR
       INC  A
       LD   (CY),A
EDIC47 CALL PRPAG
       CALL PRNUL0
       CALL LIMBUF
       CALL EDITA1
       JR   EDIC45
EDIC8  LD   A,(CX)
       OR   A
       JR   Z,EDICI3
       CALL BORCUR
       DEC  A
EDIC88 LD   (CX),A
       JP   EDICI2
EDICIN LD   A,(FLAGS1)
       XOR  4
       LD   (FLAGS1),A
       RES  0,(IY+7)
       JR   EDI_14
EDIC14 LD   A,(23617)
       XOR  1
       LD   (23617),A
EDI_14 CALL PRIEST
       JP   EDICI3
EDIC9  LD   A,(CX)
       CP   31
       JP   Z,EDICI3
       CALL BORCUR
       INC  A
       JR   EDIC88
EDIC11 LD   A,(CY)
       CP   2
       JR   Z,EDI111
       CALL BORCUR
       DEC  A
       LD   (CY),A
       LD   HL,(LINCUR)
       DEC  HL
       LD   (LINCUR),HL
       JP   EDICI2
EDI111 LD   HL,(LINEA)
       LD   A,H
       OR   A
       JR   NZ,EDI112
       LD   A,L
       CP   1
       JP   Z,EDICI3
EDI112 DEC  HL
       LD   (LINEA),HL
       LD   (LINCUR),HL
       CALL BORCUR
       LD   BC,(TEXINI)
       LD   HL,(DIRUP)
       DEC  HL
EDI113 LD   A,H
       CP   B
       JR   NZ,EDI114
       LD   A,L
       CP   C
       JR   Z,EDI115
EDI114 DEC  HL
       CALL ILDAHL
       CP   13
       JR   NZ,EDI113
       INC  HL
EDI115 LD   (DIRUP),HL
       JP   EDICI1
EDIC10 LD   HL,(ULTLIN)
       LD   A,H
       CP   255
       JR   NZ,EDI100
       LD   HL,(LINCUR)
       INC  HL
       LD   (LINCUR),HL
       LD   A,(CY)
       CP   22
       JR   Z,EDI101
EDI103 CALL BORCUR
       INC  A
       LD   (CY),A
       JP   EDICI2
EDI101 CALL POSILI
       JP   EDICI1
EDI100 LD   DE,(LINCUR)
       CALL CODEHL
       JP   Z,EDICI2
EDI102 LD   HL,(LINCUR)
       INC  HL
       LD   (LINCUR),HL
       LD   A,(CY)
       JR   EDI103
EDICSP RES  0,(IY+7)
       CALL BORCUR
       PUSH AF
       CALL PRIEST
       POP  BC
       JR   EDICE0
EDICES LD   HL,0
       LD   (DIREDI),HL
       CP   184
       JP   Z,EDICI1
       PUSH AF
       CALL ACTLIN
       POP  BC
       LD   HL,0
       LD   (DIREDI),HL
EDICE0 LD   HL,TAEDIC
EDICE1 LD   A,(HL)
       INC  HL
       CP   255
       JP   Z,EDICI3
       CP   B
       JR   Z,EDICE2
       INC  HL
       INC  HL
       JR   EDICE1
EDICE2 LD   C,(HL)
       INC  HL
       LD   B,(HL)
       LD   HL,(LINCUR)
       PUSH BC
       RET
TAEDIC DEFB 199
       DEFW EDCSQ
       DEFB 11
       DEFW EDIC11
       DEFB 10
       DEFW EDIC10
       DEFB 172
       DEFW EDCSI
       DEFB 205
       DEFW EDCSD
       DEFB 173
       DEFW EDCEP
       DEFB 194
       DEFW EDCEU
       DEFB 196
       DEFW EDCEB
       DEFB 224
       DEFW EDCEC
       DEFB 225
       DEFW EDCEV
       DEFB 185
       DEFW EDCEX
       DEFB 227
       DEFW EDCEA
       DEFB 178
       DEFW EDCEQ
       DEFB 192
       DEFW EDCEL
       DEFB 228
       DEFW EDCED
       DEFB 180
       DEFW EDCEE
       DEFB 176
       DEFW EDCEJ
       DEFB 188
       DEFW EDCEF
       DEFB 189
       DEFW EDCEG
       DEFB 166
       DEFW EDCEN
       DEFB 177
       DEFW EDCEK
       DEFB 187
       DEFW HLPEDI
       DEFB 255
ACTLIN CALL CONVLI
       LD   HL,(DIREDI)
       LD   A,H
       OR   L
       JR   NZ,ACTLI0
       LD   HL,(LINCUR)
       CALL CADILI
       LD   (DIREDI),HL
ACTLI0 PUSH HL
       CALL BORLI0
       POP  HL
       JP   INSLI0
HLPEDI LD   A,1
       CALL HELP
       JP   EDICIO
EDCSQ  LD   HL,(FINTEX)
       LD   BC,(TEXINI)
       OR   A
       SBC  HL,BC
       LD   B,H
       LD   C,L
       LD   HL,(PUNTEX)
       LD   (HL),C
       INC  HL
       LD   (HL),B
       CALL CLS
       JP   MENU
EDCSI  CALL LINNUE
EDCSI0 JP   EDICI1
EDCSD  PUSH HL
       CALL ZRLIN
       POP  HL
       CALL BORLIN
       CALL CMPFIN
       JR   EDCSI0
EDCEP  LD   (LINP),HL
       LD   HL,0
       LD   (LINU),HL
       RES  6,(IY-27)
       JP   EDICI3
EDCEU  LD   DE,(LINP)
       PUSH HL
       OR   A
       SBC  HL,DE
       POP  HL
       JR   NC,EDCEU1
       EX   DE,HL
       LD   (LINP),DE
EDCEU1 LD   (LINU),HL
       JP   EDICI3
EDCEE  LD   HL,1
       LD   (LNETIN),HL
       JR   EDCED1
EDCEJ  LD   HL,(LNETIN)
       LD   A,H
       OR   L
       JP   Z,EDICI3
       INC  HL
       LD   (LNETIN),HL
       JR   EDCED0
EDCED  LD   HL,(LINCUR)
       LD   (LNETIN),HL
EDCED1 CALL BOPRED
       CALL PONNEG
       CALL PRIMES
       DEFM "Search for"
       DEFM " Symbol?:@"
       CALL EDVA0C
       PUSH HL
       LD   A,32
       LD   BC,31
       CPIR
       DEC  HL
       LD   (HL),255
       POP  HL
       LD   DE,BUFETI
       LD   BC,31
       LDIR
EDCED0 LD   HL,(LNETIN)
       PUSH HL
       CALL CADILI
       POP  BC
       JP   C,EDICI0
       LD   DE,BUFETI
EDCED2 PUSH DE
       PUSH HL
       PUSH BC
       CALL CMPETI
       POP  BC
       POP  HL
       POP  DE
       JR   Z,EDCEDF
       PUSH BC
       CALL DIFIL0
       POP  BC
       JR   C,EDCED3
       INC  BC
       JR   EDCED2
EDCED3 CALL BOPRED
       CALL PRIMES
       DEFM "Symbol not found@"
       CALL ZRLNET
       JP   PRBLN0
EDCEDF LD   (DIRUP),HL
       LD   (LINCUR),BC
       LD   (LINEA),BC
       LD   (LNETIN),BC
       CALL ZRCY
       JP   EDICI0
EDCEG  LD   HL,1
       LD   (LNCAIN),HL
       JR   EDCEF1
EDCEN  LD   HL,(LNCAIN)
       LD   A,H
       OR   L
       JP   Z,EDICI3
       INC  HL
       LD   (LNCAIN),HL
       JR   EDCEF0
EDCEF  LD   HL,(LINCUR)
       LD   (LNCAIN),HL
EDCEF1 CALL BOPRED
       CALL PONNEG
       CALL PRIMES
       DEFM "Text to find?:@"
       CALL EDVA0C
       PUSH HL
       LD   HL,BUFEDI+31
EDCE_1 DEC  HL
       LD   A,(HL)
       CP   32
       JR   Z,EDCE_1
       INC  HL
       LD   (HL),255
       POP  HL
       LD   DE,BUFCAD
       LD   BC,31
       LDIR
EDCEF0 LD   HL,(LNCAIN)
       PUSH HL
       CALL CADILI
       POP  BC
       JP   C,EDICI0
       PUSH HL
       LD   DE,BUFCAD
       LD   IX,(FINTEX)
EDCEF2 PUSH DE
       PUSH HL
       PUSH BC
       PUSH IX
       CALL CMPCAD
       POP  IX
       POP  BC
       POP  HL
       POP  DE
       JR   Z,EDCEFF
       CALL ILDAHL
       INC  HL
       CP   13
       JR   NZ,EDCEF2
       CALL CADIL5
       JR   C,EDCEF3
       INC  SP
       INC  SP
       PUSH HL
       INC  BC
       JR   EDCEF2
EDCEF3 POP  HL
       CALL BOPRED
       CALL PRIMES
       DEFM "Text not found@"
       CALL ZRLNCA
       JP   PRBLN0
EDCEFF POP  HL
       LD   (DIRUP),HL
       LD   (LINCUR),BC
       LD   (LINEA),BC
       LD   (LNCAIN),BC
       CALL ZRCY
       JP   EDICI0
CMPETI CALL ILDAHL
       CP   13
       JR   Z,CMPETF
       CP   128
       JR   NC,CMPETF
       LD   C,A
       LD   A,(DE)
       CP   255
       RET  Z
       CP   C
       RET  NZ
       INC  HL
       INC  DE
       JR   CMPETI
CMPETF LD   A,(DE)
       CP   255
       RET
CMPCAD CALL ILDAHL
       RES  7,A
       CP   13
       JR   Z,CMPETF
       LD   C,A
       LD   A,(DE)
       CP   255
       RET  Z
       CP   C
       RET  NZ
       INC  HL
       INC  DE
       JR   CMPCAD
EDCEL  CALL BOPRED
       CALL PONNEG
       CALL PRIMES
       DEFM "Goto line?:@"
       CALL EDVA0C
       CALL VAL
       LD   H,D
       LD   L,E
       LD   A,H
       OR   L
       JP   Z,EDICI0
       CALL CADILI
       JR   NC,EDCEL0
       CALL ZRDRUP
       DEC  BC
       JR   EDCEL1
EDCEL0 LD   (DIRUP),HL
EDCEL1 LD   (LINCUR),BC
       LD   (LINEA),BC
       CALL ZRCY
       JP   EDICI0
EDCEC  CALL COMBLO
       JP   Z,PRBLNO
       SET  6,(IY-27)
       CALL CALLGB
       CALL CPYBUF
       JP   EDICI3
EDCEB  CALL COMBLO
       JP   Z,PRBLNO
       CALL CALLGB
EDCEB0 CALL ZRCUR
       LD   HL,(DIRBLO)
       LD   BC,(LONGBL)
       CALL BORMEM
       CALL ZRLIN
       CALL CMPFIN
       CALL ZRDRUP
       CALL ZRCY
       JP   EDICI0
EDCEX  CALL COMBLO
       JP   Z,PRBLNO
       SET  6,(IY-27)
       CALL CALLGB
       CALL CPYBUF
       JR   EDCEB0
EDCEQ  LD   HL,(LINEA)
       LD   A,H
       OR   A
       JR   NZ,EDCEQ0
       LD   A,L
       CP   1
       JP   Z,EDICI3
EDCEQ0 PUSH HL
       POP  IX
       LD   HL,(DIRUP)
       LD   DE,(TEXINI)
       DEC  HL
       LD   B,21
EDCEQ1 CALL CODEHL
       JR   NZ,EDCEQ2
       DEC  IX
EDCEQQ LD   (LINCUR),IX
       LD   (LINEA),IX
       LD   (DIRUP),HL
       CALL ZRCY
       JP   EDICI0
EDCEQ2 DEC  HL
       CALL ILDAHL
       CP   13
       JR   NZ,EDCEQ1
       DEC  IX
       DJNZ EDCEQ1
       INC  HL
       JR   EDCEQQ
CODEHL LD   A,D
       CP   H
       RET  NZ
       LD   A,E
       CP   L
       RET
EDCEA  LD   IX,(LINEA)
       LD   HL,(DIRUP)
       LD   DE,(FINTEX)
       LD   B,21
EDCEA1 CALL CODEHL
       JR   NZ,EDCEA2
EDCEAB DEC  IX
       LD   (LINCUR),IX
       LD   (LINEA),IX
       CALL ZRDRUP
EDCEAA CALL ZRCY
       JP   EDICI0
EDCEA2 INC  IX
EDCEA3 CALL DIFIL1
       DJNZ EDCEA1
       CALL CODEHL
       JR   Z,EDCEAB
       LD   (LINCUR),IX
       LD   (LINEA),IX
       LD   (DIRUP),HL
       JR   EDCEAA
EDCEV  CALL CADILI
       BIT  6,(IY-27)
       JR   Z,PRBFNO
       LD   BC,(LONGBL)
       PUSH HL
       CALL CREMEM
       LD   HL,49152
       POP  DE
       LD   BC,(LONGBL)
       CALL PAGTEX
       EX   AF,AF'
       LD   A,19
       CALL SLDIR
       JP   EDICI0
PRBLNO CALL BOPRED
       CALL PRIMES
       DEFM "No block"
       DEFM " defined@"
PRBLN0 CALL ESPTEC
       JP   EDICI0
PRBFNO CALL BOPRED
       CALL PRIMES
       DEFM "No block in"
       DEFM " buffer@"
       JR   PRBLN0
EDCEK  CALL BOPRED
       CALL PONNEG
       CALL PRIMES
       DEFM "Calculate "
       DEFM " expression?:@"
       CALL EDVA0C
       CALL BOPRED
       CALL CALCUL
       CALL ESPTEC
       JP   EDICI0
ESPTEC LD   (IY-50),0
ESPTE1 LD   A,(23560)
       OR   A
       JR   Z,ESPTE1
       RET
COMBLO LD   HL,(LINP)
       LD   A,H
       OR   L
       RET  Z
       LD   HL,(LINU)
       LD   A,H
       OR   L
       RET
POSILI LD   HL,(LINEA)
       INC  HL
       LD   (LINEA),HL
       LD   HL,(DIRUP)
       PUSH AF
       CALL DIFIL0
       POP  AF
       LD   (DIRUP),HL
       RET
LIMBUF LD   HL,BUFEDI
       LD   DE,BUFEDI+1
       LD   BC,31
       LD   (HL),32
       LDIR
       LD   (HL),13
       RET
BORMEM PUSH HL
       ADD  HL,BC
       EX   DE,HL
       LD   HL,(FINTEX)
       PUSH HL
       OR   A
       SBC  HL,BC
       LD   (FINTEX),HL
       POP  HL
       OR   A
       SBC  HL,DE
       LD   B,H
       LD   C,L
       EX   DE,HL
       PUSH HL
       PUSH BC
       CALL PRMEMO
       POP  BC
       POP  HL
       POP  DE
       LD   A,B
       OR   C
       RET  Z
       JP   ILDIR
CREMEM PUSH HL
       LD   HL,(FINTEX)
       PUSH HL
       POP  IX
       ADD  HL,BC
       LD   (FINTEX),HL
       EX   DE,HL
       DEC  DE
       POP  BC
       PUSH IX
       POP  HL
       OR   A
       SBC  HL,BC
       LD   B,H
       LD   C,L
       PUSH IX
       PUSH BC
       PUSH DE
       CALL PRMEMO
       POP  DE
       POP  BC
       POP  HL
       DEC  HL
       LD   A,B
       OR   C
       RET  Z
       JP   ILDDR
ZRLIN  LD   HL,0
       LD   (LINP),HL
       LD   (LINU),HL
       RET
CMPFIN LD   HL,(LINCUR)
       CALL CADILI
       RET  NC
       LD   HL,(FINTEX)
       LD   A,13
       CALL ILDHLA
       INC  HL
       LD   (FINTEX),HL
       JP   PRMEMO
ZRCUR  LD   HL,(LINP)
       LD   (LINCUR),HL
       LD   (LINEA),HL
       RET
CPYBUF LD   HL,(DIRBLO)
       LD   BC,(LONGBL)
       LD   DE,49152
       LD   (PUNETI),DE
       LD   (FINNDX),DE
       LD   A,19
       EX   AF,AF'
       CALL PAGTEX
       JP   SLDIR
ZRDRUP LD   HL,0
       LD   (DIRUP),HL
       RET
PONTEX LD   B,A
       LD   A,(IY-27)
       AND  %11111100
       OR   B
       LD   (IY-27),A
       LD   A,B
       CALL ACTPAG
       LD   A,B
PONPNT OR   A
       JR   NZ,PONPN1
       LD   HL,LONG0
       LD   DE,23755
       JR   PONPN0
PONPN1 CP   1
       JR   NZ,PONPN2
       LD   HL,LONG1
       JR   PONPN3
PONPN2 LD   HL,LONG2
PONPN3 LD   DE,49152
PONPN0 LD   (PUNTEX),HL
       LD   (TEXINI),DE
       LD   (ADIREC),DE
       LD   A,(HL)
       INC  HL
       LD   H,(HL)
       LD   L,A
       ADD  HL,DE
       LD   (FINTEX),HL
       RET
ZRTEX  CALL ZRTXTS
ZRTEX0 XOR  A
       JP   PONTEX
PONNEG LD   A,4
PONCOL LD   HL,22528
       LD   DE,22529
       LD   BC,63
       LD   (HL),A
       LDIR
       RET
ZRCY   LD   A,2
       LD   (CY),A
       RET
CALLGB LD   HL,(LINU)
       CALL DIFILI
       PUSH HL
       LD   HL,(LINP)
       CALL CADILI
       LD   (DIRBLO),HL
       EX   DE,HL
       POP  HL
       OR   A
       SBC  HL,DE
       LD   (LONGBL),HL
       RET
ZRLNET LD   HL,0
       LD   (LNETIN),HL
       RET
ZRLNCA LD   HL,0
       LD   (LNCAIN),HL
       RET
EDVA0C RES  3,(IY-28)
       LD   A,(CY)
       PUSH AF
       CALL EDIVA0
       SET  3,(IY-28)
       POP  AF
       LD   (CY),A
       RET
PONLI1 LD   HL,1
       LD   (LINEA),HL
       RET
INIETI RES  6,(IY-27)
       XOR  A
       LD   HL,49152
       CALL ELDHLA
       INC  HL
       LD   A,255
       CALL ELDHLA
       LD   HL,49152+9
       LD   A,11
       CALL ELDHLA
       INC  HL
       LD   A,192
       CALL ELDHLA
       INC  HL
       LD   A,254
       CALL ELDHLA
       INC  HL
       LD   A,255
       CALL ELDHLA
       LD   HL,49152+9+11
       LD   A,11
       CALL ELDHLA
       INC  HL
       LD   A,192
       CALL ELDHLA
       INC  HL
       LD   (PUNETI),HL
       RET
CPLABE LD   A,(DE)
       CALL ESETIQ
       JR   C,CPLABF
       LD   C,A
       CALL ELDAHL
       CP   255
       JR   NZ,CPLAB1
       LD   A,1
       OR   A
       RET
CPLAB1 LD   B,A
       LD   A,C
       CP   B
       RET  NZ
       INC  HL
       INC  DE
       JR   CPLABE
CPLABF LD   C,A
       CALL ELDAHL
       CP   255
       JR   NZ,CPLAF1
       XOR  A
       RET
CPLAF1 SCF
       RET
CRENDX LD   DE,(PUNETI)
       LD   HL,49152
CREND2 EX   DE,HL
       LD   A,E
       CALL ELDHLA
       INC  HL
       LD   A,D
       CALL ELDHLA
       INC  HL
       EX   DE,HL
       PUSH HL
       LD   BC,9
       ADD  HL,BC
       CALL ELDAHL
       LD   C,A
       INC  HL
       CALL ELDAHL
       LD   B,A
       POP  HL
       OR   A
       SBC  HL,BC
       JR   Z,CRENDF
       LD   H,B
       LD   L,C
       JR   CREND2
CRENDF LD   (FINNDX),DE
       RET
PONETI EX   DE,HL
       LD   HL,49152
       PUSH HL
PONET2 PUSH HL
       LD   BC,9
       ADD  HL,BC
       CALL ELDAHL
       LD   C,A
       INC  HL
       CALL ELDAHL
       LD   B,A
       POP  HL
       PUSH HL
       PUSH BC
       PUSH DE
       CALL CPLABE
       POP  DE
       POP  BC
       POP  HL
       JR   Z,PONETF
       JR   C,PONET8
       INC  SP
       INC  SP
       PUSH HL
       LD   H,B
       LD   L,C
       JR   PONET2
PONETF POP  HL
       SCF
       RET
PONET8 PUSH HL
       CALL INSETI
       POP  DE
       LD   A,E
       CALL ELDHLA
       INC  HL
       LD   A,D
       CALL ELDHLA
       LD   BC,10
       OR   A
       SBC  HL,BC
       EX   DE,HL
       POP  HL
       LD   BC,9
       ADD  HL,BC
       LD   A,E
       CALL ELDHLA
       INC  HL
       LD   A,D
       CALL ELDHLA
       OR   A
       RET
INSETI LD   HL,(PUNETI)
       PUSH HL
INSET1 LD   A,(DE)
       CP   13
       JR   Z,INSETF
       CP   128
       JR   NC,INSETF
       CALL ELDHLA
       INC  HL
       INC  DE
       JR   INSET1
INSETF LD   A,255
       CALL ELDHLA
       POP  HL
       LD   BC,7
       ADD  HL,BC
       LD   A,LX
       CALL ELDHLA
       INC  HL
       LD   A,HX
       CALL ELDHLA
       INC  HL
       INC  HL
       INC  HL
       LD   (PUNETI),HL
       DEC  HL
       DEC  HL
       RET
CALETI PUSH HL
       POP  IX
       LD   HL,(FINNDX)
       PUSH HL
       LD   DE,49152
       OR   A
       SBC  HL,DE
       POP  HL
       JR   NZ,CALET1
       SCF
       PUSH AF
       JP   CALETF
CALET1 LD   DE,(PUNETI)
       OR   A
       SBC  HL,DE
CALET2 EX   DE,HL
       SRL  D
       RR   E
       DEC  DE
       LD   BC,0
CALET3 LD   H,D
       LD   L,E
       PUSH HL
       OR   A
       SBC  HL,BC
       POP  HL
       JR   C,CALETR
       ADD  HL,BC
       SRL  H
       RR   L
       PUSH HL
       PUSH BC
       PUSH DE
       CALL CALERE
       POP  DE
       POP  BC
       JR   Z,CALESI
       POP  HL
       JR   C,CALET4
       LD   B,H
       LD   C,L
       INC  BC
       JR   CALET3
CALET4 LD   D,H
       LD   E,L
       DEC  DE
       JR   CALET3
CALETR PUSH AF
       JR   CALETF
CALESI POP  DE
       OR   A
       PUSH AF
       LD   BC,7
       ADD  HL,BC
       CALL ELDAHL
       LD   E,A
       INC  HL
       CALL ELDAHL
       LD   D,A
CALETF LD   A,(IX+0)
       CALL ESETIQ
       JR   C,CALEF2
       INC  IX
       JR   CALETF
CALEF2 PUSH IX
       POP  HL
       POP  AF
       RET
CALERE ADD  HL,HL
       LD   DE,(PUNETI)
       ADD  HL,DE
       CALL ELDAHL
       LD   C,A
       INC  HL
       CALL ELDAHL
       LD   H,A
       LD   L,C
       PUSH HL
       PUSH IX
       POP  DE
       CALL CPLABE
       POP  HL
       RET
ESETIQ CP   48
       RET  C
       CP   128
       JR   NC,ESETIF
       CP   65
       RET  NC
       CP   58
ESETIF CCF
       RET
APUN   LD   HL,49152
       LD   (PUNETI),HL
       LD   (FINNDX),HL
       LD   (DIRDBI),HL
APUN2  LD   HL,49152
       LD   (PUNDES),HL
       LD   (PUNDIR),HL
       LD   (PUNORG),HL
       LD   (DIRENT),HL
       RET
ACOPLI LD   HL,(ADIREC)
       LD   DE,ABUF
ACOPL1 CALL ILDAHL
       INC  HL
       CP   13
       JR   Z,ACOPFI
       CP   128
       JR   C,ACOPL2
       LD   B,A
       LD   A,255
       LD   (DE),A
       INC  DE
       LD   A,B
       SUB  128
ACOPL2 LD   (DE),A
       INC  DE
       JR   ACOPL1
ACOPFI LD   (DE),A
       INC  DE
       LD   A,255
       LD   (DE),A
       RET
APRLIN LD   HL,(ALINEA)
       CALL PRNUM
       LD   A,13
       RST  16
       LD   HL,(ADIREC)
       JP   PRLIN0
APROBJ LD   HL,(PUNDES)
       PUSH HL
       CALL PRNUM
       LD   A,":"
       RST  16
       LD   DE,(BYTES)
       LD   HL,4
       OR   A
       SBC  HL,DE
       POP  HL
       JR   NC,APROB1
       LD   DE,4
APROB1 LD   A,D
       OR   E
       JR   Z,APROBF
       PUSH DE
       PUSH HL
       LD   A,32
       RST  16
       POP  HL
       CALL OLDAHL
       PUSH HL
       CALL PRNUMA
       POP  HL
       POP  DE
       INC  HL
       DEC  DE
       JR   APROB1
APROBF LD   A,13
       RST  16
       RET
ASILIS BIT  7,(IY-27)
       RET  Z
       BIT  7,(IY-28)
       RET
ACOMA  LD   A,(HL)
       CP   34
       JR   NZ,ACOMA1
       LD   A,B
       CPL
       LD   B,A
       JR   ACOM11
ACOMA1 CP   ","
       JR   NZ,ACOM10
       LD   A,B
       OR   A
       RET  Z
       JR   ACOM11
ACOM10 CP   13
       JR   Z,ACOMA2
ACOM11 SCF
       RET
ACOMA2 OR   A
       RET
ASIOP1 LD   A,(HL)
       CP   255
       RET  Z
       CP   13
       JR   Z,ACOMA2
       SCF
       RET
ACOMP  LD   A,(DE)
       CP   (HL)
       RET  NZ
       INC  HL
       INC  DE
       CP   255
       RET  Z
       JR   ACOMP
APOMAS LD   HL,OP11
       CALL CADIL5
       JR   NC,APOMA1
       LD   (IY-20),C
       RET
APOMA1 LD   (IY-19),C
       RET
PONINS BIT  1,(IY-18)
       JR   Z,PONBYT
       EX   AF,AF'
       LD   A,(BYTES)
       INC  A
       LD   (BYTES),A
       LD   A,(ADES)
       PUSH AF
       LD   A,(IX+2)
       CP   203
       JR   NZ,PONIN1
       POP  AF
       JR   PONIN2
PONIN1 POP  AF
       EX   AF,AF'
PONIN2 CALL PONBYT
       EX   AF,AF'
PONBYT BIT  7,(IY-28)
       JR   Z,PONBY1
       CALL OLDHLA
PONBY1 INC  HL
       RET
AMIOP  OR   A
       JR   NZ,AMIOP1
       LD   A,(IX+0)
       OR   A
       JR   NZ,AMIOP0
       LD   A,(IX+1)
       OR   A
       JR   NZ,AMIOP0
       LD   A,(IX+2)
       OR   A
       JR   NZ,AMIOP0
AMIO00 LD   A,1
       OR   A
       RET
AMIOP0 XOR  A
       RET
AMIOP1 PUSH DE
       PUSH HL
       LD   L,(IX+0)
       LD   H,(IX+1)
       LD   E,(IX+2)
       LD   D,A
AMIOP2 RR   E
       RR   H
       RR   L
       PUSH AF
       DEC  D
       JR   Z,AMIOP3
       POP  AF
       JR   AMIOP2
AMIOP3 POP  AF
       POP  HL
       POP  DE
       JR   C,AMIO00
       JR   AMIOP0
AFINBU LD   A,(DE)
       INC  DE
       CP   255
       JR   NZ,AFINBU
       RET
ACPSTR LD   C,0
ACPST1 LD   A,(DE)
       OR   A
       JR   Z,AMIO00
       PUSH HL
       CALL ACOMP
       POP  HL
       RET  Z
       INC  C
       CALL AFINBU
       JR   ACPST1
ASOLO  CP   11
       JR   Z,ASOL2
       JR   C,ASOL1
       CP   13
       JR   Z,ASOL2
       JR   C,ASOL1
       CP   24
       JR   Z,ASOL2
       CP   22
       RET  NC
       CP   14
       RET  Z
       CP   18
       RET  Z
ASOL2  SCF
       RET
ASOL1  OR   A
       RET
ADATIP CP   9
       JR   NC,ADATI1
       CP   3
       JR   Z,ADATI1
       CP   4
       JR   Z,ADATI1
       JR   ADATI2
ADATI1 SCF
       RET
ADATI2 OR   A
       RET
ADAMAS CP   22
       RET  NC
       CP   12
       CCF
       RET
APOPRE PUSH AF
       LD   A,(APREXY)
       OR   A
       JR   Z,APOPR0
       CALL PONBYT
       LD   A,(BYTES)
       INC  A
       LD   (BYTES),A
APOPR0 LD   A,(IX+2)
       OR   A
       JR   Z,APOPR1
       CALL PONBYT
       LD   A,(BYTES)
       INC  A
       LD   (BYTES),A
       POP  AF
       RET
APOPR1 POP  AF
       RET
AMA18  LD   A,(IX+2)
       CP   203
       JR   Z,AMA180
       CP   237
       RET  Z
       LD   A,B
       CP   192
       RET  NC
       CP   128
       CCF
       RET
AMA180 SCF
       RET
AMEZC  PUSH BC
       CALL ADAMAS
       JR   NC,AMEZC0
       RLC  C
       JR   AMEZC1
AMEZC0 CP   22
       JR   Z,AMEZC9
       CP   23
       JR   Z,AMEZC9
AMEZ00 CALL AMA18
       JR   C,AMEZC2
AMEZC1 RLC  C
       RLC  C
       RLC  C
AMEZC2 LD   A,C
       ADD  A,B
       POP  BC
       RET
AMEZC9 BIT  1,(IY-25)
       JR   Z,AMEZ00
       LD   C,3
       JR   AMEZ00
ASI8B  CP   1
       RET  Z
       CP   5
       RET  Z
       CP   8
       RET
ADADIS EX   AF,AF'
       LD   A,B
       CP   8
       JR   NZ,ADADI1
       LD   HL,(ADIS)
ADADI1 EX   AF,AF'
       RET
AINNUM PUSH BC
       CALL ADADIS
       PUSH HL
       LD   HL,(PUNDES)
       CALL APOPRE
       LD   B,(IX+3)
       CALL AMEZC
       CALL PONINS
       POP  DE
       LD   A,E
       CALL PONBYT
       POP  AF
       CALL ASI8B
       JR   Z,AINNU2
       LD   A,D
       CALL PONBYT
       LD   A,(BYTES)
       ADD  A,3
       JR   AINNU3
AINNU2 LD   A,(BYTES)
       ADD  A,2
AINNU3 LD   (BYTES),A
       JP   ANXTLI
ADOSMA LD   A,(IY-20)
       RLCA
       RLCA
       RLCA
       ADD  A,(IY-19)
       ADD  A,(IX+3)
       LD   HL,(PUNDES)
       CALL APOPRE
       CALL PONINS
       LD   A,(BYTES)
       INC  A
       LD   (BYTES),A
       JP   ANXTLI
AOPSOL OR   A
       JR   Z,AOPSO1
       LD   HL,(AVAL1)
       LD   A,(IX+0)
       LD   C,(IY-20)
       JR   AOPSO2
AOPSO1 LD   HL,(AVAL2)
       LD   A,(IX+1)
       LD   C,(IY-19)
AOPSO2 CALL ADATIP
       JR   C,AOPSO3
       LD   B,A
       LD   C,0
       JP   AINNUM
AOPSO3 LD   HL,(PUNDES)
       CALL APOPRE
       LD   B,(IX+3)
       CALL AMEZC
       CALL PONINS
       LD   A,(BYTES)
       INC  A
       LD   (BYTES),A
       JP   ANXTLI
ABUSOL LD   DE,ATASO2
       CALL ACPSTR
       JP   NZ,ABUSO1
       LD   L,C
       LD   H,0
       ADD  HL,HL
       LD   DE,ATASOL
       ADD  HL,DE
       LD   A,(HL)
       INC  HL
       LD   E,(HL)
       LD   D,0
       PUSH IX
       POP  HL
       ADD  HL,DE
       OR   (HL)
       LD   (HL),A
       JP   ABUSIG
ABUSO1 PUSH HL
       LD   HL,ASTRDM
       LD   DE,(ANEMO)
       CALL ACOMP
       POP  HL
       JP   Z,ABUSIG
       JP   ABUNUM
ANUOP1 LD   HL,(ADIOP1)
ANUCAL PUSH IX
       CALL VAL
       POP  IX
       CCF
       RET  C
       BIT  4,(IY-28)
       JR   Z,ANUCA2
       BIT  7,(IY-28)
       JR   NZ,ANUCA2
       LD   (IX+0),207
       XOR  A
       RET
ANUCA2 LD   A,1
       OR   A
       RET
APONUM CALL APONU0
       JP   ABUSIG
APONU0 LD   HL,OP11
       CALL CADIL5
       JR   NC,APONU1
       LD   HL,AVAL1
       JR   APONU2
APONU1 LD   HL,AVAL2
APONU2 LD   (HL),E
       INC  HL
       LD   (HL),D
       RET
ANU8BI LD   A,D
       OR   A
       RET  Z
       CP   255
       RET  NZ
ANU800 LD   A,E
       CP   128
       JR   C,ANU8B1
ANU8B0 XOR  A
       RET
ANU8B1 LD   A,1
       OR   A
       RET
ANUDIS LD   A,D
       OR   A
       JR   NZ,ANUDI1
       LD   A,E
       OR   A
       JP   M,ANU8B1
       JR   ANU8B0
ANUDI1 CP   255
       JR   NZ,ANU8B1
       JR   ANU800
AERNUM BIT  4,(IY-28)
       JR   Z,AERNU1
       LD   A,3
       JP   ANONE1
AERNU1 LD   A,2
       JP   ANONE1
ABUNUM LD   A,(HL)
       CP   "("
       JR   NZ,ABUNU2
       INC  HL
       CALL ANUCAL
       JR   NZ,ABUNU0
       LD   (IX+0),48
       JP   ABUSIG
ABUNU0 JR   NC,AERNUM
       LD   A,(HL)
       CP   ")"
       JR   NZ,AERNUM
       SET  5,(IX+0)
       CALL ANU8BI
       JR   NZ,ABUNU1
       SET  4,(IX+0)
ABUNU1 JP   APONUM
ABUNU2 CALL ANUCAL
       JP   Z,ABUSIG
       JR   NC,AERNUM
       LD   A,(HL)
       CP   ")"
       JR   Z,AERNUM
       CALL APONU0
       SET  6,(IX+0)
       CALL ANU8BI
       JR   NZ,ABUNU3
       SET  0,(IX+0)
       LD   A,E
       CP   8
       JR   NC,ABUN21
       SET  2,(IX+0)
       LD   C,E
       CALL APOMAS
       LD   A,C
       OR   A
       JR   NZ,ABUNU3
ABUN21 RRCA
       JR   C,ABUNU3
       RRCA
       JR   C,ABUNU3
       RRCA
       JR   C,ABUNU3
       CP   8
       JR   NC,ABUNU3
       SET  3,(IX+0)
       LD   C,A
       CALL APOMAS
ABUNU3 LD   HL,(PUNDIR)
       INC  HL
       INC  HL
       OR   A
       PUSH DE
       EX   DE,HL
       SBC  HL,DE
       LD   A,L
       LD   (ADIS),A
       EX   DE,HL
       CALL ANUDIS
       POP  DE
       JP   NZ,ABUSIG
       SET  7,(IX+0)
       JP   ABUSIG
ASIQUI PUSH HL
       LD   A,C
       OR   A
       JR   Z,ASIQU2
       INC  HL
       LD   (HL),255
ASIQU2 POP  DE
       RET
APOHL  LD   A,C
       OR   A
       JR   NZ,APOHL1
       LD   BC,2
       LD   HL,ASTRHL
       JR   APOHL2
APOHL1 DEC  A
       ADD  A,A
       LD   L,A
       LD   H,0
       LD   BC,ASTRH
       ADD  HL,BC
       LD   BC,1
APOHL2 LDIR
       RET
AFIN0  CALL PRCOD
       CALL PRINEN
AFIN   XOR  A
       JP   PONTEX
ALEVAL LD   A,(HL)
       CP   ","
       JR   NZ,ALEVA1
       INC  HL
ALEVA1 LD   A,(HL)
       CP   255
       JR   NZ,ALEVA2
       INC  HL
       LD   A,(HL)
       CP   255
       JR   Z,ALEVFI
ALEVA2 CALL ANUCAL
       JR   Z,ALEVSI
       RET  NC
ALEVSI LD   A,1
       OR   A
       SCF
       RET
ALEVFI XOR  A
       RET
APONDP BIT  5,(IY-27)
       RET  Z
       BIT  7,(IY-28)
       RET  NZ
       LD   A,D
       OR   E
       RET  Z
       CALL BTXTUS
       CP   2
       RET  Z
       LD   HL,(ADIREC)
       CALL BACT
       OR   A
       JR   Z,APOND1
       LD   DE,49152
       OR   A
       SBC  HL,DE
APOND1 EX   DE,HL
       LD   A,23
       LD   (NUMPAG+1),A
       LD   HL,(DIRDBI)
       LD   A,C
       CALL PONBYM
       LD   A,B
       CALL PONBYM
       LD   A,E
       CALL PONBYM
       LD   A,D
       CALL PONBYM
       LD   (DIRDBI),HL
       JP   ACTPAG
PONBYM CALL ILDHLA
       INC  HL
       RET
PRPASE CALL PRIMES
       DEFM "#Pass @"
       LD   A,(IY-28)
       RLCA
       AND  1
       INC  A
       JP   PRNUMA
PRPAER CALL PRPASE
       CALL PRIMES
       DEFM ", Errors @"
       LD   HL,(AERRS)
       CALL PRNUM
       LD   A,13
       RST  16
       RET
AINICI CALL PRSPED
       CALL PRCPYR
       RES  7,(IY-28)
       RES  5,(IY-27)
       CALL APUN
       CALL INIETI
       LD   HL,0
       LD   (AERRS),HL
AINIC0 RES  7,(IY-27)
       XOR  A
       CALL PONTEX
       CALL PRPASE
       LD   A,13
       RST  16
AINIC2 LD   HL,1
       LD   (ALINEA),HL
       CALL PRIMES
       DEFM "#Text"
       DEFM " block @"
       CALL BACT
       CALL PRNUMA
       LD   A,13
       RST  16
ABUC   XOR  A
       LD   (ERROR),A
       LD   (AOPE),A
       LD   (APREXY),A
       LD   H,A
       LD   L,A
       LD   (OP11),HL
       LD   (OP13),HL
       LD   (OP22),HL
       LD   (BYTES),HL
       LD   A,(IY-18)
       AND  %11111100
       LD   (IY-18),A
       RES  6,(IY-28)

       BIT  1,(IY-17)
       JR   NZ,ABUC21

       LD   HL,(ADIREC)
       CALL CADIL4
       JR   NC,ABUC2
       CALL NUMTEX
       LD   B,A
       CALL BACT
       CP   B
       JR   Z,ABUC1
       INC  A
       CALL PONTEX
       JR   AINIC2
ABUC1  CALL PRPAER
       BIT  7,(IY-28)
       JP   NZ,AFIN0
       LD   HL,(AERRS)
       LD   A,H
       OR   L
       JP   NZ,AFIN
       SET  7,(IY-28)
       CALL CRENDX
       CALL PRTAB
       CALL PRDEB
       CALL APUN2
       JP   AINIC0
ABUC2  CALL ACOPLI
       LD   (ASIGDI),HL
ABUC21 LD   HL,ABUF
       LD   A,(HL)
       CP   ";"
       JP   Z,ANXTLI
       CP   13
       JP   Z,ANXTLI
       CP   255
       JR   Z,ANOETI
       SET  6,(IY-28)
       LD   HL,(PUNDIR)
       LD   (ETINUM),HL
       LD   HL,ABUF
ABUC3  INC  HL
       LD   A,(HL)
       CP   13
       JP   Z,ANXTLI
       CP   255
       JR   Z,ANOETI
       JR   ABUC3
ANOETI INC  HL
       LD   (ANEMO),HL
ANOET0 CALL ASIOP1
       INC  HL
       JR   C,ANOET0
       JR   NZ,ANOOPE
       LD   A,1
       LD   (AOPE),A
       LD   (ADIOP1),HL
       LD   B,0
ANOET1 CALL ACOMA
       INC  HL
       JR   C,ANOET1
       JR   NZ,ANOOPE
       LD   A,2
       LD   (AOPE),A
       LD   (ADIOP2),HL
       DEC  HL
       LD   (HL),255
       INC  HL
ANOET2 CALL ASIOP1
       INC  HL
       JR   C,ANOET2
ANOOPE DEC  HL
       LD   (HL),255
       LD   A,(AOPE)
       OR   A
       JP   Z,ABUNEM
       LD   IX,OP11
       LD   (APASO),A
       LD   HL,(ADIOP1)
ABUOPE LD   DE,ASTRSP
       PUSH HL
       CALL ACOMP
       POP  HL
       JR   NZ,ABUOP1
       SET  4,(IX+1)
       SET  3,(IX+1)
ABUOP0 LD   C,3
       CALL APOMAS
       JP   ABUSIG
ABUOP1 LD   DE,ASTRX
       CALL ACPSTR
       JR   NZ,ABUO15
       LD   A,221
       LD   (APREXY),A
       PUSH HL
       CALL ASIQUI
       CALL APOHL
       POP  HL
       JR   ABUO19
ABUO15 LD   DE,ASTRY
       CALL ACPSTR
       JR   NZ,ABUO16
       LD   A,253
       LD   (APREXY),A
       PUSH HL
       CALL ASIQUI
       CALL APOHL
       POP  HL
       JR   ABUO19
ABUO16 LD   A,(HL)
       CP   "("
       JR   NZ,ABUO19
       PUSH HL
       INC  HL
       LD   A,(HL)
       CP   "I"
       JR   NZ,ABUO18
       INC  HL
       LD   A,(HL)
       CP   "X"
       JR   Z,ABUO17
       CP   "Y"
       JR   NZ,ABUO18
       LD   C,253
       JR   ABO170
ABUO17 LD   C,221
ABO170 INC  HL
       LD   A,(HL)
       CP   "+"
       JR   Z,ABO171
       CP   "-"
       JR   Z,ABO171
       CP   ")"
       JR   NZ,ABUO18
       LD   A,C
       LD   (APREXY),A
       POP  DE
       JR   ABO173
ABO171 LD   A,C
       LD   (APREXY),A
       CALL ANUCAL
       POP  BC
       JR   Z,ABO172
       JP   NC,AERNUM
       CALL ANUDIS
       JP   NZ,AERDES
ABO172 LD   A,(HL)
       CP   ")"
       JP   NZ,AERNUM
       INC  HL
       LD   A,(HL)
       CP   255
       JP   NZ,AERNUM
       LD   A,E
       LD   (ADES),A
       SET  1,(IY-18)
       LD   D,B
       LD   E,C
ABO173 PUSH DE
       LD   HL,ASTRH2
       LD   BC,5
       LDIR
ABUO18 POP  HL
ABUO19 LD   DE,ASTRAF
       PUSH HL
       CALL ACOMP
       POP  HL
       JR   NZ,ABUOP2
       SET  0,(IX+2)
       SET  5,(IX+1)
       JP   ABUOP0
ABUOP2 LD   DE,ASTR8B
       CALL ACPSTR
       JR   NZ,ABUOP3
       PUSH HL
       CALL APOMAS
       POP  HL
       SET  1,(IX+1)
       LD   A,C
       CP   1
       JR   NZ,ABUO21
       SET  5,(IX+2)
       SET  6,(IX+2)
ABUO21 CP   6
       JR   NC,ABUOP7
       JR   ABUSIG
ABUOP3 LD   DE,ASTR16
       CALL ACPSTR
       JR   NZ,ABUOP4
       PUSH HL
       CALL APOMAS
       POP  HL
       SET  3,(IX+1)
       SET  5,(IX+1)
       LD   A,C
       OR   A
       JR   NZ,ABUOP7
       JR   ABUSIG
ABUOP4 LD   DE,ASTRCO
       CALL ACPSTR
       JR   NZ,ABUOP5
       CALL APOMAS
       SET  6,(IX+2)
       LD   A,C
       CP   4
       JR   NC,ABUSIG
       SET  5,(IX+2)
       JR   ABUSIG
ABUOP5 LD   DE,ASTRBC
       CALL ACPSTR
       JR   NZ,ABUOP6
       CALL APOMAS
       SET  1,(IX+2)
       JR   ABUSIG
ABUOP6 LD   DE,ASTRIR
       CALL ACPSTR
       JR   NZ,ABUOP7
       CALL APOMAS
       SET  0,(IX+1)
       JR   ABUSIG
ABUOP7 JP   ABUSOL
ABUSIG LD   IX,OP21
       LD   HL,(ADIOP2)
       LD   A,(APASO)
       DEC  A
       LD   (APASO),A
       JP   NZ,ABUOPE
ABUNEM LD   HL,(ANEMO)
       LD   DE,ATABLA
ABUNE1 LD   A,(DE)
       CP   255
       JP   Z,ABUPSE
       PUSH HL
       CALL ACOMP
       JR   Z,ABUNE2
       CALL AFINBU
       LD   A,(DE)
       INC  DE
       LD   L,A
       LD   H,0
       ADD  HL,HL
       ADD  HL,HL
       ADD  HL,DE
       EX   DE,HL
       POP  HL
       JR   ABUNE1
ABUNE2 POP  HL
       LD   A,(DE)
       LD   B,A
       INC  DE
ABUNE3 LD   IX,OP11
       LD   A,(DE)
       CALL AMIOP
       JR   NZ,ABUNE4
       INC  DE
       INC  DE
       INC  DE
       INC  DE
ABUNE0 DJNZ ABUNE3
       JP   AEROPE
ABUNE4 LD   IX,OP21
       INC  DE
       LD   A,(DE)
       CALL AMIOP
       JR   NZ,ABUNE5
       INC  DE
       INC  DE
       INC  DE
       JR   ABUNE0
ABUNE5 DEC  DE
       PUSH DE
       POP  IX
       LD   A,(DE)
       OR   A
       JP   Z,ANOMAS
       CALL ASOLO
       JR   NC,ANSOL1
       INC  DE
       LD   A,(DE)
       OR   A
       JR   Z,ANOMAS
       CALL ASOLO
       JR   C,ANOMAS
       XOR  A
       JP   AOPSOL
ANSOL1 INC  DE
       LD   A,(DE)
       CALL ASOLO
       JR   NC,ANSOL2
       LD   A,1
       JP   AOPSOL
ANSOL2 DEC  DE
       LD   A,(DE)
       CALL ADATIP
       JR   NC,ANMA1
       INC  DE
       LD   A,(DE)
       OR   A
       JR   Z,ANOOP2
       CALL ADATIP
       JP   C,ADOSMA
       LD   HL,(AVAL2)
       LD   A,(DE)
       LD   B,A
       DEC  DE
       LD   A,(DE)
       LD   C,(IY-20)
       JP   AINNUM
ANOOP2 DEC  DE
       LD   A,(DE)
       LD   C,(IY-20)
       LD   HL,(PUNDES)
       CALL APOPRE
       LD   B,(IX+3)
       CALL AMEZC
       CALL PONINS
       LD   A,(BYTES)
       INC  A
       LD   (BYTES),A
       JR   ANXTLI
ANMA1  LD   HL,(AVAL1)
       LD   C,(IY-19)
       LD   A,(DE)
       LD   B,A
       INC  DE
       LD   A,(DE)
       OR   A
       JP   NZ,AINNUM
       DEC  DE
       LD   A,(DE)
       LD   C,0
       JP   AINNUM
ANOMAS PUSH IX
       POP  DE
       INC  DE
       INC  DE
       LD   HL,(PUNDES)
       CALL APOPRE
       INC  DE
       LD   A,(DE)
       CALL PONBYT
       LD   A,(BYTES)
       INC  A
       LD   (BYTES),A
ANXTLI CALL ASILIS
       JR   Z,ANXTL2

       BIT  1,(IY-17)
       JR   NZ,ANXTL2

       CALL APRLIN
ANXTL2 LD   A,(ERROR)
       OR   A
       JR   Z,ANXTL5

       BIT  1,(IY-17)
       JR   NZ,ANXTL4

       BIT  7,(IY-28)
       JR   Z,ANXTL3
       BIT  7,(IY-27)
       JR   NZ,ANXTL4
ANXTL3 PUSH AF
       CALL APRLIN
       POP  AF
ANXTL4 CALL APRERR
       LD   HL,(AERRS)
       INC  HL
       LD   (AERRS),HL
       CALL ESPTEC
       CP   199
       RET  Z
       JR   ANXT00
ANXTL5 BIT  6,(IY-28)
       JR   Z,ANXTL6
       BIT  7,(IY-28)
       JR   NZ,ANXTL6
       LD   HL,ABUF
       LD   IX,(ETINUM)
       CALL PONETI
       JR   NC,ANXTL6
       LD   A,8
       LD   (ERROR),A
       JR   ANXTL3
ANXTL6 CALL ASILIS
       JR   Z,ANXT00
       CALL APROBJ
ANXT00 LD   DE,(BYTES)
       LD   HL,(PUNDES)
       LD   B,H
       LD   C,L
       ADD  HL,DE
       LD   (PUNDES),HL
       LD   HL,(PUNDIR)
       ADD  HL,DE
       LD   (PUNDIR),HL
       CALL APONDP
       LD   HL,(ASIGDI)
       LD   (ADIREC),HL
       LD   HL,(ALINEA)
       INC  HL
       LD   (ALINEA),HL

       BIT  1,(IY-17)
       RET  NZ

       BIT  5,(IY-17)
       RET  NZ
       JP   ABUC
ABUSCO LD   A,(HL)
       CP   ","
       RET  Z
       CP   255
       RET  Z
       INC  HL
       JR   ABUSCO
ADEFBE POP  AF
       POP  BC
       POP  AF
       LD   A,5
       JP   ANONE1
ADEFE2 POP  AF
       JP   AERNUM
ADEFB  LD   HL,(ADIOP1)
       LD   BC,(PUNDES)
       XOR  A
ADEFB1 PUSH AF
       PUSH BC
       CALL ALEVAL
       PUSH AF
       BIT  4,(IY-28)
       JR   Z,ADEB11
       CALL ABUSCO
       JR   ADEFB2
ADEB11 CALL ANU8BI
       JR   NZ,ADEFBE
ADEFB2 POP  AF
       POP  BC
       JR   Z,ADEFBF
       JP   NC,ADEFE2
       PUSH HL
       LD   H,B
       LD   L,C
       LD   A,E
       CALL PONBYT
       LD   B,H
       LD   C,L
       POP  HL
       POP  AF
       INC  A
       JR   ADEFB1
ADEFBF POP  AF
       JR   ADEFW0
ADEFW  LD   HL,(ADIOP1)
       LD   BC,(PUNDES)
       XOR  A
ADEFW1 PUSH AF
       PUSH BC
       CALL ALEVAL
       PUSH AF
       BIT  4,(IY-28)
       JR   Z,ADEW11
       CALL ABUSCO
ADEW11 POP  AF
       POP  BC
       JR   Z,ADEFWF
       JP   NC,ADEFE2
       PUSH HL
       LD   H,B
       LD   L,C
       LD   A,E
       CALL PONBYT
       LD   A,D
       CALL PONBYT
       LD   B,H
       LD   C,L
       POP  HL
       POP  AF
       INC  A
       INC  A
       JR   ADEFW1
ADEFWF POP  AF
ADEFW0 JR   ADEFM2
ADEFM  LD   DE,(ADIOP1)
       LD   A,(DE)
       CP   34
       JP   NZ,AERCOM
       LD   HL,(PUNDES)
       LD   C,0
ADEFM1 INC  DE
       LD   A,(DE)
       CP   255
       JP   Z,AERCOM
       CP   34
       JR   Z,ADEFMF
       CALL PONBYT
       INC  C
       JR   ADEFM1
ADEFMF LD   A,C
ADEFM2 LD   (BYTES),A
       JR   AEQU1
AEQU   CALL ANUOP1
       JP   NC,AERNUM
AEQU0  LD   (ETINUM),DE
AEQU1  JP   ANXTLI
ADEFS  CALL ANUOP1
       JP   NC,AERNUM
       LD   (BYTES),DE
       JR   AEQU1
AENT   CALL ANUOP1
       JP   NC,AERNUM
       LD   (PUNDIR),DE
       LD   (DIRENT),DE
       JR   AEQU0
ALSTM  SET  7,(IY-27)
       JR   AEQU1
ALSTN  RES  7,(IY-27)
       JR   AEQU1
AORG   CALL ANUOP1
       JP   NC,AERNUM
       LD   (PUNORG),DE
       LD   (PUNDES),DE
       JR   AEQU1
ADPRM  SET  5,(IY-27)
       JR   AEQU1
ADPRN  RES  5,(IY-27)
       JR   AEQU1
ABUPSE LD   DE,ATAPSE
ABUPS1 CALL ACPSTR
       JR   NZ,ANONEM
       LD   L,C
       LD   H,0
       ADD  HL,HL
       LD   DE,ATAPS1
       ADD  HL,DE
       LD   A,(HL)
       INC  HL
       LD   H,(HL)
       LD   L,A
       JP   (HL)
ANONEM LD   A,7
ANONE1 LD   (ERROR),A
       JP   ANXTLI
AEROPE LD   A,1
       JR   ANONE1
AERCOM LD   A,4
       JR   ANONE1
AERDES LD   A,6
       JR   ANONE1
APRERR LD   HL,STRERR
       DEC  A
       CALL DPRCAD
       LD   A,13
       RST  16
       RET
STRERR DEFM "Invalid operand"
       DEFM "/ out of range"
       DEFB 255
       DEFM "Num. Syntax Err"
       DEFB 255
       DEFM "Undefined Symbol"
       DEFB 255
       DEFM "Missing Quotes"
       DEFB 255
       DEFM "DEFB out of range"
       DEFB 255
       DEFM "Range too big"
       DEFB 255
       DEFM "Unknown mnemonic"
       DEFB 255
       DEFM "Symbol exists"
       DEFB 255
DINCBY LD   A,(BYTES)
       INC  A
       LD   (BYTES),A
       RET
DNUMOP CP   9
       JR   NZ,DNUMO0
DNUM00 LD   A,1
       RET
DNUMO0 CP   18
       JR   Z,DNUM00
DNUMO1 CP   12
       JR   NC,DNUMO2
DNUM11 LD   A,7
       RET
DNUMO2 CP   15
       JR   NC,DNUMO3
DNUM21 LD   A,3
       RET
DNUMO3 CP   23
       JR   NC,DNUM11
       JR   DNUM21
DDAMAS EX   AF,AF'
       LD   A,(IX+2)
       CP   203
       JR   NZ,DDAMA2
DDAMA1 LD   A,%11111000
       RET
DDAMA2 CP   237
       JR   Z,DDAMA6
       LD   A,(IX+3)
       CP   192
       JR   NC,DDAMA6
       CP   64
       JR   C,DDAMA6
       LD   A,%11111000
       RET
DDAMA6 EX   AF,AF'
DDAM61 PUSH AF
       CALL DNUMOP
       CPL
       RLCA
       RLCA
       RLCA
       EX   AF,AF'
       POP  AF
       CALL ADAMAS
       JR   NC,DDAM62
       EX   AF,AF'
       RLCA
       RET
DDAM62 EX   AF,AF'
       RET
DIN2BY LD   A,(BYTES+1)
       INC  A
       LD   (BYTES+1),A
       RET
DLEEOP CALL ADATIP
       JR   C,DLEEO1
       PUSH AF
       CALL DIN2BY
       POP  AF
       CALL ASI8B
       JR   Z,DLEEOC
       CALL DIN2BY
DLEEOC SCF
       RET
DLEEO1 CALL ASOLO
       RET  C
       CALL DDAMAS
       OR   A
       RET
DPNOAS PUSH HL
       LD   (DDIRIN),HL
       SET  0,(IY-17)
       CALL DPRAF0
       POP  HL
       RET
DPRASM RES  0,(IY-17)
       LD   (DDIRIN),HL
       BIT  6,(IY-18)
       JR   Z,DPRAS0
       PUSH HL
       LD   HL,(DIRDBI)
       LD   DE,49152
       CALL CODEHL
       POP  HL
       JR   Z,DPRAS0
       LD   A,23
       LD   (NUMPAG+1),A
       PUSH HL
       CALL DBUDPR
       POP  HL
       JR   C,DPRAS0
       PUSH DE
       EX   DE,HL
       LD   DE,16384
       OR   A
       SBC  HL,DE
       POP  HL
       JR   NC,DPRAS1
       LD   DE,49152
       ADD  HL,DE
       LD   A,22
       JR   DPRAS2
DPRAS1 LD   A,16
DPRAS2 LD   (NUMPAG+1),A
       CALL PRLIN0
       CALL ACTPAG
       JR   DPRASF
DPRAS0 OR   A
       JR   DPRAF1
DPRASF LD   HL,(DDIRIN)
DPRAF0 SCF
DPRAF1 PUSH AF
       LD   A,1
       LD   (BYTES),A
       DEC  A
       LD   (BYTES+1),A
       LD   (DPREF),A
       LD   (APREXY),A
       RES  7,(IY-18)
       CALL OLDAHL
       CP   118
       JP   Z,DPASHT
       CP   221
       JR   Z,DPAS0
       CP   253
       JR   NZ,DPAS1
DPAS0  LD   (APREXY),A
       INC  HL
       CALL DINCBY
       CALL OLDAHL
DPAS1  CP   237
       JR   Z,DPAS2
       CP   203
       JR   NZ,DPAS3
DPAS2  LD   (DPREF),A
       CP   203
       JR   NZ,DPAS21
       LD   A,(APREXY)
       OR   A
       JR   Z,DPAS21
       INC  HL
       CALL OLDAHL
       LD   (ADES),A
       SET  7,(IY-18)
DPAS21 INC  HL
       CALL DINCBY
DPAS3  CALL OLDAHL
       LD   C,A
       INC  HL
       CALL OLDAHL
       LD   E,A
       INC  HL
       CALL OLDAHL
       LD   D,A
       LD   (AVAL1),DE
       DEC  HL
       DEC  HL
;AQUI: C=CODIGO DE INSTRUCCION
;      HL=DIRECC. DE INSTRUCC.
       LD   DE,ATABLA
DPASBU PUSH DE
       LD   A,(DE)
       CP   255
       JP   Z,DPASBF
       CALL AFINBU
       LD   A,(DE)
       LD   B,A
       INC  DE
       PUSH DE
       POP  IX
       LD   A,(DPREF)
       LD   E,A
DPASB1 XOR  A
       LD   (BYTES+1),A
       LD   A,(IX+2)
       CP   E
       JR   NZ,DPASB0
       LD   A,(IX+0)
       OR   A
       JR   NZ,DPASB2
DPAB11 LD   A,255
       JR   DCOMP
DPASB2 CALL DLEEOP
       LD   (AMAOP1),A
       PUSH AF
       LD   A,(IX+1)
       OR   A
       JR   NZ,DPASB3
       POP  AF
       JR   NC,DCOMP
       JR   DPAB11
DPASB3 CALL DLEEOP
       LD   (AMAOP2),A
       JR   NC,DPASB4
       POP  AF
       JR   C,DPAB11
       JR   DCOMP
DPASB4 LD   D,A
       POP  AF
       JR   C,DPASB5
       LD   (IY-20),%11000111
       LD   (IY-19),%11111000
       LD   A,%11000000
       JR   DCOMP
DPASB5 LD   A,D
DCOMP  AND  C
       CP   (IX+3)
       JR   Z,DPASF1
DPASB0 INC  IX
       INC  IX
       INC  IX
       INC  IX
       DJNZ DPASB1
       POP  DE
       PUSH IX
       POP  DE
       JR   DPASBU
DPASHT LD   DE,DHALT
       JR   DPASF2
DPASF1 POP  DE
       JR   DPRAF2
DPASBF POP  DE
       LD   DE,DNOP
DPASF2 LD   IX,DNOP2
DPRAF2 LD   A,(APREXY)
       OR   A
       JR   Z,DPRA27
       LD   A,C
       AND  %11000111
       CP   %01000110
       JR   NZ,DPRA21
DPRA20 SET  7,(IY-18)
       JR   DPRA27
DPRA21 LD   A,C
       AND  %11111000
       CP   %01110000
       JR   Z,DPRA20
       LD   A,(IX+0)
       CP   10
       JR   NZ,DPRA27
       PUSH DE
       LD   E,(IY-20)
       CALL DDANUM
       POP  DE
       CP   6
       JR   Z,DPRA20
DPRA27 BIT  0,(IY-17)
       JR   NZ,DPRA28
       CALL OLDAHL
       CP   205
       JR   NZ,DPRA28
       POP  AF
       PUSH AF
       EX   AF,AF'
       PUSH HL
       PUSH DE
       LD   DE,DVUELV
       INC  HL
       CALL OLDAHL
       CP   E
       JR   NZ,DPA271
       INC  HL
       CALL OLDAHL
       CP   D
       JR   NZ,DPA271
       EX   AF,AF'
       PUSH AF
       JR   NC,DPA270
       DEC  (IY+73)
DPA270 LD   (IY+72),6
       LD   A,"."
       RST  16
       LD   (IY+72),0
       POP  AF
       JR   NC,DPA271
       INC  (IY+73)
DPA271 POP  DE
       POP  HL
DPRA28 POP  AF
       JR   C,DPRAF3
       PUSH BC
       PUSH HL
       PUSH IX
       PUSH DE
       LD   HL,(DDIRIN)
       CALL PRNU16
       LD   A,":"
       RST  16
       LD   (IY+72),7
       POP  HL
       CALL DPR255
       LD   (IY+72),12
       POP  IX
       POP  HL
       POP  BC
       LD   A,(IX+0)
       OR   A
       JR   Z,DPRA29
       LD   E,(IY-20)
       CALL DPRAF4
       LD   A,(IX+1)
       OR   A
       JR   Z,DPRA29
       PUSH AF
       PUSH HL
       PUSH BC
       LD   A,","
       RST  16
       POP  BC
       POP  HL
       POP  AF
       LD   E,(IY-19)
       CALL DPRAF4
DPRA29 LD   A,13
       RST  16
DPRAF3 LD   HL,(BYTES)
       LD   A,(APREXY)
       OR   A
       JR   Z,DPRA31
       BIT  7,(IY-18)
       JR   Z,DPRA31
       INC  L
DPRA31 LD   A,L
       ADD  A,H
       RET
DPRAF4 PUSH IX
       PUSH HL
       PUSH BC
       CALL DPROPE
       POP  BC
       POP  HL
       POP  IX
       RET
DPROPE PUSH BC
       PUSH HL
       PUSH DE
       PUSH AF
       LD   L,A
       LD   H,0
       ADD  HL,HL
       LD   DE,DTAOPE-2
       ADD  HL,DE
       LD   A,(HL)
       INC  HL
       LD   H,(HL)
       LD   L,A
       PUSH HL
       POP  IX
       POP  AF
       POP  DE
       POP  HL
       POP  BC
       JP   (IX)
D1OP   LD   A,(AVAL1)
       JP   PNUM8
D3OP   CALL DDANUM
       JP   PNUM8
D4OP   LD   A,E
       CPL
       AND  C
       JP   PNUM8
D5OP   LD   A,"("
       RST  16
       CALL D1OP
DPRPAC LD   A,")"
       RST  16
       RET
D6OP   LD   A,"("
       RST  16
       CALL D7OP
       JR   DPRPAC
D7OP   LD   HL,(AVAL1)
       JP   PNUM16
D8OP   LD   A,(AVAL1)
       CALL D8B16B
       INC  HL
       INC  HL
       ADD  HL,BC
       JP   PNUM16
D9OP   CALL DDANUM
       LD   HL,ASTRIR
       JP   DPRCAD

D10OP  CALL DDANUM
       CP   6
       JP   Z,DPRHPA
       PUSH AF
       LD   HL,ASTR8B
       CALL DPRCAD
       POP  AF
       CP   4
       JR   Z,D10OP1
       CP   5
       RET  NZ
D10OP1 LD   A,(APREXY)
       OR   A
       RET  Z
       BIT  7,(IY-18)
       RET  NZ
       CP   221
       JR   NZ,D10OP2
       LD   A,"X"
       JR   D10OP0
D10OP2 LD   A,"Y"
D10OP0 RST  16
       RET
D11OP  LD   A,"A"
       RST  16
       RET
D12OP  CALL DDANUM
       CP   3
       JR   NZ,D14OP0
D13OP  LD   HL,ASTRSP
       JP   DPR255
D14OP  CALL DDANUM
D14OP0 CP   2
       JR   Z,DPRHL
       CP   3
       JR   NZ,D14OP1
       LD   HL,ASTRAF
       JP   DPR255
D14OP1 LD   HL,ASTR16
       JP   DPRCAD
D15OP  LD   HL,ASTRDE
       JP   DPR255
DPRHL  LD   A,(APREXY)
       OR   A
       JR   NZ,DPRHL1
       LD   HL,ASTRHL
       JR   DPRHL0
DPRHL1 CP   221
       JR   NZ,DPRHL2
       LD   HL,ASTRX
       JR   DPRHL0
DPRHL2 LD   HL,ASTRY
DPRHL0 JP   DPR255
D17OP  LD   HL,ASTRAF
       JP   DPR255
D18OP  CALL DDANUM
       LD   HL,ASTRBC
       JP   DPRCAD
D19OP  LD   A,"("
       RST  16
       CALL DPRHL
       JP   DPRPAC
D20OP  LD   HL,ASTSPP
       JP   DPR255
D21OP  LD   HL,ASTC
       JP   DPR255
D22OP  CALL DDANUM
       LD   HL,ASTRCO
       JP   DPRCAD
D24OP  LD   HL,ASTAFC
       JP   DPR255
DTAOPE DEFW D1OP,D1OP,D3OP,D4OP
       DEFW D5OP,D6OP,D7OP,D8OP
       DEFW D9OP,D10OP,D11OP
       DEFW D12OP,D13OP,D14OP
       DEFW D15OP,DPRHL,D17OP
       DEFW D18OP,D19OP,D20OP
       DEFW D21OP,D22OP,D22OP
       DEFW D24OP
DDANUM LD   A,E
       CPL
       AND  C
DDANU1 RRC  E
       RET  NC
       RRCA
       JR   DDANU1
DPRCAD OR   A
       JP   Z,DPR255
       LD   C,A
DPRCA1 LD   A,(HL)
       INC  HL
       CP   255
       JR   NZ,DPRCA1
       DEC  C
       JR   NZ,DPRCA1
       JP   DPR255
DPRDES BIT  7,A
       JR   Z,DPRDE1
       NEG
       PUSH AF
       LD   A,"-"
       JR   DPRDE2
DPRDE1 PUSH AF
       LD   A,"+"
DPRDE2 RST  16
       POP  AF
       JP   PNUM8
DPRHPA LD   A,"("
       RST  16
       CALL DPRHL
       LD   A,(APREXY)
       OR   A
       JR   Z,DPRHP0
       LD   A,(DPREF)
       CP   203
       JR   Z,DPRHP1
       LD   HL,(AVAL1)
       LD   A,H
       LD   (AVAL1),A
       LD   A,L
       JR   DPRHP2
DPRHP1 LD   A,(ADES)
DPRHP2 CALL DPRDES
DPRHP0 JP   DPRPAC
DREPAN CALL DPANRE
       RET  Z
DREPA1 CALL ILDIR
       JP   ACTPAG
DGUPAN CALL DPANRE
       RET  Z
       EX   DE,HL
       JR   DREPA1
DPANRE LD   HL,49152
       LD   DE,16384
       LD   BC,6912
       LD   A,(FLAGS3)
       RRCA
       RRCA
       AND  3
       RET  Z
       CP   1
       JR   NZ,DPANR1
       LD   (FINNDX),HL
       LD   A,19
       JR   DPANR2
DPANR1 CALL BTXTUS
       CP   2
       RET  Z
       LD   (DIRDBI),HL
       LD   A,23
DPANR2 LD   (NUMPAG+1),A
       OR   A
       RET
DREBUF LD   HL,BUFDEB
       LD   DE,BUFORI
       LD   BC,203
       RET
DDEBGU CALL DREBUF
       EX   DE,HL
       LD   HL,23552
       LDIR
       RET
DAJUR0 LD   A,(DREGR)
       PUSH AF
       INC  A
       AND  127
       LD   C,A
       POP  AF
       AND  128
       OR   C
       LD   (DREGR),A
       RET
DAJURR LD   A,R
       PUSH AF
       SUB  C
       AND  127
       LD   C,A
       POP  AF
       AND  128
       OR   C
       LD   R,A
       RET
DPUSRE PUSH AF
       PUSH BC
       PUSH DE
       PUSH HL
       EXX
       EX   AF,AF'
       PUSH AF
       PUSH BC
       PUSH DE
       PUSH HL
       PUSH IY
       PUSH IX
       LD   C,42
       CALL DAJURR
       LD   A,R
       LD   (DREGR),A
       LD   HL,10072
       EXX
       LD   IY,23610
       CALL DREBUF
       LD   HL,23552
       LDIR
       CALL DREBUF
       LD   DE,23552
       LDIR
       LD   A,I
       JP   PO,DPUSR1
       SET  2,(IY-17)
       JR   DPUSR2
DPUSR1 RES  2,(IY-17)
DPUSR2 EI
       LD   HL,(DRETOR)
       JP   (HL)
DPOPRE LD   A,(DREGR)
       LD   R,A
       LD   C,34
       CALL DAJURR
       POP  IX
       POP  IY
       POP  HL
       POP  DE
       POP  BC
       POP  AF
       EXX
       EX   AF,AF'
       POP  HL
       POP  DE
       POP  BC
       POP  AF
       JP   DPOPR1
DVUEL1 LD   SP,23552
       PUSH HL
       LD   HL,DVUEL2
       LD   (DRETOR),HL
       POP  HL
       JP   DPUSRE
DVUEL2 LD   HL,BUFORI+41
       BIT  4,(HL)
       JR   Z,DVUE20
       RES  4,(HL)
       CALL DGUPAN
       CALL CLS
       CALL PRIMES
       DEFM "BASIC error    @"
       LD   A,(BUFORI+58)
       CALL PRNUMA
       CALL PRIMES
       DEFM "#Ajust register "
       DEFM " PC and SP !@"
       CALL ESPTEC
       JP   DINI10
DVUE20 BIT  3,(IY-17)
       JR   Z,DVUEL3
       RES  3,(IY-17)
       LD   DE,(DNXTPC)
       CALL DPONPC
       JR   DVUEL4
DVUEL3 CALL DNOBRK
DVUEL4 JP   DINIC1
DNOBRK LD   HL,(DREGSP)
       CALL OLDAHL
       LD   E,A
       INC  HL
       CALL OLDAHL
       LD   D,A
       DEC  DE
       DEC  DE
       DEC  DE
       LD   (DMEMO),DE
       LD   A,D
       CALL OLDHLA
       DEC  HL
       LD   A,E
       CALL OLDHLA
       LD   HL,DBYBRK
       LD   BC,3
       LD   A,20
       EX   AF,AF'
       LD   A,17
       JP   SLDIR
DPR255 LD   A,(HL)
       INC  HL
       CP   255
       RET  Z
       CP   "#"
       JR   NZ,DPR25_
       LD   A,13
DPR25_ PUSH HL
       RST  16
       POP  HL
       JR   DPR255
DPR2ES LD   A,32
       RST  16
DPR2E1 LD   A,32
       RST  16
       RET
DPRSPP LD   A,":"
       RST  16
       JR   DPR2E1
DPRMEM PUSH HL
       CALL PRNU16
       CALL DPRSPP
       POP  HL
       PUSH HL
       LD   B,6
DPRME1 CALL OLDAHL
       PUSH HL
       PUSH BC
       CALL PRHEX8
       POP  BC
       PUSH BC
       BIT  5,(IY-18)
       JR   Z,DPRM11
       LD   A,B
       CP   1
       JR   Z,DPRM11
       LD   A,32
       RST  16
DPRM11 POP  BC
       POP  HL
       INC  HL
       DJNZ DPRME1
       CALL DPR2ES
       POP  HL
       LD   B,6
DPRME2 CALL OLDAHL
       PUSH BC
       PUSH HL
       CALL DPRASC
       POP  HL
       POP  BC
       INC  HL
       DJNZ DPRME2
       BIT  5,(IY-18)
       RET  NZ
       LD   A,13
       RST  16
       RET
DPRASC RES  7,A
       CP   32
       JR   NC,DPRAC1
       LD   A,"."
DPRAC1 RST  16
       RET
DPONPC LD   HL,(DREGSP)
       LD   A,E
       CALL OLDHLA
       INC  HL
       LD   A,D
       JP   OLDHLA
DDARPC LD   HL,(DREGSP)
       CALL OLDAHL
       LD   E,A
       INC  HL
       CALL OLDAHL
       LD   D,A
       RET
DBUDPR PUSH HL
       POP  IX
       LD   HL,(DIRDBI)
       LD   DE,49152
       OR   A
       SBC  HL,DE
       EX   DE,HL
       SRL  D
       RR   E
       SRL  D
       RR   E
       DEC  DE
       LD   BC,0
DBUDP1 LD   H,D
       LD   L,E
       PUSH HL
       OR   A
       SBC  HL,BC
       POP  HL
       RET  C
       ADD  HL,BC
       SRL  H
       RR   L
       PUSH HL
       PUSH BC
       CALL DBUDRE
       POP  BC
       JR   Z,DBUDSI
       POP  HL
       JR   C,DBUDP2
       LD   B,H
       LD   C,L
       INC  BC
       LD   A,B
       OR   C
       JR   Z,DBUDER
       JR   DBUDP1
DBUDP2 LD   D,H
       LD   E,L
       LD   A,D
       OR   E
       JR   Z,DBUDER
       DEC  DE
       JR   DBUDP1
DBUDSI POP  DE
       CALL ILDAHL
       LD   E,A
       INC  HL
       CALL ILDAHL
       LD   D,A
       OR   A
       RET
DBUDER SCF
       RET
DBUDRE ADD  HL,HL
       ADD  HL,HL
       LD   BC,49152
       ADD  HL,BC
       CALL ILDAHL
       LD   C,A
       INC  HL
       CALL ILDAHL
       LD   B,A
       INC  HL
       PUSH HL
       PUSH IX
       POP  HL
       OR   A
       SBC  HL,BC
       POP  HL
       RET
DPRREG PUSH AF
       LD   A,32
       RST  16
       POP  AF
       PUSH AF
       ADD  A,A
       LD   E,A
       LD   D,0
       LD   HL,DTAREG
       ADD  HL,DE
       LD   A,(HL)
       INC  HL
       LD   H,(HL)
       LD   L,A
       CALL DPR255
       POP  AF
       CP   3
       JR   C,DPRRE1
       CP   7
       JR   NC,DPRRE1
       LD   A,"'"
       RST  16
       RET
DPRRE1 LD   A,32
       RST  16
       RET
DPRAF  PUSH HL
       CALL PRHE16
       CALL DPR2ES
       POP  BC
       LD   HL,DTAFLG
       LD   B,8
DPRFL1 RLC  C
       JR   NC,DPRFL2
       LD   A,(HL)
       JR   DPRFL3
DPRFL2 LD   A,32
DPRFL3 PUSH BC
       PUSH HL
       RST  16
       POP  HL
       POP  BC
       INC  HL
       DJNZ DPRFL1
       LD   A,13
       RST  16
       RET
DTAFLG DEFM "SZxHxVNC"
DINICI LD   SP,23552
       LD   A,(IY-18)
       AND  %11110011
       LD   (IY-18),A
       RES  3,(IY-17)
       LD   HL,65534
       LD   (DREGSP),HL
       LD   DE,(PUNORG)
       LD   (DMEMO),DE
       CALL DPONPC
       CALL DDEBGU
       CALL 3435
       LD   HL,DINIC1
       LD   (DRETOR),HL
       JP   DPUSRE
DINIC1 CALL DGUPAN
DINI10 CALL CLS
       SET  3,(IY+48)
       RES  5,(IY-18)
       LD   HL,DSTRME
       CALL DPR255
       LD   HL,(DMEMO)
       PUSH HL
       CALL DPRMEM
       POP  HL
       PUSH HL
       CALL DPRASM
       LD   (DLONG),A
       LD   B,3
DINIC2 POP  HL
       LD   E,A
       LD   D,0
       ADD  HL,DE
       PUSH HL
       PUSH BC
       CALL DPRASM
       POP  BC
       DJNZ DINIC2
       POP  HL
       LD   A,13
       RST  16
       LD   A,10
       CALL DPRREG
       LD   HL,(23550)
       CALL DPRAF
       LD   A,32
       RST  16
       LD   HL,ASTRSP
       CALL DPR255
       LD   A,32
       RST  16
       LD   HL,(DREGSP)
       INC  HL
       INC  HL
       PUSH HL
       CALL PRNU16
       CALL DPRSPP
       POP  HL
       LD   B,3
DINI21 PUSH BC
       PUSH HL
       CALL DDARE1
       CALL PRNU16
       CALL DPR2ES
       POP  HL
       INC  HL
       INC  HL
       POP  BC
       DJNZ DINI21
       CALL DPNXPC
       CALL PRIMES
       DEFM " PC @"
       CALL DDARPC
       EX   DE,HL
       LD   (PUNDIR),HL
       PUSH HL
       CALL PRNU16
       CALL PRIMES
       DEFM "->@"
       LD   HL,(DNXTPC)
       CALL PRNU16
       LD   A,13
       RST  16
       POP  HL
       CALL DPRASM
       LD   A,13
       RST  16
       LD   B,9
       LD   A,1
       LD   HL,23552-20
DINIC3 PUSH BC
       PUSH AF
       PUSH HL
       CALL DPRREG
       POP  HL
       LD   E,(HL)
       INC  HL
       LD   D,(HL)
       INC  HL
       POP  AF
       CP   6
       PUSH AF
       PUSH HL
       EX   DE,HL
       JR   NZ,DINIC4
       CALL DPRAF
       JR   DINIC5
DINIC4 CALL DPRMEM
DINIC5 POP  HL
       POP  AF
       POP  BC
       INC  A
       DJNZ DINIC3
       LD   A,13
       RST  16
       LD   A,32
       RST  16
       CALL PRSPED
       LD   (IY+72),11
       CALL PRIMES
       DEFM "I=@"
       LD   A,"0"
       BIT  2,(IY-17)
       JR   Z,DINIC6
       INC  A
DINIC6 RST  16
       CALL PRIMES
       DEFM " P->@"
       LD   A,(IY-18)
       RRCA
       RRCA
       CPL
       AND  3
       CP   3
       JR   Z,DINIC7
       ADD  A,22
DINIC7 ADD  A,45
       RST  16
       CALL PRIMES
       DEFM " VD=@"
       LD   A,"0"
       BIT  6,(IY-18)
       JR   Z,DINIC8
       INC  A
DINIC8 RST  16
       CALL PRIMES
       DEFM " R @"
       LD   A,(DREGR)
       CALL PRHE8S
DEDIT  CALL PRIMES
       DEFB 22,22,0
       DEFM ">@"
DEDIT1 LD   HL,(23682)
       LD   (CX),HL
       CALL PONCUR
DEDIT2 LD   (IY-50),0
DEDIT3 LD   A,(23560)
       OR   A
       JR   Z,DEDIT3
       CALL BORCUR
       PUSH AF
       CALL DPRASC
       POP  AF
       LD   C,A
       LD   HL,DTATEC
DBUSTE LD   A,(HL)
       INC  HL
       CP   255
       JR   Z,DMENER
       CP   C
       JR   Z,DBUST1
       INC  HL
       INC  HL
       JR   DBUSTE
DMENER CALL MENER0
       CALL ESPTEC
       JP   DINI10
DBUST1 LD   A,(HL)
       INC  HL
       LD   H,(HL)
       LD   L,A
       CALL MEJPHL
       JP   DINI10
DFIN   CALL CLS
       JP   MENU
DHEXA  LD   A,(FLAGS3)
       XOR  16
       LD   (FLAGS3),A
       RET
DCO13  LD   HL,(DMEMO)
       INC  HL
DCO13_ LD   (DMEMO),HL
       RET
DCO7   LD   HL,(DMEMO)
       DEC  HL
       JR   DCO13_
DCO8   LD   HL,(DMEMO)
       LD   DE,8
DCO8_  OR   A
       SBC  HL,DE
       JR   DCO13_
DCO9   LD   HL,(DMEMO)
       LD   DE,8
DCO9_  ADD  HL,DE
       JR   DCO13_
DCO10  LD   HL,(DMEMO)
       LD   DE,(DLONG)
       LD   D,0
       JR   DCO9_
DCOMEM CALL DCALC
       JR   DCO13_
DCOCAL CALL DCALC
       CALL CALCU1
       JP   ESPTEC
DCOINS LD   A,":"
       RST  16
       LD   A,13
       RST  16
       LD   HL,(DMEMO)
DCOIN1 PUSH HL
       CALL PRNU16
       CALL DPRSPP
       CALL EDIVA0
       LD   HL,BUFEDI
       CALL VAL
       POP  HL
       LD   A,(BUFEDI)
       CP   32
       RET  Z
       LD   A,E
       CALL OLDHLA
       INC  HL
       JR   DCOIN1
DCOLIS CALL CLS
       LD   HL,(DMEMO)
       SET  5,(IY-18)
DCOLI1 LD   B,16
DCOLI2 PUSH HL
       PUSH BC
       CALL DPRMEM
       POP  BC
       POP  HL
       LD   DE,6
       ADD  HL,DE
       DJNZ DCOLI2
       CALL ESPTEC
       CP   199
       RET  Z
       JR   DCOLI1
DCOASM CALL CLS
       LD   HL,(DMEMO)
DCOAS1 LD   B,16
DCOAS2 PUSH HL
       PUSH BC
       CALL DPRASM
       POP  BC
       POP  HL
       LD   E,A
       LD   D,0
       ADD  HL,DE
       DJNZ DCOAS2
       CALL ESPTEC
       CP   199
       RET  Z
       JR   DCOAS1
DCOMAS RES  3,(IY-17)
DCOMA0 POP  BC
       BIT  2,(IY-17)
       JR   NZ,DCOMA1
       DI
DCOMA1 CALL DREPAN
       CALL DDEBGU
       CALL DREBUF
       EX   DE,HL
       LD   DE,23552
       LDIR
       JP   DPOPRE
DCOCOM LD   DE,(DMEMO)
       JP   DPONPC
DCOBRK LD   HL,(DMEMO)
DCOBR1 LD   DE,DBYBRK
       LD   BC,3
       LD   A,17
       EX   AF,AF'
       LD   A,20
       PUSH HL
       CALL SLDIR
       POP  HL
DCOBR2 PUSH HL
       LD   DE,23296
       OR   A
       SBC  HL,DE
       POP  HL
       JR   NC,DCOBR3
       CALL PRIMES
       DEFM "#Invalid "
       DEFM " Breakpoint!@"
       CALL ESPTEC
       SCF
       RET
DCOBR3 LD   DE,DVUELV
       LD   A,205
       CALL OLDHLA
       INC  HL
       LD   A,E
       CALL OLDHLA
       INC  HL
       LD   A,D
       CALL OLDHLA
       OR   A
       RET
DCALC  LD   A,":"
       RST  16
       CALL EDIVA0
       LD   HL,BUFEDI
       CALL VAL
       EX   DE,HL
       RET
DCOSHD LD   A,(IY-18)
       XOR  %01000000
       LD   (IY-18),A
       RET
DCOPAN CALL DREPAN
       JP   ESPTEC
DCOSHS CALL DCALC
       LD   A,L
       AND  3
       CP   3
       RET  Z
       RLCA
       RLCA
       LD   L,A
       LD   A,(IY-18)
       AND  %11110011
       OR   L
       LD   (IY-18),A
       RET
DCOSHT CALL DDARPC
       EX   DE,HL
       CALL DPNOAS
       LD   E,A
       LD   D,0
       ADD  HL,DE
       CALL DCOBR1
       RET  C
       JP   DCOMAS
DCO11  LD   HL,(DMEMO)
       PUSH HL
       LD   DE,10
       OR   A
       SBC  HL,DE
DCO110 CALL DPNOAS
       LD   E,A
       LD   D,0
       ADD  HL,DE
       POP  DE
       PUSH DE
       PUSH HL
       OR   A
       SBC  HL,DE
       POP  HL
       JR   C,DCO110
       POP  BC
       LD   E,A
       LD   D,0
       JP   DCO8_
DCOREG LD   A,":"
       RST  16
       CALL EDIVA0
       LD   HL,BUFEDI
       INC  HL
       INC  HL
       LD   A,(HL)
       CP   "'"
       PUSH AF
       LD   (HL),255
       JR   NZ,DCORE1
       INC  HL
DCORE1 INC  HL
       CALL VAL
       PUSH DE
       LD   HL,DTAREG
       LD   B,7
DCORE2 LD   E,(HL)
       INC  HL
       LD   D,(HL)
       INC  HL
       PUSH HL
       EX   DE,HL
       LD   DE,BUFEDI
       CALL ACOMP
       POP  HL
       JR   Z,DCORE3
       DJNZ DCORE2
       CALL DCORNO
       POP  DE
       POP  AF
       RET
DCORE3 POP  DE
       POP  AF
       LD   A,B
       JR   NZ,DCORE4
       CP   5
       JR   NC,DCORNO
DCOR31 LD   HL,23550-20
       JR   DCORE0
DCORE4 CP   7
       JR   NZ,DCORE5
       JP   DPONPC
DCORE5 CP   5
       JR   NC,DCOR31
       LD   HL,23550-12
DCORE0 LD   A,7
       SUB  B
       ADD  A,A
       LD   C,A
       LD   B,0
       ADD  HL,BC
       LD   (HL),E
       INC  HL
       LD   (HL),D
       RET
DCORNO CALL PRIMES
       DEFM "Register not"
       DEFM " found!@"
       JP   ESPTEC
DCOENS CALL DCOENP
       LD   HL,(PUNDES)
       PUSH HL
       LD   HL,(DMEMO)
       LD   (PUNDES),HL
       LD   (PUNDIR),HL
DCOEN1 LD   HL,(PUNDES)
       CALL PRNU16
       CALL DCOENP
       CALL EDIVAC
       LD   HL,BUFEDI
       LD   A,(HL)
       CP   32
       JR   Z,DCOENF
       PUSH HL
       LD   A,32
       LD   BC,30
       CPIR
       DEC  HL
       LD   (HL),255
       LD   HL,BUFEDI+30
DCOEN2 LD   A,(HL)
       CP   255
       JR   Z,DCOEN4
       CP   32
       JR   NZ,DCOEN3
       DEC  HL
       JR   DCOEN2
DCOEN3 INC  HL
DCOEN4 LD   (HL),13
       INC  HL
       LD   (HL),255
       LD   HL,ABUF
       LD   (HL),255
       INC  HL
       EX   DE,HL
       POP  HL
       LD   BC,30
       LDIR
       SET  7,(IY-28)
       RES  7,(IY-27)
       SET  1,(IY-17)
       CALL ABUC
       RES  1,(IY-17)
       JR   DCOEN1
DCOENF POP  HL
       LD   (PUNDES),HL
       RET
DCOENP LD   A,":"
       RST  16
       LD   A,13
       RST  16
       RET
DDARET LD   HL,(DREGSP)
       INC  HL
       INC  HL
DDARE1 CALL OLDAHL
       LD   E,A
       INC  HL
       CALL OLDAHL
       LD   H,A
       LD   L,E
       RET
DPNXPC RES  3,(IY-17)
       XOR  A
       LD   (DTIPIN),A
       CALL DDARPC
       EX   DE,HL
       PUSH HL
       CALL DPNOAS
       LD   E,A
       LD   D,0
       ADD  HL,DE
       LD   (DNXTPC),HL
       POP  HL
       CALL OLDAHL
       INC  HL
       CP   195
       JR   NZ,DPNXP1
DPNXP0 CALL DDARE1
DPNFIN LD   (DNXTPC),HL
       RET
DPNX00 LD   A,1
       LD   (DTIPIN),A
       JR   DPNXP0
DPNXP1 CP   205
       JR   Z,DPNX00
       CP   24
       JR   NZ,DPNXP2
DPNX10 CALL DDADIR
       JR   DPNFIN
DPNXP2 CP   201
       JR   NZ,DPNX21
DPNX20 CALL DDARET
       LD   A,3
DPN200 LD   (DTIPIN),A
       JR   DPNFIN
DPNX21 LD   B,A
       AND  %11100111
       CP   32
       JR   NZ,DPNXP3
       LD   A,B
       AND  %00011000
       CALL DDACOR
       JR   Z,DPNX10
DPNX22 INC  HL
       JR   DPNFIN
DPNXP3 LD   A,B
       AND  %11000111
       CP   194
       JR   NZ,DPNXP4
       LD   A,B
       AND  %00111000
       CALL DDACOR
       JR   Z,DPNXP0
DPNX30 INC  HL
       JR   DPNX22
DPNXP4 CP   196
       JR   NZ,DPNXP5
       LD   A,B
       AND  %00111000
       CALL DDACOR
       JR   Z,DPNX00
       JR   DPNX30
DPNXP5 CP   199
       JR   NZ,DPNX50
       LD   A,B
       AND  %00111000
       LD   L,A
       LD   H,0
       LD   A,2
       JR   DPN200
DPNX50 CP   192
       JR   NZ,DPNXP6
       LD   A,B
       AND  %00111000
       CALL DDACOR
       JR   Z,DPNX20
       JR   DPNFIN
DPNXP6 LD   A,B
       CP   16
       JR   NZ,DPNX60
       PUSH HL
       LD   HL,23549
       LD   A,(HL)
       POP  HL
       CP   1
       JR   Z,DPNX22
       JR   DPNX10
DPNX60 CP   233
       JR   NZ,DPNXP7
       LD   HL,23544
       JP   DPNXP0
DPNXP7 CP   221
       JR   NZ,DPNXP8
       CALL OLDAHL
       CP   233
       JR   NZ,DPNXP9
       LD   HL,23532
       JP   DPNXP0
DPNXP8 CP   253
       JR   NZ,DPNXP9
       CALL OLDAHL
       CP   233
       JR   NZ,DPNXP9
       LD   HL,23534
       JP   DPNXP0
DPNXP9 CP   237
       JR   NZ,DPNXSI
       CALL OLDAHL
       AND  %11000111
       CP   69
       JR   NZ,DPNXSI
       INC  HL
       JP   DPNX20
DPNXSI SET  3,(IY-17)
       RET
DCOPAS BIT  3,(IY-17)
       JR   NZ,DCOPSI
       CALL DAJUR0
       CALL DDARPC
       EX   DE,HL
       CALL OLDAHL
       CP   237
       JR   Z,DCOPA1
       CP   221
       JR   Z,DCOPA1
       CP   253
       JR   NZ,DCOPA2
DCOPA1 CALL DAJUR0
DCOPFI LD   DE,(DNXTPC)
       LD   (DMEMO),DE
       JP   DPONPC
DCOPA2 CP   16
       JR   NZ,DCOPA3
       LD   HL,23549
       DEC  (HL)
       JR   DCOPFI
DCOPA3 LD   A,(DTIPIN)
       OR   A
       JR   Z,DCOPFI
       CP   3
       JR   Z,DCOPA4
       CP   2
       JR   Z,DCOP30
       INC  HL
       INC  HL
DCOP30 INC  HL
       CALL DICALL
       JR   DCOPFI
DCOPA4 LD   HL,(DREGSP)
       INC  HL
       INC  HL
       LD   (DREGSP),HL
       JR   DCOPFI
DCOPSI CALL DDARPC
       PUSH DE
       LD   HL,(DNXTPC)
       LD   (DMEMO),HL
       OR   A
       SBC  HL,DE
       LD   B,H
       LD   C,L
       LD   DE,DBYSAL
       CALL DPONPC
       POP  HL
       LD   A,17
       EX   AF,AF'
       LD   A,20
       CALL SLDIR
       EX   DE,HL
       CALL DCOBR2
       JP   DCOMA0
DICALL EX   DE,HL
       LD   HL,(DREGSP)
       PUSH HL
       LD   A,E
       CALL OLDHLA
       INC  HL
       LD   A,D
       CALL OLDHLA
       POP  HL
       DEC  HL
       DEC  HL
       LD   (DREGSP),HL
       RET
DDADIR CALL OLDAHL
       INC  HL
       CALL D8B16B
       ADD  HL,BC
       RET
D8B16B LD   C,A
       LD   B,0
       CP   128
       RET  C
       LD   B,255
       RET
HLPDES LD   A,2
HELP   PUSH AF
       CALL CLS
       CALL PRIMES
       DEFM "HELP ON@"
       POP  AF
       LD   HL,STRHLP
       CALL DPRCAD
       CALL ESPTEC
       JP   CLS
DDACOR RRCA
       RRCA
       RRCA
DDACON LD   C,A
       SRL  A
       LD   E,A
       LD   D,0
       PUSH HL
       LD   HL,DTACON
       ADD  HL,DE
       LD   B,(HL)
       LD   HL,23550
       CALL OLDAHL
       POP  HL
       RLCA
DDACO1 RRCA
       DJNZ DDACO1
       AND  1
       LD   B,A
       LD   A,C
       AND  1
       XOR  B
       RET
DTACON DEFB 7,1,3,8
DTAREG DEFW DSTRPC,ASTRX
       DEFW ASTRY,ASTRHL
       DEFW ASTRDE,ASTR16
       DEFW ASTRAF,ASTRHL
       DEFW ASTRDE,ASTR16
       DEFW ASTRAF
DTATEC DEFB 199
       DEFW DFIN
       DEFB 35
       DEFW DHEXA
       DEFB 13
       DEFW DCO13
       DEFB 7
       DEFW DCO7
       DEFB 8
       DEFW DCO8
       DEFB 9
       DEFW DCO9
       DEFB 10
       DEFW DCO10
       DEFB 11
       DEFW DCO11
       DEFB "$"
       DEFW DCOASM
       DEFB "+"
       DEFW DCOMAS
       DEFB ":"
       DEFW DCOPAS
       DEFB ","
       DEFW DCOCOM
       DEFB "C"
       DEFW DCOCAL
       DEFB "E"
       DEFW DCOENS
       DEFB "H"
       DEFW HLPDES
       DEFB "I"
       DEFW DCOINS
       DEFB "L"
       DEFW DCOLIS
       DEFB "M"
       DEFW DCOMEM
       DEFB "R"
       DEFW DCOREG
       DEFB "S"
       DEFW DCOPAN
       DEFB "W"
       DEFW DCOBRK
       DEFB ">"
       DEFW DCOSHT
       DEFB 195
       DEFW DCOSHS
       DEFB 205
       DEFW DCOSHD
       DEFB 255
ASTR8B DEFB "B",255,"C",255
       DEFB "D",255,"E",255
ASTRH  DEFB "H",255,"L",255
ASTRH2 DEFM "(HL)"
       DEFB 255,"A",255,0
ASTR16 DEFB "B","C",255
ASTRDE DEFB "D","E",255
ASTRHL DEFB "H","L",255,0
ASTRAF DEFB "A","F",255
ASTRSP DEFB "S","P",255
ASTRCO DEFB "N","Z",255,"Z",255
       DEFB "N","C",255,"C",255
       DEFB "P","O",255,"P","E"
       DEFB 255,"P",255,"M",255
       DEFB 0
ASTRBC DEFM "(BC)"
       DEFB 255
       DEFM "(DE)"
       DEFB 255,0
ASTRIR DEFB "I",255,"R",255,0
ASTRX  DEFB "I","X",255,"H","X"
       DEFB 255,"L","X",255,0
ASTRY  DEFB "I","Y",255,"H","Y"
       DEFB 255,"L","Y",255,0
DSTRPC DEFB "P","C",255
DSTRME DEFB 32,"M","E"," ",255
ATASOL DEFB 4,1,64,1,128,1
       DEFB 4,2,8,2,16,2,128,2
ATASO2 DEFB "A",255,"D","E",255
       DEFB "H","L",255
       DEFM "(HL)"
       DEFB 255
ASTSPP DEFM "(SP)"
       DEFB 255
ASTC   DEFB "(","C",")",255
ASTAFC DEFB "A","F","'",255,0
DNOP   DEFM "NOP*"
       DEFB 255
DNOP2  DEFB 0
DHALT  DEFM "HALT"
       DEFB 255
ATAPSE DEFM "EQU"
       DEFB 255
       DEFM "DEFS"
       DEFB 255
ASTRDM DEFM "DEFM"
       DEFB 255
       DEFM "DEFW"
       DEFB 255
       DEFM "DEFB"
       DEFB 255
       DEFM "ENT"
       DEFB 255
       DEFM "LST+"
       DEFB 255
       DEFM "LST-"
       DEFB 255
       DEFM "ORG"
       DEFB 255
       DEFM "DPR+"
       DEFB 255
       DEFM "DPR-"
       DEFB 255,0
ATAPS1 DEFW AEQU,ADEFS,ADEFM
       DEFW ADEFW,ADEFB,AENT
       DEFW ALSTM,ALSTN,AORG
       DEFW ADPRM,ADPRN
ATABLA DEFM "LD"
       DEFB 255,14
       DEFB 10,10,0,64
       DEFB 12,7,0,1
       DEFB 10,1,0,6
       DEFB 16,6,0,42
       DEFB 6,16,0,34
       DEFB 11,18,0,10
       DEFB 18,11,0,2
       DEFB 11,6,0,58
       DEFB 6,11,0,50
       DEFB 12,6,237,75
       DEFB 6,12,237,67
       DEFB 11,9,237,87
       DEFB 9,11,237,71
       DEFB 13,16,0,249
       DEFM "INC"
       DEFB 255,2
       DEFB 10,0,0,4
       DEFB 12,0,0,3
       DEFM "DEC"
       DEFB 255,2
       DEFB 10,0,0,5
       DEFB 12,0,0,11
       DEFM "JR"
       DEFB 255,2
       DEFB 8,0,0,24
       DEFB 22,8,0,32
       DEFM "DJNZ"
       DEFB 255,1
       DEFB 8,0,0,16
       DEFM "JP"
       DEFB 255,3
       DEFB 7,0,0,195
       DEFB 23,7,0,194
       DEFB 19,0,0,233
       DEFM "CALL"
       DEFB 255,2
       DEFB 7,0,0,205
       DEFB 23,7,0,196
       DEFM "RET"
       DEFB 255,2
       DEFB 0,0,0,201
       DEFB 23,0,0,192
       DEFM "PUSH"
       DEFB 255,1
       DEFB 14,0,0,197
       DEFM "POP"
       DEFB 255,1
       DEFB 14,0,0,193
       DEFM "RST"
       DEFB 255,1
       DEFB 4,0,0,199
       DEFM "ADD"
       DEFB 255,3
       DEFB 11,10,0,128
       DEFB 11,1,0,198
       DEFB 16,12,0,9
       DEFM "ADC"
       DEFB 255,3
       DEFB 11,10,0,136
       DEFB 11,1,0,206
       DEFB 16,12,237,74
       DEFM "SUB"
       DEFB 255,2
       DEFB 10,0,0,144
       DEFB 1,0,0,214
       DEFM "SBC"
       DEFB 255,3
       DEFB 11,10,0,152
       DEFB 11,1,0,222
       DEFB 16,12,237,66
       DEFM "CP"
       DEFB 255,2
       DEFB 10,0,0,184
       DEFB 1,0,0,254
       DEFM "AND"
       DEFB 255,2
       DEFB 10,0,0,160
       DEFB 1,0,0,230
       DEFM "XOR"
       DEFB 255,2
       DEFB 10,0,0,168
       DEFB 1,0,0,238
       DEFM "OR"
       DEFB 255,2
       DEFB 10,0,0,176
       DEFB 1,0,0,246
       DEFM "RLCA"
       DEFB 255,1
       DEFB 0,0,0,7
       DEFM "RRCA"
       DEFB 255,1
       DEFB 0,0,0,15
       DEFM "RLA"
       DEFB 255,1
       DEFB 0,0,0,23
       DEFM "RRA"
       DEFB 255,1
       DEFB 0,0,0,31
       DEFM "SCF"
       DEFB 255,1
       DEFB 0,0,0,55
       DEFM "CCF"
       DEFB 255,1
       DEFB 0,0,0,63
       DEFM "RLC"
       DEFB 255,1
       DEFB 10,0,203,0
       DEFM "RRC"
       DEFB 255,1
       DEFB 10,0,203,8
       DEFM "RL"
       DEFB 255,1
       DEFB 10,0,203,16
       DEFM "RR"
       DEFB 255,1
       DEFB 10,0,203,24
       DEFM "SLA"
       DEFB 255,1
       DEFB 10,0,203,32
       DEFM "SRA"
       DEFB 255,1
       DEFB 10,0,203,40
       DEFM "SRL"
       DEFB 255,1
       DEFB 10,0,203,56
       DEFM "BIT"
       DEFB 255,1
       DEFB 3,10,203,64
       DEFM "RES"
       DEFB 255,1
       DEFB 3,10,203,128
       DEFM "SET"
       DEFB 255,1
       DEFB 3,10,203,192
       DEFM "LDIR"
       DEFB 255,1
       DEFB 0,0,237,176
       DEFM "LDDR"
       DEFB 255,1
       DEFB 0,0,237,184
       DEFM "LDI"
       DEFB 255,1
       DEFB 0,0,237,160
       DEFM "LDD"
       DEFB 255,1
       DEFB 0,0,237,168
       DEFM "CPIR"
       DEFB 255,1
       DEFB 0,0,237,177
       DEFM "CPDR"
       DEFB 255,1
       DEFB 0,0,237,185
       DEFM "CPI"
       DEFB 255,1
       DEFB 0,0,237,161
       DEFM "CPD"
       DEFB 255,1
       DEFB 0,0,237,169
       DEFM "EXX"
       DEFB 255,1
       DEFB 0,0,0,217
       DEFM "EX"
       DEFB 255,3
       DEFB 17,24,0,8
       DEFB 15,16,0,235
       DEFB 20,16,0,227
       DEFM "IN"
       DEFB 255,2
       DEFB 11,5,0,219
       DEFB 10,21,237,64
       DEFM "OUT"
       DEFB 255,2
       DEFB 5,11,0,211
       DEFB 21,10,237,65
       DEFM "INI"
       DEFB 255,1
       DEFB 0,0,237,162
       DEFM "OUTI"
       DEFB 255,1
       DEFB 0,0,237,163
       DEFM "IND"
       DEFB 255,1
       DEFB 0,0,237,170
       DEFM "OUTD"
       DEFB 255,1
       DEFB 0,0,237,171
       DEFM "INIR"
       DEFB 255,1
       DEFB 0,0,237,178
       DEFM "OTIR"
       DEFB 255,1
       DEFB 0,0,237,179
       DEFM "INDR"
       DEFB 255,1
       DEFB 0,0,237,186
       DEFM "OTDR"
       DEFB 255,1
       DEFB 0,0,237,187
       DEFM "RRD"
       DEFB 255,1
       DEFB 0,0,237,103
       DEFM "RLD"
       DEFB 255,1
       DEFB 0,0,237,111
       DEFM "IM0"
       DEFB 255,1
       DEFB 0,0,237,70
       DEFM "IM1"
       DEFB 255,1
       DEFB 0,0,237,86
       DEFM "IM2"
       DEFB 255,1
       DEFB 0,0,237,94
       DEFM "DAA"
       DEFB 255,1
       DEFB 0,0,0,39
       DEFM "CPL"
       DEFB 255,1
       DEFB 0,0,0,47
       DEFM "NEG"
       DEFB 255,1
       DEFB 0,0,237,68
       DEFM "EI"
       DEFB 255,1
       DEFB 0,0,0,251
       DEFM "DI"
       DEFB 255,1
       DEFB 0,0,0,243
       DEFM "HALT"
       DEFB 255,1
       DEFB 0,0,0,118
       DEFM "RETN"
       DEFB 255,1
       DEFB 0,0,237,69
       DEFM "RETI"
       DEFB 255,1
       DEFB 0,0,237,77
       DEFM "RETE"
       DEFB 255,1
       DEFB 0,0,237,85
       DEFM "SLL"
       DEFB 255,1
       DEFB 10,0,203,48
       DEFM "NOP"
       DEFB 255,1
       DEFB 0,0,0,0
       DEFB 255

EJECUT CALL COPPAG
       LD   HL,INICI0
       PUSH HL
       LD   HL,EJEINI
       LD   D,H
       LD   E,L
       LD   BC,16384
       LD   A,17
       EX   AF,AF'
       LD   A,20
       LD   (PAGNOR+1),A
       CALL SLDIR
       LD   A,17
       LD   (PAGNOR+1),A
       JP   ILDAHL


STRHLP DEFM " COMMAND LINE OPT"
       DEFM "S#####"
       DEFM " A Assemble# B Go"
       DEFM "to Basic# C Calcu"
       DEFM "late Expression#"
       DEFM " D Go to Monitor/"
       DEFM "Disassembler#"
       DEFM " E Go to Editor#"
       DEFM " F Modify Date#"
       DEFM " G n Set Tex"
       DEFM "t Block in use# I"
       DEFM " General Informac"
       DEFM "ion# L Load Sourc"
       DEFM "e Code# N New blo"
       DEFM "ck of Text#"
       DEFM " O Save Object Co"
       DEFM "de# S Save Source"
       DEFM " Code# "
       DEFM "T Show Symbol Tab"
       DEFM "le# Z DELETE"
       DEFM " Source Code!!!"
       DEFB 255
       DEFM " EDITOR#EXTENDED "
       DEFM "mode:# A Page Dow"
       DEFM "n# B Delete Block"
       DEFM "# C Copy Block"
       DEFM "# D Symbol Search"
       DEFM "# E - from start#"
       DEFM " F Search Text# G"
       DEFM " - from start#"
       DEFM " I Insert Mode on"
       DEFM "/off# J Next Symb"
       DEFM "ol# K Calculate E"
       DEFM "xpression# L Goto"
       DEFM " Line# "
       DEFM "N Find Next Text#"
       DEFM " P Mark Block Sta"
       DEFM "rt# Q PAge up# U "
       DEFM "End of Block# V "
       DEFM "Paste Block# X C"
       DEFM "ut Block# Z R"
       DEFM "estore Line#Norma"
       DEFM "l mode:# SS+D Del"
       DEFM "ete Line# SS+I In"
       DEFM "sert line# SS+Q "
       DEFM "Exit Editor "
       DEFB 255
       DEFM " MONITOR/DISSASM#"
       DEFM "# C Calculate Exp"
       DEFM "ression# $ Disass"
       DEFM "emble#SS+D Read "
       DEFM "debug data yes/no"
       DEFM "# E Assemble i"
       DEFM "nstruction# I Ins"
       DEFM "ert bytes#SS+K Ex"
       DEFM "ecute# L"
       DEFM " List in Hex and "
       DEFM "ASCII# M Change M"
       DEFM "emory Pointer#SS+"
       DEFM "N Set PC with ME#"
       DEFM "SS+Q Exit to Comm"
       DEFM "and# R Modify Re"
       DEFM "gister (RR=XXXX)#"
       DEFM " S Show Screen#SS"
       DEFM "+S Configure "
       DEFM "screen#(0"
       DEFM "0:None 1:Symbol "
       DEFM "2:Debug)#SS+T Bre"
       DEFM "akpoint after ins"
       DEFM "truc.# W Set B"
       DEFM "reakpoint#SS+Z Si"
       DEFM "ngle Step#SS+3 Nu"
       DEFM "mbers Hex/Decimal"
       DEFM "##EDIT/ENTER ME"
       DEFM "-+1 Byte#Cursor L"
       DEFM "eft/Right ME-+8 B"
       DEFM "ytes#Cursor Up/Do"
       DEFM "wn. ME-+1 Instruc"
       DEFM "."
       DEFB 255



