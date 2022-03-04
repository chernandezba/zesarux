Beginning Assembler on the Spectrum
===================================
           ,        ,       ~,
Thanks to Cesar Hernandez Bano

(Yes that's him the creator of ZEsarUX the first full Next emulator)

Who worked in 1996 for around 50 days to produce a 128K assembler, monitor and
dissasembler - which was called SPED52, as that is how many saves he made...
we can now provide an on machine tool to learn a bit of assembly language.

This is no toy either - in the spirit of Open Source it includes its own
source code under GNU GPL - yes that's correct it is written in itself!

Anyway all spelling errors in SPED53 EN(glish) and this document are due to
Tim Gilberts.  The original ES (Spanish) version is also provided and they
should be functionally identical, only the text has changed.

To run it:
----------

Use the Browser to change the speed to 14Mhz - the editor and the assembly are
really improved with this speed boost.  If you forget just tap F8 of course.

Then use Mode 0 to load SPED53EN.TAP (or .RUN SPED53EN.TAP) from the tools
directory. You can also simply choose SPED53.BAS and it will ask you
which language version you want to use.

If you exit to BASIC (Capital B) then it will print an address:

Return to 49453

Note that it leaves you in 48K BASIC so of course you need to use the DOT
commands like .ls, .tapein, .cd etc

The printed address is the one you need to type in RANDOMIZE USR xxxxx to go
back to the system with your Source Code intact - The source code is the
list of assembly language instructions you want it to turn into the long
stream of numbers that the Z80 microprocessor at the heart of the system
actually understands.

Some Rules:

1/ First of all make sure you use Capitals for just about everything!

2/ Other First NEVER try to run Assembler without SAVING first - this is the
only warning you will get, the next is a hard lesson in a crashed machine
with your masterpiece in some transistors just waiting for the power off
or RESET that is coming.

Using TAP Files with older software
-----------------------------------

This advice applies to most older software that only supports tape and is
the reason for TAP files support on the NMI button browsers in NextZXOS and
in ESXDOS.  You can use also do this with the older A17C copy of PAW that has
been provided in the distribution as well, although we hope you will start to
use the full ESXDOS version or even better get a copy of the new NextPAWS!

To Save your work you will need to attach a TAP file. Time to practice with
the new NextZXOS NMI menu when you press the Mutiface button on
the left side near the front!  This functionality is also available from
the ESXDOS NMI system either from the standard menu or the Dr Slump improved one
(Press H to cycle through some help pages on ESXDOS)

This allows you to select a different TAP to load your work from and create a
new one for what you save.  This way you will always have a history of the
changes you made just like Cesar did...

PROJECT1.TAP
PROJECT2.TAP
etc

The next time you just select PROJECT2.TAP to Attach to load from and create
a new one PROJECT3.TAP to save into.  Easy to go back and look if you break
something.

You can do several saves in each TAP with a unique name for the text version
as well you will just have to remember its name.

Of course as, at the moment, it is a standard 128K program you can use the
snapshot function to save a .SNA at any time to get you back to exactly where
you were!

Remember it is Capital H for help when you get to the Command Prompt:

A - Assemble
B - Back to Basic
D - Disassembler
E - Editor
F - Modify Date
G n - Set block of text to use
I - General Information
L - Load source code
N - New block of Text
O - Save Object Code
S - Save Source Code
T - View Symbol Table
Z - Delete Source Code

First things first:

E - edit mode
-------------

In here you have a number of keys available after pressing EXTEND mode
CAPS and SYMBOL on a 48K KB.

A - Page down
B - Delete Block
C - Copy Block
D - Search for Label
E - " from start
F - Search Text
G - " from start
N - Find Next text
H - HELP!!! (not listed on help - you can sort of see the logic)
I - Insert ON/OFF
J - Next Label
K - Calculate Expression
L - Goto line
P - Mark start of Block
Q - Page up
U - Mark end of Block
V - Paste Block
X - Cut Block
Z - Restore Line

While holding Symbol Shift:

D Delete line
I Insert line
Q Exit editor

Try it just type in two lines (you can include the comments if you wish ;)

;Sample program make A=200
 LD A,200
 RET

When you have finished your masterpiece then:

Q Exit editor (just go back to the command prompt)

A - Assembler
-------------

Now the above text is just like that, some text - unlike BASIC where you would
type RUN to go through the code, as this will be given to the main CPU when
we finally run it, we need to turn it from text into a list of the numbers.
You will find these next to the character set in the Spectrum manuals.

For the above that is:

3E C8		; 3E / 62 means LD A and C8 = 200 in Hexadecimal
C9		; C9 / 201 is a return instruction

So press A now and SPED will do its job and convert the text into the numbers
for us.

If there are any errors they will be shown, press ENTER to move onto the next
one.  Make a note of the line number as you can use EXTEND+L to GOTO it in
the editor!

How do I RUN it then?
---------------------

Remember the rules - make sure you have Attached a TAP file for save and then
at the command prompt type:

S FIRST.SPE			- or whatever you want to call the file.

This is just the text - you will probably also want to save the object code
(the list of numbers SPED just worked hard to produce) 

O FIRST.BIN			- BIN is usually meant for blocks of code

In order to run your code you would need to go back to BASIC and load in the
object code you have saved from the tap.  Then you will be able to use
the Debug tools in the NextZXOS MF to look at memory - this is a whole other
topic which will be covered elsewhere.

Or like a real grown up IDE you can use the built in tools for SPED53 which
includes what is called a 'Monitor/Dissasembler' to allow you to see what is
happening to the regsisters and memory and turn the numbers back into a
human form.

The Dissasmbler Monitor
-----------------------

Type D at the Command Prompt

There is quite a bit to learn here but the screen is showing you all the
contents of the Z80 and some of the memory at the locations that the various
registers (like variables) it has.  Can you spot the A register?

EDIT/ENTER ME-+1
cursor left and right +-8 bytes
Up/Down -+ Instruction

C Calculate Expression
$ Disassemble
SS+D Read Debug Table Yes/No
E Assemble instruction
I Insert Bytes
SS+K Execute
L List Hex and Ascii
M Change point in Memory
SS+N Make PC = ME
SS+Q Quit disassembler / monitor
R Modify register RR=XXXX
S See screen
SS+S Configure screen destination
(0:None 1:Symbols 2:Debug)
SS+T Set Breakpoint after instruction
SS+Z Step by Step
SS+3 Numbers Hex or Dec (i.e. #)

Symbol Shift and Q to quit dissassembly listing which should show you the 
the numbers converted back into Source code....

You use SS + Q then as well to go back to the Command prompt

SS+Z will allow you to step over each instruction and see what happens - do
that now and you should see the value of A change.  Well done!

List of allowed Directives
--------------------------

DEFS		- Define an amount of space to leave
DEFM		- Define a string
DEFB		- Define a byte
DEFW		- Define a word (2 Bytes)
ENT		- Entry address for code
LST+		- listing on and off
LST-
ORG		- Where the assembled code should be put to run it
DPR+		- see note below
DPR-

DPR means: DePuRacion (debugging in Spanish)
The DPR+ and DPR- will allow you to see the source code while debugging.
Just enable it at the beginning of source code and after compile, go to the
debugger where you can use SS+D to read the debug data to allow you to see the
source as you run.

On the subject of Blocks
------------------------

The editor uses the idea of Blocks of Text held on multiple banked memory pages.
The block in current use is shown in the top right. Using G as a command
G 0,1,2 (and eventually 3-xxx) will allow you to go to that block if it has
been started.

To see this you can load the source code for SPED53 itself as it uses 2 and a
bit blocks...

Source Code
-----------
Attach the SPE53AEN.TAP for English or SPE53AES.TAP for Spanish which will be in
the sources folder.  Just use L to load the first set of blocks!

You can then go to each block and use E to edit them and look through the
source code.  'A' will of course Assemble this, which will be a bit slower
that the above example.

TIP: from the MF menu you can use the browse to look at what is on a TAP to
'rewind' the tape just click on the first block or click on the specific 
file if you want.  Note that there are can be multiple blocks in a SPED source
file.

The name is SPE for source, 52 for the program and A for version with EN/ES for 
language.  So version 1.2 will be B etc.  You can of course use either the
Spanish or the English to assemble each others code.

Using the assembled Object code
-------------------------------

You can get at the file you have made from in the TAP using standard basic
commands:

NextZXOS

CLEAR 49151
.tapein PROJECT1.TAP
LOAD "t:"
LOAD "name" CODE 49152
.tapein -c

ESXDOS

CLEAR 49151
.tapein PROJECT1.TAP
LOAD "name" CODE 49152
.tapein -c

Your assembly language is now in memory - so you can save it out as a block
with SAVE "name" CODE 49152,length on NextZXOS or SAVE *"name" CODE 49152,length
on ESXDOS

TODO
----

The following things may get addressed in future versions:

Line 1567 end of block 1 is the instruction tables can we add NEXTREG?

Page 7 is used making return to NextZXOS other than 48K a problem

It could make use of more pages on the Next, but that will stop SNA working.

Add native file system support for saving blocks without using TAP's

For info
--------

The original used two BASIC programs to load; the first asked you to press
a key and then LOAD "" causing a restart by doing:

RANDOMIZE USR 0 (to go to 48k mode but having 128k of memory).

Then the second which was next on the TAP did:

CLEAR 49151: OUT 32765,17: LOAD "" CODE 49152: RANDMOIZE USR 49152

We have put the above in a bit of BASIC for you - so you can just use the
Next facility to load from a TAP using the Browser just make sure you use
option 0.

For some more background:

https://github.com/chernandezba/zesarux/blob/master/src/my_soft/spectrum/sped

License
-------

    SPED53 - integrated Assembler/Monitor and Disassembler for the 128K Spectrum
    Copyright (C) 1997 César Hernández Bañó

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.


Tim Gilberts
August 2018


Manifest
--------

tools/

SPED53.BAS   - Loader with language selection (Must use on DivMMC ESXDOS)
SPED53EN.TAP - English version of the Assembler (Can load direct on NextZXOS)
SPED53ES.TAP - Spanish version of the Assembler (Can load direct on NextZXOS)

sources/sped52

SPE53AEN.TAP - English source (and object code) for SPED53 1.2 (A) ENglish
SPE53AES.TAP - Spanish source (and object code) for SPED53 1.2 (A) ESpañol

docs/

readmeSPED53.txt

SPED53 changes
--------------

Fixed bug when saving object code, was saving always from 49152


Release 2 changes
-----------------

Fixed duplicate "para" in Spanish Version text

Added note on Source debugging: DPR means: DePuRacion (debugging in Spanish)

Bug on ESXDOS 0.8.5 on original ZX128 where keyboard fails to work - fixed with
SPED53.BAS loader instead of direct boot of TAP files...


