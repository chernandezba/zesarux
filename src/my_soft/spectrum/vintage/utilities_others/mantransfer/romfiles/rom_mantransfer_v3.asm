; Adaptación del mantransfer v3 a ROM

; Esta rutina solo graba juegos que funcionen con im1...
; CAMBIOS:
; - Etiquetas por ahí, etiquetas por allá... el ensamblador de ZX Spin
;	no soporta algunas directivas, y además hay que tener en cuenta
;	que aunque el loader se va a ejecutar desde la memoria de pantalla
;	realmente está grabado en la ROM.
; - Antes de grabar el código, se copia desde la ROM hasta la RAM.
; - Se cambia el modo de retorno (estamos en una NMI).

; CURIOSIDADES:
; - Todo el follón de IM1 sobra. Cuando iniciamos el Spectrum se pone con
;	I=$3f e IM1, y como no lo tocamos para nada después de cargar seguimos
;	con IM1. Establecerlo otra vez es inofensivo pero tampoco hace falta.
; - BUG: Supuestamente hay una rutina que comprueba el valor de IFF2 y
;	habilita las interrupciones. El problema es que como no las hemos
;	deshabilitado, siempre se quedarán habilitadas.

org $66
	; salto a la rutina de grabación
	jp inicio_nmi

org 5433
	db 16,2
	db 'Mantransfer v3 - NMI->SAV','E'+128
	; Mensaje en ROJO para indicar que es la versión (casi) original

;Contenido del codigo fuente de la ultima version de manstransfe, MANTR11.SP que hay en el mantransfev3.tap

;RUN        ORG  49152
;       ENT  16384

; Las rutinas del transfer ahora van en la dirección $3900.
; ENT no está soportado en el ensamblador de ZX Spin, así que lo quito.
; Pongo aquí las etiquetas que he cambiado.

IMMODE	equ 16384+_im_mode-inicio_loader
REGSP	equ 16384+_reg_sp-inicio_loader
ESTADI	equ 16384+_estadi-inicio_loader
INICIO	equ 16384
FINRUT	equ 16584
longitud_loader	equ inicio_nmi-inicio_loader

	org $3900

inicio_loader:	
_reg_sp:
       DEFW 0
;ESTA INSTRUCCION LA ALTERA
;EL EMULADOR, CON EL MODO
;ACTIVO, SI IM1 O IM2

;IMMODE IM1
_im_mode:
	im 1
; Como el ENT no ha funcionado, cambio la etiqueta IMMODE
; IM1 tampoco es un opcode válido del ensamblador de ZX Spin	
       RET
;
;INICIO RUTINA CARGA
;

CARGAR LD   SP,FINRUT

;METEMOS STACK AL FINAL DEL
;TODO
       LD   A,255
       SCF

       LD   IX,FINRUT
       LD   DE,65536-FINRUT
       CALL 1366
;RESTAURAR REGISTROS

RESTAU LD   SP,(REGSP)
       POP  HL
       POP  DE
       POP  BC
       POP  AF
       EX   AF,AF'
       EXX
       POP  IY
       POP  IX
       POP  HL
       POP  DE
       POP  BC
;REGISTRO I Y FLAGS QUE INDICAN
;SI DI O EI
       POP  AF

       LD   I,A

;MODO INTERRUPCIONES
       CALL IMMODE

;SI EI O DI
       JP   PO,ESTADI
       EI
; BUG!!! No hemos deshabilitado las interrupciones por lo que nunca se pondrán
; como DI. El código correcto es:
;	jp pe,_estadi
;	di
; Aún así no es del todo correcto. Lo bueno hubiera sido deshabilitarlas al
; inicio del cargador
       
_estadi:
       POP  AF
_ret_restaura:
       RET
;

inicio_nmi:
;RUTINA GRABAR SNAPSHOT
;
GRABAR PUSH AF
       LD   A,I
       PUSH AF
       PUSH BC
       PUSH DE
       PUSH HL
       PUSH IX
       PUSH IY
       EXX
       EX   AF,AF'
       PUSH AF
       PUSH BC
       PUSH DE
       PUSH HL
       LD   (REGSP),SP

;GRABAR PROGRAMA BASIC
       LD   A,0
       LD   IX,CABBAS
       LD   DE,17
       CALL SAVPAU
       LD   A,255
       LD   IX,INIBAS
       LD   DE,FINBAS-INIBAS
       CALL SAVPAU
; Se copia la rutina de carga a la memoria de pantalla
	ld hl,_im_mode
	ld de,16384+_im_mode-inicio_loader
	ld bc,inicio_nmi-_im_mode
	ldir
	
;BLOQUE BYTES PRIMERO
       LD   A,0
       LD   IX,CABCOD
       LD   DE,17
       CALL SAVPAU
       LD   A,255
       LD   IX,16384
       LD   DE,longitud_loader
       CALL SAVPAU
;Y BLOQUE DATOS
       LD   A,255
       LD   IX,FINRUT
       LD   DE,65536-FINRUT
       CALL SAVPAU
;DESPUES DE GRABAR QUE HACEMOS
;       JP   RESTAU
;	jp funcionaba si era una interrupción normal
;	esta rutina es una NMI, hay que retornar con RETI
	ld hl,77*256+237
	ld (16384+_ret_restaura-inicio_loader),hl
	jp 16384+RESTAU-inicio_loader
	; pongo un RETI a martillazos y salto	
	

SAVPAU CALL 1218
       LD   BC,0
PAUSA2 DEC  BC
       LD   A,B
       OR   C
       JR   NZ,PAUSA2
       RET
;PROGRAMA BASIC QUE CARGA
INIBAS DEFB 0,1
       DEFW FINBAS-LINEA1
LINEA1 DEFB #EF,34,34,#AF,#3A
;LOAD ""CODE:

       DEFB #F9,#C0,#B0,34
;RANDOMIZE USR VAL "

       DEFM "16389"
       DEFB 34
       DEFB 13
FINBAS
CABBAS DEFB 0
       DEFM "MANTRANSFE"
       DEFW FINBAS-INIBAS
       DEFW 1
       DEFW FINBAS-INIBAS
CABCOD DEFB 3
       DEFM "1234567890"
       DEFW longitud_loader
       DEFW 16384
       DEFW 0
