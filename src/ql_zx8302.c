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



#include <stdio.h>


#include "ql_zx8302.h"
#include "ql.h"
#include "ql_i8049.h"
#include "debug.h"
#include "utils.h"
#include "timer.h"

//Emulacion ULA ZX8302 del QL

/*
* PC peripheral chip (zx8302 ic23) registers
        nolist

* The peripheral chip is selected by addresses in the range $18000 to $1bfff.
* The hardware does not decode bits 13-7 and 4-2 of addresses, so the following
* are actually the minimum addresses that perform the given functions.
* On some older ql's, bit 6 is also not decoded, but later m/c's distinguish
* the mc_stat register by decoding it as having bit 6 set.
* It could be unwise to assume that bits 13-7 can be anything but zero.
* It would be novel to redefine the $1800x versions by adding $1c, thus making
* the whole set more conveniently addressable.

* read addresses

pc_clock equ    $18000  real time clock in seconds (long word)

pc_ipcrd equ    $18020  IPC read address
pc_intr equ     $18021  bits 4..0 set as pending level 2 interrupts
pc_trak1 equ    $18022  microdrive read track1
pc_trak2 equ    $18023  microdrive read track2

* write addresses

*pc_clc0 equ    $18000  writing anything here resets the clock
*pc_clc1 equ    $18001  clock selected bytes of time
pc_tctrl equ    $18002  transmit control
pc_ipcwr equ    $18003  IPC write address

pc_mctrl equ    $18020  microdrive/link control register
*pc_intr equ    $18021  7..5 masks and 4..0 to clear interrupt
pc_tdata equ    $18022  transmit register

*mc_stat equ    $18023  on some ql's this controls the video display.
**FIX: chernandezba : This is $18063, not $18023

*************** pc_clock function:
* Reading the longword at pc_clock gets the current time.
* Writing anything to pc_clock resets the clock to zero.
* Bytes written to pc_clock+1 increment selected bytes of the current time as
* follows: a zero in bit 4 will increment the msb (to be read back from
* pc_clock) and subsequently a zero in bits 3, 2 and 1 will increment the bytes
* of lesser significance. Bits 7-5 and 0 are not significant.
* Clock setting, etc, could be speeded up by writing combinations of zero bits,
* but there is strange management of carries which make it far too awkward.
* As soon as the sign bit of a byte is set, the next more significant byte can
* no longer be incremented directly, only by propagation of carry.
* On a crash, there is a chance that there may be rubbish written to memory,
* and, what with the lack of decode as well, the clock often gets messed up.
* It gets reset to one of $0w0x0y0z, where w/x/y/z are small, typically 0.

*************** pc_tctrl transmit control register values
* The values set in bits 4..3 of the transmit control register select the
* device to which data written to pc_tdata applies.
* At boot, zero is written here. A zero was also written to pc_tdata, but this
* has been removed. It had the effect of sending a spurious null to the ser1
* port, which should not have been done.

pc..diro equ    7       direct output (e.g. to network)
*   ?    equ    6       0
*   ?    equ    5       0
pc..serb equ    4       0=serial io
pc..sern equ    3       serial port number
*baud2  equ     2       \
*baud1  equ     1        > three bit baud rate
*baud0  equ     0       /

pc.bmask equ    %00000111       baud rate mask
pc.notmd equ    %11100111       all bits except mode control
pc.mdvmd equ    %00010000       microdrive mode
pc.netmd equ    %00011000       network mode

*************** pc_ipcwr is used to send serial commands and data to IPC.
* The pattern %000011x0 is written here (with x always 1 when serial data is
* supposed to be being fetched), then bit 6 of pc_ipcrd is examined, waiting
* for it to go to zero, indicating that the data bit has been received by the
* IPC and/or that a return bit is ready in bit 7 of pc_ipcrd.
* Nibbles and bytes are sent most significant bit first.
* The sound control function has two 16 bit parameters and, in uncharacteristic
* fashion, these need their least significant byte to be sent first.
* At boot time, %00000001 is written here. at a guess, one of bits 0, 2 or 4 is
* used as a flag "data bit awaiting transmission".
* Note that the PC and the IPC are only connected by two wires, comctl and
* comdata, and the protocol on these is pretty obscure!

*   ?   equ     7       0
*   ?   equ     6       0
*   ?   equ     5       0
*   ?   equ     4       0
*   ?   equ     3       1
*   ?   equ     2       1
*   ?   equ     1       data bit for IPC (comctl/wr 
*   ?   equ     0       0

*************** pc_ipcrd and pc_mctrl read.
* This returns miscellaneous data bits appertaining to the IPC link, serial
* output control, microdrives and the network.

*   ?   equ     7       data bit from IPC
*   ?   equ     6       set when bit 7 is OK and IPC is ready to receive
pc..cts2 equ    5       CTS on port 2 (set if ser2 transmit held up)
pc..dtr1 equ    4       DTR on port 1 (set if ser1 transmit held up)
pc..gap  equ    3       gap: set normally, or gap is present on running mdv
pc..rxrd equ    2       microdrive read buffer ready
pc..txfl equ    1       xmit buffer full (mdv or ser)
*   ?    equ    0       network input bit. 1 if active. (netin)

*************** pc_mctrl microdrive control write register
* Selecting a drive is done by clocking a bit along the chain of drives.
* pc..sel is set on the first byte sent and clear on as many more as are
* needed to get to the right drive number.
* Clocking through eight bytes with pc..sel clear selects "mdv9_", or rather,
* as the ql only permits "mdv1_" to "mdv8_", deselects all.
* Anyone who connects more than six external drives will get a shock when the
* seventh one keeps getting selected!
* The clocking is done by pc..sclk transitioning from set to clear, and the
* hardware is expected to be able to manage with one transition each 25us.
* At boot, %0110 was written here, immediately followed by deselecting.
* now just the deselection is done.
*  ?    equ     7
*  ?    equ     6
*  ?    equ     5
*  ?    equ     4
pc..eras equ    3       microdrive erase
pc..writ equ    2       microdrive write
pc..sclk equ    1       microdrive select clock bit
pc..sel equ     0       microdrive select bit

pc.read equ     1<<pc..sclk             read (or idle) microdrive
pc.selec equ    pc.read!1<<pc..sel      select bit set
pc.desel equ    pc.read                 select bit not set
pc.erase equ    pc.read!1<<pc..eras     erase on / write off
pc.write equ    pc.erase!1<<pc..writ    erase and write

*************** pc_intr is read to supply the current set of pending
* interrupts, and is written to clear them and/or enable some of them.
* sv_intr holds the current setting and is managed carefully, on a bit basis.
* The three msbs must be set to enable their corresponding interrupts.
* On read, bit 7 is the baud rate clock, pulsing up and down once per bit.
* Bit 6 is normally set, but is clear while an mdv is running. dunno why...
* Bit 5 is the lsb of 1/65536ths of a second, i.e. straight x2 crystal.
* The five lsbs are set on read to indicate which interrupts are pending and
* writing a set bit will clear such an interrupt once it has been serviced.
* At boot, $1f is written here twice, then $c0 is put in sv_intr to start off
* with transmit and IPC interrupts enabled.
* The second write of $1f, one gathers, is because a bug in the h/w sometmes
* causes the first one to fail.
* The IPC interface interrupt deserves a mention. It was useless, and was
* ignored on Minerva until v1.94. Then Hermes finally got it right, and uses it
* to signal serial input buffer data ready (every 8 bytes).
* It actually also happens whenever data transfer goes on between the CPU and
* the IPC, which could be a problem for people who do not use mt.ipcom, but
* all internal routines clear this interrupt before returning.

pc.maskt equ    1<<7    transmit mask register
pc.maski equ    1<<6    interface mask register
pc.maskg equ    1<<5    gap mask register

pc.intre equ    1<<4    external interrupt register
pc.intrf equ    1<<3    frame interrupt register
pc.intrt equ    1<<2    transmit interrupt register
pc.intri equ    1<<1    interface interrupt register
pc.intrg equ    1<<0    gap interrupt register

        list

*/


