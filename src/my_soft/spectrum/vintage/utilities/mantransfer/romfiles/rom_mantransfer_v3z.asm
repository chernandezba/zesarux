; Adaptación del mantransfer v3 a ROM

; CAMBIOS:
; - Etiquetas por ahí, etiquetas por allá... el ensamblador de ZX Spin
;	no soporta algunas directivas, y además hay que tener en cuenta
;	que aunque el loader se va a ejecutar desde la memoria de pantalla
;	realmente está grabado en la ROM.
; - Antes de grabar el código, se copia desde la ROM hasta la RAM.
; - Se añade una rutina de ¿detección? de IM copiada del Pokeador Automático.
; - Se corrige el bug de las interrupciones
; - Para hacerlo más compatible, ya no se usa el stack del programa.
; - Se quita el CALL del IMMODE, para ahorrar unos bytes (que luego
;	desperdicio con la posición del stack).
; - Se cambia el modo de retorno (estamos en una NMI).

org $66
	; salto a la rutina de grabación
	jp inicio_nmi

org 5433
	db 16,1
	db 'Mantransfer v3z - NMI=SAV','E'+128
	; Mensaje en AZUL para indicar modificaciones a stack

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
longitud_loader	equ 200
stack_transfer	equ 16484
FINRUT	equ 16584
; Ahora el loader incluye el stack.
; Además, se consume menos pantalla (cuando grabamos el loader también se
;	graba parte de la pantalla).

	org $3900

inicio_loader:	
_reg_sp:
       DEFW 0
;ESTA INSTRUCCION LA ALTERA
;EL EMULADOR, CON EL MODO
;ACTIVO, SI IM1 O IM2

;
;INICIO RUTINA CARGA
;

CARGAR LD   SP,stack_transfer-22
; El stack para la rutina de carga se pone justo debajo de los
;	registros que hemos preservado.

       LD   A,255
       SCF

       LD   IX,FINRUT
       LD   DE,65536-FINRUT
       CALL 1366
;RESTAURAR REGISTROS

RESTAU
	di
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
_im_mode:
	im 1
; Ahora se pone directamente el modo correcto

;SI EI O DI
       JP   PO,ESTADI
       EI
_estadi:
       POP  AF
       LD   SP,(REGSP)
; SP debe ser el último registro a recuperar
_ret_restaura:
       RET
;

inicio_nmi:
;RUTINA GRABAR SNAPSHOT
;
GRABAR LD   (REGSP),SP
; Lo primero de todo, se preserva SP
	ld sp,stack_transfer
; Ahora ponemos el stack en el área que hemos reservado
;	y preservamos los registros
       PUSH AF
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
	ld hl,CARGAR
	ld de,16384+CARGAR-inicio_loader
	ld bc,inicio_nmi-CARGAR
	ldir
	
; Detección cutre de IM ;)
	ld a,i
	cp $3f
	jr z,era_im1
	; si i=$3f asumimos que es im1
	ld hl,16384+_im_mode-inicio_loader+1
	ld (hl),94
	; si no, componemos el IM2
era_im1:
       
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

       DEFM "16386"
; Se modifica la dirección de inicio       
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
