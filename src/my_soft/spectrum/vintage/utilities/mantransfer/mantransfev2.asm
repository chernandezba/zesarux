;listado extraido del codigo fuente contenido en cinta mantransfer2.tzx


*C-
*D+
*L+
    ORG    65230
INIINT    LD    A,254
    LD    I,A
    IM    2
    RET
FININT    IM    1
    RET
START    PUSH    AF
    PUSH    BC
    PUSH    DE
    PUSH    HL
    PUSH    IX
    PUSH    IY
    EXX
    EX    AF,AF'
    PUSH    AF
    PUSH    BC
    PUSH    DE
    PUSH    HL
    LD    HL,(DIRINT)
    CALL    111
    POP    HL
    POP    DE
    POP    BC
    POP    AF
    EX    AF,AF'
    EXX
    POP    IY
    POP    IX
    POP    HL
    POP    DE
    POP    BC
    POP    AF
    JP    56
DIRINT    DEFW    RUTINA
INTER    DEFW    START
RUTINA    DI
    LD    A,3
    OUT    (254),A
    XOR    A
    OUT    (254),A
    LD    A,253
    IN    A,(254)
    AND    7
    JR    Z,GRABA
    EI
    RET
GRABA    LD    (16384),SP
    XOR    A
    LD    IX,BASIC
    LD    DE,17
    CALL    1218
    LD    A,255
    LD    IX,LINEAS
    LD    DE,FIN-LINEAS
    CALL    1218
    XOR    A
    LD    IX,16384
    LD    DE,6912
    CALL    1218
    XOR    A
    LD    IX,25000
    LD    DE,40536
    CALL    1218
    XOR    A
    LD    IX,23296
    LD    DE,1704
    CALL    1218
    EI
    RST    8
    DEFB    8
BASIC    DEFB    0
    DEFM    "MANTRANSFE"
    DEFW    FIN-LINEAS
    DEFW    2
    DEFW    FIN-LINEAS
LINEAS    DEFW    0
    DEFW    LINEA2-LINEA1
LINEA1    XOR    A
    SCF
    LD    IX,16384
    LD    DE,6912
    CALL    2050
    XOR    A
    SCF
    LD    IX,25000
    LD    DE,40536
    CALL    2050
    LD    SP,(16384)
    XOR    A
    SCF
    LD    IX,23296
    LD    DE,1704
    JP    1366
    DEFB    13
LINEA2    DEFB    0,2
    DEFW    FIN-COMAN
COMAN    DEFB    253,176,34
    DEFM    "24999"
    DEFB    34,58
    DEFB    249,192,176
    DEFB    34
    DEFM    "23759"
    DEFB    34
    DEFB    13
FIN    DEFB    0
