;Compile with Z88DK with the command:
;z80asm -b PrismBootRom.asm
;
; Prism Boot ROM by César Hernández Bañó and Jeff Braine
; Dark Side of the Boot Chimes by Andrew Owen
;
; Version 0.04
;	Merged in changes from César's Prism test ROM:
;      - Relocated variables so they don't get overwritten when
;         clearing page 2
;      - Updated rom-switch bootstrap to set ULA2 border back to 
; 		black and switch in page 0 at 0xC000 before booting
;	  - Translated code & messages from Spanish to English
;	  - Moved where the screen attributes are set for machines
;		without planar mode
;	Updated rom-switch bootstap to clear variable area 
;		before jumping to the selected ROM
;	Fixed small error in plane 3 of the rainbow
;    
; Version 0.03
;	Switched to using 16 colour planar mode for menu+logo
;	Moved menu to middle 3rd of screen
;	Started adding Prism logo in top 3rd of screen
; 	Fixed bug in clear_page_at_C000 (thanks César)
; Version 0.02 
;	Changed location for rom-switch bootstrap to 33000
;	Added clearing of all memory (including 0x5B00 - 0x7FFF)
;	Small modification to wording of options
;
; Version 0.01 by César Hernández Bañó
;	Initial version for testing Prism emulation in ZEsarUX
;
;
		org 0
	

		di
		ld sp,32768

		; CPU Speed: 3.5MHz
		xor a
		ld bc,36411
		out (c),a

		;Set ULA border to black (enables ULA2 border)
		xor a
		out (254),a

		; Setup screen
		; Resolution: 256x192
		ld a,32
		ld bc,36411
		out (c),a

		; Attribute decode: 256 colour mode II
		ld a,53
		ld bc,36411
		out (c),a

		; Palette: Default (hard-coded)
		ld a,64
		ld bc,36411
		out (c),a

		; Clear all VRAM
		; Set video write mask to 1111 (write to all 4 pages)
		ld a,95
		ld bc,36411
		out (c),a
		; Clear screen
		ld hl,16384
		ld de,16385
		ld bc,6911
		xor a
		ld (hl),a
		ldir

		; Set video write mask to 0000 (default - just VRAM0)
		ld a,80
		ld bc,36411
		out (c),a

		; Clear SRAM pages
		xor a
		call clear_8_pages
		ld a,64
		call clear_8_pages
		ld a,128
		call clear_8_pages
		ld a,192
		call clear_8_pages

		; Set attributes on plane 0 so text will be visible
		; on-screen if ROM used in non-ULA2 machine
		ld hl,22528
		ld de,22529 
		ld bc,767
		ld (hl),71
		ldir


		ld sp,0

		; Clear (non-VRAM) memory between 0x5B00 and 0x7FF
		call clear_5B00_to_7FFF

		;Reset variables
		xor a
		ld (rompage),a
		ld (vramaperture),a
		ld (compatibility128),a
		ld (cursory),a


		; Attribute decode: 256 colour mode II
		ld a,54
		ld bc,36411
		out (c),a

		; Palette: Prism's redefinable palette 
		; We'll just use the defaults for now
		ld a,66
		ld bc,36411
		out (c),a


		;Set border black
		xor a
		ld bc,254
		out (c),a
		ld bc,40507
		out (c),a

		; Display logo in top 8 lines

		call print_logo		


		call FourNotes



;print_start_cls:
;		call cls

print_start:
		;Reset coords
		xor a
		ld (print_coord_x),a
		ld a,17
           ld (print_coord_y),a

; We're in planar mode, so easiest way to 
; print in colour is to set the planar write mask

; Print the headings in bright yellow (colour 14) 
;  by OUT 36411,94