unsigned char ql_pc_intr;
unsigned char ql_mc_stat;


void ql_debug_port(unsigned int Address)
{
	return;


	switch (Address) {
		case 0x18000:
		printf ("	PC_CLOCK		Real-time clock (Long word)\n\n");
		break;

		case 0x18002:
		printf ("	PC_TCTRL		Transmit control register\n\n");
		break;

		case 0x18003:
		printf ("	PC_IPCWR		IPC port - write only\n\n");
		break;

		case 0x18020:
		printf ("	PC_IPCRD		IPC port - read only\n");
		printf ("	PC_MCTRL		Microdrive control register - write only\n\n");
		break;

		case 0x18021:
		printf ("	PC_INTR		Interrupt register\n\n");
		//usleep(20000);
		break;

		case 0x18022:
		printf ("	PC_TDATA		Transmit register - write only\n");
		printf ("	PC_TRAK1		Read microdrive track 1\n\n");
		break;

		case 0x18023:
		printf ("	PC_TRAK2		Read microdrive track 2\n\n");
		break;

		case 0x18063:
		printf ("	MC_STAT		Master chip status register\n\n");
		break;

		default:
		printf ("Unknown i/o port %08XH\n",Address);
		break;
	}
}





void ql_zx8032_write(unsigned int Address, unsigned char Data)
{
	

	int anterior_video_mode;

	switch (Address) {

  	    case 0x18003:
			//printf ("Escribiendo IPC. Valor: %02XH PC=%06XH\n",Data,get_pc_register() );
			ql_write_ipc(Data);

		break;

		case 0x18020:
			//printf ("Writing on 18020H - Microdrive control register - write. Data: %02XH\n",Data);
		break;

		case 0x18021:
/*
$18021 WRITE Interrupt mask and clear

Controls interrupt masking and clears interrupt bits once the relevant interrupt source has been serviced. Based on the Minerva sources, the interrupts are edge triggered and should be cleared by writing an appropriate bit with 1.
Since this register also contains 3 interrupt enable bits, but is write only, a copy of the data written needs to be kept in the SV_INTR system variable so that the proper enable bits can be appended to the clear interrupt bits when writing this register.
In any case, 8 bits are implemented:

0..4 clear the interrupt when written with 1.
The bit assignments correspond to the ones when the register is read:
0 = gap, 1 = interface, 2=transmit, 3=frame, 4=external

5 = gap mask, enables the gap interrupt if written as 1.
6 = interface mask, enables the interface interrupt if written as 1.
7 = transmit mask, enables the transmit interrupt if written

*/
		  //printf ("Escribiendo pc_intr. Valor: %02XH\n",Data);
/*
*pc_intr equ    $18021  7..5 masks and 4..0 to clear interrupt
*/

            //Invertimos bits. Asi lo que se quiera poner a 0, entraba como 1
            Data ^=255;

            //Y no tocamos los 3 bits superiores
            Data |=(128+64+32);

            ql_pc_intr &=Data;

		break;

		case 0x18063:
			//MC_STAT		Master chip status register
			anterior_video_mode=(ql_mc_stat>>3)&1;

			ql_mc_stat=Data;

			int video_mode=(ql_mc_stat>>3)&1;

			if (video_mode!=anterior_video_mode) {
				//0=512x256
				//1=256x256
				/*
					0 = 4 colour (mode 4) =512x256
				  1 = 8 colour (mode 8) =256x256
				*/
				if (video_mode==0) screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Setting mode 4 512x256");
				else screen_print_splash_text_center(ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,"Setting mode 8 256x256");
			}

		break;
	}

}