; Print the setting in bright cyan (colour 13)
;  by OUT 36411,93

		ld a,95
		ld bc,36411
		out (c),a
		call print_mensaje
		defm "  "
		defb 255
		ld a,94
		ld bc,36411
		out (c),a
		call print_mensaje
		defm "Flash Page: "
		defb 255

		ld a,93
		ld bc,36411
		out (c),a
		ld a,(rompage)
		call print_number

		ld a,95
		ld bc,36411
		out (c),a
		call print_mensaje
		defb 13
		defm "  "
		defb 255
		ld a,94
		ld bc,36411
		out (c),a
		call print_mensaje
		
		defm "VRAM aperture size: "
		defb 255

		ld a,93
		ld bc,36411
		out (c),a
		ld a,(vramaperture)
		call print_16K_6K

		ld a,95
		ld bc,36411
		out (c),a
		call print_mensaje
		defb 13
		defm "  "
		defb 255
		ld a,94
		ld bc,36411
		out (c),a
		call print_mensaje
		defm "Memory: "
		defb 255

		ld a,93
		ld bc,36411
		out (c),a
		ld a,(compatibility128)
		call print_128K_512K

		ld a,95
		ld bc,36411
		out (c),a
		call print_mensaje
		defb 13
		defm " "   ;Para borrar el cursor
		defb 13
		defm "  Boot"
		defb 255

		call print_cursor

		call wait_no_key
endless_loop:
		call wait_key

		;If cursor down key
                ld a,239
                in a,(254)
                and 16
                jr nz,nokey_6

		;Increment position if not at the end
		ld a,(cursory)
		cp 4
		jr z,endless_loop
		inc a
		ld (cursory),a
		jp print_start

nokey_6:
                ;If cursor up key
                ld a,239
                in a,(254)
                and 8
                jr nz,nokey_7

                ;Decrement position if not at the beginning
                ld a,(cursory)
                or a
                jr z,endless_loop
                dec a
                ld (cursory),a
                jp print_start


nokey_7:
		;If pressed enter
                ld a,191
                in a,(254)
                and 1
                jr nz,nokey_enter

		;Run function depending on cursor position
		;0: Flash page
		;1: VRAM aperture size
		;2: Amount of memory

		ld a,(cursory)
		or a
		jr nz,enter_no0

		;Increment rom page
		ld a,(rompage)
		cp 19  ;Maximum value: 19
		jr nz,enter_0_no_limit

		ld a,255  ;Set to 255 because when incrementing (at the next opcode) will be reset to 0


enter_0_no_limit:
		inc a

		ld (rompage),a
		jp print_start

enter_no0:

		cp 1
		jr nz,enter_no1

		;vram aperture
		ld a,(vramaperture)
		cpl 
		ld (vramaperture),a
		jp print_start

enter_no1:

		cp 2
		jr nz,enter_no2

		;128k compatibility
		ld a,(compatibility128)
		cpl
		ld (compatibility128),a
		jp print_start

enter_no2:

		;Position 3 empty
		;4 means apply changes
		cp 4
		jr nz,enter_no4

		jp apply_changes


enter_no4:
		jp print_start


nokey_enter:

		jr endless_loop

print_cursor:		
		xor a
		ld (print_coord_x),a
		ld a,(cursory)
		add a,17
		ld (print_coord_y),a

		ld a,'>'
		call print_character
		ret
		
print_16K_6K:
		or a
		jr z,print_16K_6K2
		call print_mensaje
		defm "16K"
		defb 255
		ret
print_16K_6K2:
		call print_mensaje
		defm "6K "
		defb 255
		ret

print_128K_512K:
		or a
		jr z,print_128K_512K2
		call print_mensaje
		defm "128K"
		defb 255
		ret
print_128K_512K2:
		call print_mensaje
		defm "512K"
		defb 255
		ret




print_number:

		;Primer digito. Sale de dividir entre 100

		push af
		;Dividimos
		ld e,100
		ld h,0
		ld l,a
		call Divide


		;En BC resultado
		ld a,c
		add a,'0'
		call print_character


		;Este resultado, lo multiplicamos por 100 y restamos a valor original
		ld l,c
		ld h,0
		call Multiplicar_10
		call Multiplicar_10
		pop af
		sub l

		;En A tenemos los siguientes dos digitos
		push af
		;Dividimos
                ld e,10
                ld h,0
                ld l,a
                call Divide


                ;En BC resultado
                ld a,c
                add a,'0'
                call print_character


                ;Este resultado, lo multiplicamos por 10 y restamos a valor original
                ld l,c
                ld h,0
                call Multiplicar_10
                pop af
                sub l

                ;En A tenemos el digito final
		add a,'0'
		call print_character

		ret

Divide:                          ; this routine performs the operation BC=HL/E
  ld a,e                         ; checking the divisor; returning if it is zero
  or a                           ; from this time on the carry is cleared
  ret z
  ld bc,-1                       ; BC is used to accumulate the result
  ld d,0                         ; clearing D, so DE holds the divisor
DivLoop:                         ; subtracting DE from HL until the first overflow
  sbc hl,de                      ; since the carry is zero, SBC works as if it was a SUB
  inc bc                         ; note that this instruction does not alter the flags
  jr nc,DivLoop                  ; no carry means that there was no overflow
  ret

Multiplicar_10:
	;Multiplicar HL por 10. HL*10=HL*(8+2)=HL*8+HL*2
	add hl,hl ;*2
	push hl   ;guardar *2 
	add hl,hl ;*4
	add hl,hl  ;*8
	pop de
	add hl,de  ;*10
	ret

clear_5B00_to_7FFF:
		ld hl,23296
		ld de,23297
		ld bc,9471
		xor a
		ld (hl),a
		ldir
		ret

clear_8_pages:
		ld (currentpage),a
		call clear_page_at_C000
		ld a,(currentpage)
		inc a
		ld (currentpage),a
		call clear_page_at_C000
		ld a,(currentpage)
		inc a
		ld (currentpage),a
		call clear_page_at_C000
		ld a,(currentpage)
		inc a
		ld (currentpage),a
		call clear_page_at_C000
		ld a,(currentpage)
		inc a
		ld (currentpage),a
		call clear_page_at_C000
		ld a,(currentpage)
		inc a
		ld (currentpage),a
		call clear_page_at_C000
		ld a,(currentpage)
		inc a
		ld (currentpage),a
		call clear_page_at_C000
		ld a,(currentpage)
		inc a
		ld (currentpage),a
		call clear_page_at_C000
		ret

clear_page_at_C000:
		; Change border+paper colour
		ld bc,40507
		out (c),a
		ld bc,32765
		out (c),a

		ld hl,49152
		ld de,49153
		ld bc,16383
		xor a
		ld (hl),a
		ldir
		ret

change_ula2_border_colour:
		ld bc,40507
		out (c),a
		ret

cls:
		ld hl,16384
		ld de,16385
		ld bc,6143
		ld (hl),0
		ldir

		inc hl
		inc de
		ld bc,767
		ld (hl),56
		ldir

		ret

	

print_mensaje:
		pop hl
		
print_message_loop:
		ld a,(hl)
		inc hl
		cp 255
		jr z,print_message_end
		call print_character
		jr print_message_loop

print_message_end:
		push hl
		ret




;Print character routine
;Modified registers: AF'
print_character:
		push af
		push bc
		push hl

		ex af,af'
		ld a,(print_coord_x)
		ld c,a
		ld a,(print_coord_y)
		ld l,a
		ex af,af'

		;If character 13, line feed
		cp 13
		jr nz,print_character_no13

		call print_character_line_feed
		jr print_character_continues  

print_character_no13:

		push bc
		push hl
		call print_char_lowlevel
		pop hl
		pop bc

		inc c
		ld a,c
		cp 32
		call z,print_character_line_feed

print_character_continues:
		ld a,c
		ld (print_coord_x),a

		ld a,l
		ld (print_coord_y),a

		pop hl
		pop bc
		pop af

		ret

print_character_line_feed:
		ld c,0
		inc l
		ld a,l
		cp 24
		ret nz
		ld l,23
		ret