moto_byte ql_zx8032_readbyte(unsigned int Address)
{


	unsigned char valor_retorno=0;


    long timer_seconds;


	switch (Address) {

        case 0x18000:
        case 0x18001:
        case 0x18002:
        case 0x18003:
            //RTC. 
            /*
            pc_clock equ    $18000  real time clock in seconds (long word)
            *************** pc_clock function:
* Reading the longword at pc_clock gets the current time.
* Writing anything to pc_clock resets the clock to zero.
* Bytes written to pc_clock+1 increment selected bytes of the current time as
* follows: a zero in bit 4 will increment the msb (to be read back from
* pc_clock) and subsequently a zero in bits 3, 2 and 1 will increment the bytes
* of lesser significance. Bits 7-5 and 0 are not significant.
* Clock setting, etc, could be speeded up by writing combinations of zero bits,
* but there is strange management of carries which make it far too awkward.
* As soon as the sign bit of a byte is set, the next more significant byte can
* no longer be incremented directly, only by propagation of carry.
* On a crash, there is a chance that there may be rubbish written to memory,
* and, what with the lack of decode as well, the clock often gets messed up.
* It gets reset to one of $0w0x0y0z, where w/x/y/z are small, typically 0.
            */

            timer_seconds=timer_get_current_seconds();

            //En QL: Time starts at 00:00 1 January 1961
            //En Unix, empieza en 1970
            //Hay que sumar los segundos entre esas dos fechas

            //Son 283.996.800 segundos
            timer_seconds += 283996800;

            int offset_byte=Address-0x18000;

            //MSB. Primer byte es el mas significativo (rotar 24 bits derecha)
            //Siguiente rotar 16. Siguiente 8. Y final 0
            int reverse_offset=3-offset_byte;

            int rotar_bits=reverse_offset*8;

            //printf("RTC. Address: %0X Rotar %d\n",Address,rotar_bits);

            moto_byte valor_retorno=(timer_seconds>>rotar_bits) & 0xFF;

            return valor_retorno;



        break;                        

		case 	0x18020:
			//printf ("Leyendo IPC. PC=%06XH\n",get_pc_register());
			//temp
			//return 0;
			return ql_read_ipc();

		break;


		case 0x18021:
			//printf ("Read PC_INTR		Interrupt register. Value: %02XH\n\n",ql_pc_intr);

            //printf ("Read PC_INTR		Interrupt register.\n\n");


                        //temp solo al pulsar enter
                        ////puerto_49150    db              255  ; H                J         K      L    Enter ;6
                        //if ((puerto_49150&1)==0) {
/*
* read addresses
pc_intr equ     $18021  bits 4..0 set as pending level 2 interrupts
pc.intre equ    1<<4    external interrupt register
pc.intrf equ    1<<3    frame interrupt register
pc.intrt equ    1<<2    transmit interrupt register
pc.intri equ    1<<1    interface interrupt register
pc.intrg equ    1<<0    gap interrupt register

$18021 WRITE Interrupt mask and clear

Controls interrupt masking and clears interrupt bits once the relevant interrupt source has been serviced. Based on the Minerva sources, the interrupts are edge triggered and should be cleared by writing an appropriate bit with 1.
Since this register also contains 3 interrupt enable bits, but is write only, a copy of the data written needs to be kept in the SV_INTR system variable so that the proper enable bits can be appended to the clear interrupt bits when writing this register.
In any case, 8 bits are implemented:

0..4 clear the interrupt when written with 1.
The bit assignments correspond to the ones when the register is read:
0 = gap, 1 = interface, 2=transmit, 3=frame, 4=external

5 = gap mask, enables the gap interrupt if written as 1.
6 = interface mask, enables the interface interrupt if written as 1.
7 = transmit mask, enables the transmit interrupt if written


*/

/*
Esto se lee en la rom asi:
L00352 MOVEM.L D7/A5-A6,-(A7)       *#: ext2int entry
XL00352 EQU L00352
       MOVEA.L A7,A5
       MOVEA.L #$00028000,A6
       MOVE.B  $00018021,D7
       LSR.B   #1,D7
       BCS     L02ABC
       LSR.B   #1,D7
       BCS     L02CCC               * 8049 interrupt

	Por tanto la 8049 interrupt se interpreta cuando bit 1 activo
*/


            return ql_pc_intr;
			
		break;

	}


	return valor_retorno;
}