print_char_lowlevel:

                                        ;Convert line, column to spectrum display
                                        ;     high byte          low byte
                                        ;bit  7 6 5 4 3 2 1 0  7 6 5 4 3 2 1 0
                                        ;     0 1 0 L L S S S  L L L C C C C C

		ex af,af'
		;In C, column, in L, line. In A, character
		ld a,l
		and 8+16
		or 64
		ld d,a

		ld a,l
		rlca
		rlca
		rlca
		rlca
		rlca

		and 128+64+32
		or c
		ld e,a
		ex af,af'

		;DE have the target

		;Get source on HL
		ld l,a
		ld h,0
		add hl,hl
		add hl,hl
		add hl,hl
		ld bc,character_table
		add hl,bc

		;Loop 8 pixels 

		ld b,8
print_char_lowlevel_loop:

		ld a,(hl)
		ld (de),a

		inc hl
		inc d

		djnz print_char_lowlevel_loop

		ret

wait_no_key:
                call read_all_keys
                cp 31
                jr nz,wait_no_key
                ret

wait_key:
                call read_all_keys
                cp 31
                jr z,wait_key
                ret

read_all_keys:
                xor a
                in a,(254)
                and 31
                ret

apply_changes:
		; Palette: Default (hard-coded)
		ld a,64
		ld bc,36411
		out (c),a

		; Clear all VRAM
		; Set video write mask to 1111 (write to all 4 pages)
		ld a,95
		ld bc,36411
		out (c),a
		; Clear screen
		ld hl,16384
		ld de,16385
		ld bc,6911
		xor a
		ld (hl),a
		ldir
		; Set video write mask to 0000 (default - just VRAM0)
		ld a,80
		ld bc,36411
		out (c),a

		;Set ula2 border colour to 0
		xor a
		call change_ula2_border_colour

		;Page sram 0
		xor a
		ld bc,32765
		out (c),a

		;Set attribute mode back to default
		ld a,48
		ld bc,36411
		out (c),a

		;Copy routine to 33000 and run it
		ld hl,apply_changes_start
		ld de,33000
		ld bc,apply_changes_end-apply_changes_start
		ldir
		jp 33000


apply_changes_start:
		;Set rom page
		ld bc,60987
		ld a,(rompage)
		out (c),a
		;Apply vram aperture, 128k compatibility

		;Modify register 1 from ula2
		ld a,16
		ex af,af'

		ld a,(vramaperture)
		or a
		jr z,apply_changes_start_no_aperture
		ex af,af'
		or 1
		ex af,af'

apply_changes_start_no_aperture:
		ld a,(compatibility128)
		or a
		jr z,apply_changes_start_no_compatibility128
		ex af,af'
		or 8
		ex af,af'


apply_changes_start_no_compatibility128:
		ex af,af'
		
		ld bc,36411
		out (c),a
	
		; Clear 5B00 - 7FFF 		
		ld hl,23296
		ld de,23297
		ld bc,9471
		xor a
		ld (hl),a
		ldir


		;Reset
		jp 0

apply_changes_end:

print_logo:
		; Plane 0 ("blue")
		xor a
		ld (print_coord_x),a
		ld (print_coord_y),a
		ld a,81
		ld bc,36411
		out (c),a
		call print_mensaje

		defm "              ;."
		defb 13
		defm "              </"
		defb 13
		defm "             ;##."
		defb 13
		defm "             <##/"
		defb 13
		defm "            ;####."
		defb 13
		defm "     J?~}|{"
		defb 96
		defm "#####/"
		defb 13

		defm "}|{"
		defb 96
		defm "ZYxXqQj"

		defm ";########"
		defb 96
		defm "{|}~?J"
		defb 13
		defm "qQj        <######/ jQqXxYZ#"
		defb 96
		defm "{|}"
		defm "          ;########.        jQqX"
		defm "          <########/"
		defb 13
		defm "          jjjjjjjjjj"
		
		defb 255

		call print_prism


		; Plane 1 ("red")
		xor a
		ld (print_coord_x),a
		ld (print_coord_y),a
		ld a,82
		ld bc,36411
		out (c),a
		call print_mensaje

		defm "              \\^"
		defb 13
		defm "              ]_"
		defb 13
		defm "             \\  G"
		defb 96 
		defm "{|}~?J" 
		defb 13
		defm "             ]  !########"
		defb 96 
		defm "{|}~?J"
		defm "            \\    G##############"
		defm "     J?~}|{"
		defb 96
		defm "="
		defm "    _ jQqXxYZ######"

		defm "}|{"
		defb 96
		defm "ZYxXqQj"
		defm "\\      ^        jQqXx"

		defm "qQj        ]      _"
		defb 13
		defm "          \\        ^"
		defb 13
		defm "          ]        _"
		defb 13
		defm "          jjjjjjjjjj"

		defb 13

		defb 255

		; Plane 2 ("green")
		xor a
		ld (print_coord_x),a
		ld (print_coord_y),a
		ld a,84
		ld bc,36411
		out (c),a
		call print_mensaje

		defm "              \\^"
		defb 13
		defm "              ]_"
		defb 13
		defm "             \\  ^"
		defb 13
		defm "             ]  !#"
		defb 96 
		defm "{|}~?J"
		defb 13
		defm "            \\    G########"
		defb 96
		defm "{|}~?"


		defm "     J?~}|{"
		defb 96
		defm "="

		defm "    !##############"

		defm "}|{"
		defb 96
		defm "ZYxXqQj"


		defm "\\      ^ jQqXxYZ#####"

		defm "qQj        ]      _         jQqX"
		defm "          \\        ^"
		defb 13
		defm "          ]        _"
		defb 13
		defm "          jjjjjjjjjj"

		defb 13
		defb 255

		; Plane 4 ("bright")
		xor a
		ld (print_coord_x),a
		ld (print_coord_y),a
		ld a,88
		ld bc,36411
		out (c),a
		call print_mensaje

		defm "              \\^"
		defb 13
		defm "              ]_"
		defb 13
		defm "             \\  G"
		defb 96 
		defm "{|}~?J"
		defb 13
		defm "             ]  _ jQqXxYZ"
		defb 96 
		defm "{|}~?J"
		defm "            \\    G"
		defb 96 
		defm "{|}~?J jQqXxY"
		defm "     J?~}|{"
		defb 96
		defm "="
		defm "    !########"
		defb 96 
		defm "{|}~?"
		defm "}|{"
		defb 96
		defm "ZYxXqQj"

		defm "\\      G#############"
		defm "qQj        ]      _ jQqXxYZ#####"
		
		defm "          \\        ^        jQqX"
		defm "          ]        _"
		defb 13
		defm "          jjjjjjjjjj"
		defb 255

		call print_prism


		ld a,80
		ld bc,36411
		out (c),a		
		ret	

print_prism:
		call print_mensaje
                defb 13
                defm "   ####( ####( # *###& #. ;#"
                defb 13
                defm "   #  ,# #  ,# # #-    #/ <#"
                defb 13
                defm "   #$##) #$##) # +###( #G@O#"
                defb 13
                defm "   #     # $%  #    ,# #!#=#"
                defb 13
                defm "   #     #  $% # $###) # [ #"
                defb 13
                defm " "
                defb 255

		ret


; plays four recognizable notes overlapping each other on channels C, A, B, C.
; fully relocateable
; works with interrupts on or off
; corrupts AF, BC, DE, HL, AF'
; runs from RAM or ROM
; 240 bytes in length
; no dependencies
; does not use the stack

FourNotes:
  DEFB 0x3E, 0x69, 0x21
  DEFW FourNotes + 0x1E
  DEFB 0x16, 0xBF, 0x01, 0xFD, 0xFF, 0xED, 0xA3, 0x42, 0xED, 0xA3, 0x08
  DEFB 0x01, 0x00, 0x10, 0x0B, 0x79, 0xB0, 0x20, 0xFB, 0x08, 0x3D, 0xA7, 0x20, 0xEA, 0xC9, 0x07, 0x3F
  DEFB 0x08, 0x00, 0x09, 0x00, 0x0A, 0x00, 0x04, 0xB7, 0x05, 0x03, 0x00, 0x7B, 0x01, 0x02, 0x02, 0x6B
  DEFB 0x03, 0x04, 0x07, 0x38, 0x0A, 0x0F, 0x08, 0x00, 0x09, 0x00, 0x0A, 0x0E, 0x08, 0x00, 0x09, 0x00
  DEFB 0x0A, 0x0D, 0x08, 0x00, 0x09, 0x00, 0x0A, 0x0C, 0x08, 0x00, 0x09, 0x00, 0x0A, 0x0B, 0x08, 0x00
  DEFB 0x09, 0x00, 0x0A, 0x0A, 0x08, 0x0F, 0x09, 0x00, 0x0A, 0x09, 0x08, 0x0E, 0x09, 0x00, 0x0A, 0x08
  DEFB 0x08, 0x0D, 0x09, 0x00, 0x0A, 0x07, 0x08, 0x0C, 0x09, 0x00, 0x0A, 0x06, 0x08, 0x0B, 0x09, 0x00
  DEFB 0x0A, 0x05, 0x08, 0x0A, 0x09, 0x0F, 0x0A, 0x04, 0x08, 0x09, 0x09, 0x0E, 0x0A, 0x03, 0x08, 0x08
  DEFB 0x09, 0x0D, 0x0A, 0x02, 0x08, 0x07, 0x09, 0x0C, 0x0A, 0x01, 0x08, 0x06, 0x09, 0x0B, 0x0A, 0x00
  DEFB 0x04, 0xA1, 0x05, 0x02, 0x0A, 0x0F, 0x08, 0x05, 0x09, 0x0A, 0x0A, 0x0E, 0x08, 0x04, 0x09, 0x09
  DEFB 0x0A, 0x0D, 0x08, 0x03, 0x09, 0x08, 0x0A, 0x0C, 0x08, 0x02, 0x09, 0x07, 0x0A, 0x0B, 0x08, 0x01
  DEFB 0x09, 0x06, 0x0A, 0x0A, 0x08, 0x00, 0x09, 0x05, 0x0A, 0x09, 0x08, 0x00, 0x09, 0x04, 0x0A, 0x08
  DEFB 0x08, 0x00, 0x09, 0x03, 0x0A, 0x07, 0x08, 0x00, 0x09, 0x02, 0x0A, 0x06, 0x08, 0x00, 0x09, 0x01
  DEFB 0x0A, 0x05, 0x08, 0x00, 0x09, 0x00, 0x0A, 0x04, 0x08, 0x00, 0x09, 0x00, 0x0A, 0x03, 0x08, 0x00
  DEFB 0x09, 0x00, 0x0A, 0x02, 0x08, 0x00, 0x09, 0x00, 0x0A, 0x01, 0x08, 0x00, 0x09, 0x00, 0x0A, 0x00




;Variables. Stored beyond 24576 to avoid being lost when switching VRAM aperture size
DEFC print_coord_x    = 24576
DEFC print_coord_y    = 24577
DEFC rompage          = 24578
DEFC vramaperture     = 24579
DEFC compatibility128 = 24580
DEFC cursory          = 24581
DEFC currentpage      = 24582

;256 byte padding. Reserved to have 0-31 additional characters in the future

character_table: defs 256


defb 0,0,0,0,0,0,0,0            
defb 15,15,7,7,3,3,1,1       
defb 0,36,36,0,0,0,0,0          
defb 255,255,255,255,255,255,255,255
defb 255,127,63,31,15,7,3,1       
defb 128,192,224,240,248,252,254,255
defb 255,254,252,248,240,224,192,128 
defb 1,3,7,15,31,63,127,255
defb 240,252,254,254,255,255,255,255          
defb 255,255,255,255,254,254,252,240     
defb 15,63,127,127,255,255,255,255
defb 255,255,255,255,127,127,63,15
defb 1,0,0,0,0,0,0,1           
defb 128,0,0,0,0,0,0,128           
defb 128,128,192,192,224,224,240,240          
defb 248,248,252,252,254,254,255,255          
defb 0,60,70,74,82,98,60,0      
defb 0,24,40,8,8,8,62,0         
defb 0,60,66,2,60,64,126,0      
defb 0,60,66,12,2,66,60,0       
defb 0,8,24,40,72,126,8,0       
defb 0,126,64,124,2,66,60,0     
defb 0,60,64,124,66,66,60,0     
defb 0,126,2,4,8,16,16,0        
defb 0,60,66,60,66,66,60,0      
defb 0,60,66,66,62,2,60,0       
defb 0,0,0,16,0,0,16,0          
defb 1,1,3,3,7,7,15,15         
defb 31,31,63,63,127,127,255,255           
defb 240,240,224,224,192,192,128,128         
defb 0,0,16,8,4,8,16,0  
defb 0,0,0,0,0,0,255,255          
defb 129,129,195,195,231,231,255,255
defb 0,60,66,66,126,66,66,0     
defb 0,124,66,124,66,66,124,0   
defb 0,60,66,64,64,66,60,0      
defb 0,120,68,66,66,68,120,0    
defb 0,126,64,124,64,64,126,0   
defb 0,126,64,124,64,64,64,0    
defb 255,255,127,127,63,63,31,31      
defb 0,66,66,126,66,66,66,0     
defb 0,62,8,8,8,8,62,0          
defb 0,0,0,0,0,0,0,255        
defb 0,68,72,112,72,68,66,0     
defb 0,64,64,64,64,64,126,0     
defb 0,66,102,90,66,66,66,0     
defb 0,66,98,82,74,70,66,0      
defb 255,255,254,254,252,252,248,248
defb 0,124,66,66,124,64,64,0    
defb 255,255,0,0,0,0,0,0     
defb 0,124,66,66,124,68,66,0    
defb 0,60,64,60,2,66,60,0       
defb 0,254,16,16,16,16,16,0     
defb 0,66,66,66,66,66,60,0      
defb 0,66,66,66,66,36,24,0      
defb 0,66,66,66,66,90,36,0      
defb 255,255,255,255,0,0,0,0      
defb 255,255,255,255,255,255,0,0     
defb 255,255,255,255,255,255,255,0
defb 255,255,126,126,60,60,24,24       
defb 1,1,2,2,4,4,8,8         
defb 16,16,32,32,64,64,128,128
defb 128,128,64,64,32,32,16,16 
defb 8,8,4,4,2,2,1,1  
defb 0,255,255,255,255,255,255,255    
defb 0,0,56,4,60,68,60,0        
defb 0,32,32,60,34,34,60,0      
defb 0,0,28,32,32,32,28,0       
defb 0,4,4,60,68,68,60,0        
defb 0,0,56,68,120,64,60,0      
defb 0,12,16,24,16,16,16,0      
defb 0,0,60,68,68,60,4,56       
defb 0,64,64,120,68,68,68,0     
defb 0,16,0,48,16,16,56,0       
defb 255,0,0,0,0,0,0,0          
defb 0,32,40,48,48,40,36,0      
defb 0,16,16,16,16,16,12,0      
defb 0,0,104,84,84,84,84,0      
defb 0,0,120,68,68,68,68,0      
defb 0,0,56,68,68,68,56,0       
defb 0,0,120,68,68,120,64,64    
defb 255,255,255,0,0,0,0,0        
defb 0,0,28,32,32,32,32,0       
defb 0,0,56,64,56,4,120,0       
defb 0,16,56,16,16,16,12,0      
defb 0,0,68,68,68,68,56,0       
defb 0,0,68,68,40,40,16,0       
defb 0,0,68,84,84,84,40,0       
defb 255,255,255,255,255,0,0,0     
defb 0,0,68,68,68,60,4,56       
defb 0,0,124,8,16,32,124,0      
defb 0,0,255,255,255,255,255,255         
defb 0,0,0,255,255,255,255,255            
defb 0,0,0,0,255,255,255,255  
defb 0,0,0,0,0,255,255,255          
defb 60,66,153,161,161,153,66,60

		;Fill up to 16kb
pad_zero:
		defs 16384-pad_zero
