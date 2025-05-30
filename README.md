ZEsarUX - ZX Second-Emulator And Released for UniX 

Created by Cesar Hernandez Baño

https://github.com/chernandezba/zesarux


It's a ZX Machines Emulator for UNIX based operating systems (and Windows), including all the Sinclair computers:

* MK14
* ZX80
* ZX81
* ZX Spectrum
* QL
* Z88

And also:

* Microdigital TK80, TK82, TK82C, TK83, TK85, TK90X (Portuguese and Spanish), TK95 (Portuguese and Spanish)
* Czerweny CZ 1000, CZ 1500, CZ 2000, CZ 1000 Plus, CZ 1500 Plus, CZ Spectrum, CZ Spectrum Plus
* Timex Sinclair 1000, 1500, 2068
* Timex Computer 2048, 2068
* Inves Spectrum +
* Sam Coupe
* Pentagon 
* Chloe 140 SE, 280 SE
* Chrome
* Prism
* ZX-Uno
* ZX-Evolution BaseConf (beta)
* ZX-Evolution TS-Conf
* ZX Spectrum Next
* Jupiter Ace
* Amstrad CPC 464, CPC 4128, CPC 664, CPC 6128
* Amstrad PCW 8256, PCW 8512
* MSX1 
* Spectravideo 318/328
* Colecovision
* Sega SG1000
* Sega Master System

ZEsarUX source code and binaries are distributed under GNU GPL license. 
ZEsarUX also includes a folder, "my_soft", which has some programs and data made by me. The entire folder is also covered by the GNU GPL license.

ZEsarUX also includes third-party roms, media, programs and games NOT covered by this license.

This is my second ZX Spectrum emulator after ZXSpectr
https://github.com/chernandezba/zxspectr

I recommend you to read FEATURES, INSTALL and HISTORY files, as well as other documents in this emulator.
You can open them from the help menu or from an external viewer.


ZEsarUX distributed under GNU GENERAL PUBLIC LICENSE v3. You may read it on the LICENSE file.

Please read the other licenses used in ZEsarUX, from the menu Help->Licenses or just open files from folder licenses/


Available releases for [download](https://github.com/chernandezba/zesarux/releases):
* Source code
* Binary compiled versions:
* GNU/Linux 32/64 Bits
* FreeBSD 64 Bits
* Haiku OS
* Mac OS X
* Windows
* Raspberry pi (raspbian)

Other by 3rd party:
* Arch Linux
* Slackware
* Retropie/EmulationStation
* Open Pandora
* PocketCHIP
* MorhpOS

Also an experimental [Docker image](https://hub.docker.com/r/chernandezba/zesarux) 

ZEsarUX has won the "Best Emulator" award from Retrogaming Total blog on 2015 and 2017




__DONATE__

ZEsarUX is free software and you don't need to pay to use it. 
ZEsarUX will always cost you nothing to use, but that doesn't mean it costs me nothing to make.
So if you want to demonstrate your appreciation to it and also help me cover the costs of my development servers, you can donate using Paypal.
All donors will appear in the DONORS file.

Just click:

[ZEsarUX donation](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=E5RSRST8N7KWS&lc=ES&item_name=Donativo%20ZEsarUX&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted)


Thanks!


__FEATURES__

* Runs on any UNIX based operating system and Windows: tested on GNU/Linux x86, GNU/Linux x86_64, GNU/Linux Raspbian, GNU/Linux RetroPie, FreeBSD x86_64, Haiku, Mac OS X, Windows native, Windows + Cygwin

* Emulates:
    * Science of Cambridge MK14
    * Sinclair ZX80
    * Sinclair ZX81
    * Sinclair ZX Spectrum models: 16k, 48k (English and Spanish), 48k+ (English and Spanish), Spectrum 128k (English and Spanish), Spectrum +2 (English, Spanish and French), Spectrum +2A (English and Spanish), Spectrum +3 (English and Spanish)
    * Sinclair QL
    * Cambridge Z88
    * Microdigital TK80, TK82, TK82C, TK83, TK85, TK90X (Portuguese and Spanish), TK95 (Portuguese and Spanish)
    * Czerweny CZ 1000, CZ 1500, CZ 1000 Plus, CZ 1500 Plus, CZ 2000, CZ Spectrum, CZ Spectrum Plus
    * Timex Sinclair 1000, 1500, 2068
    * Timex Computer 2048, 2068
    * Inves Spectrum +
    * Sam Coupe
    * Pentagon
    * Chloe 140 SE, 280 SE
    * Chrome
    * Prism
    * ZX-Uno
    * ZX-Evolution BaseConf (beta)
    * ZX-Evolution TS-Conf
    * ZX Spectrum Next
    * Jupiter Ace
    * Amstrad CPC 464, CPC 4128, CPC 664, CPC 6128
    * Amstrad PCW 8256, PCW 8512
    * MSX1
    * Spectravideo 318/328
    * Colecovision
    * Sega SG1000
    * Sega Master System

* Is the only emulator for machines: Chloe 140/280, Prism and Chrome

* Almost perfect emulation of timing of all Spectrum machines

* Emulates undocumented Z80 flags, features, and MEMPTR register

* Emulates idle bus port, contended memory, ULA early/late timings

* Emulates Motorola CPU 68008 (Sinclair QL)

* Emulates SC/MP CPU - INS8060 (MK14)

* Emulates perfect ZX Spectrum 16k/48k colour palette

* Emulates ULAplus: Standard 64 colour palette, linear modes with 16 colours per pixel: Radastan Mode: 128x96, ZEsarUX modes: 256x96, 128x192, 256x192

* Emulates Flash Color mode (128 colors)

* Emulates Chroma81 ZX81 Colour mode

* Emulates Spectra Video Modes

* Emulates Amstrad PCW Colour Video Modes

* Emulates Timex Video modes (Mode 0 standard, 1 dual screen, 2 hires colour 8x1, 6 512x192 monochrome)

* Emulates Pentagon 16C mode

* Emulates All Video modes from Prism machine

* Emulates ZGX Sprite Chip

* Emulates snow effect on Spectrum models

* Emulates interlaced, scanlines and Gigascreen effects

* Emulates hi-res modes on ZX Spectrum (rainbow effects and others) and ZX80/ZX81 (UDG, CHR$128, WRX, HRG and some other hi-res modes)

* Supports reducing the screen to 192x144 (0.75 scale)

* Uses its own powerful window environment (ZX Vision) inside the program application window, having the same GUI style on all plattforms. Allows to have an extended display to hold menus and multitask windows in the GUI (ZX Desktop)

* Partial support for Spanish and Catalan language on menus

* Can be used with joystick and environments without keyboard, like Raspiboy / Retropie

* Emulates all the oddities of the Inves Spectrum +: 64 KB RAM, RAM initialization with FF00H pattern, OUT ula AND RAM, EAR and MIC XOR, no contended memory, snow in border, colour ula delay, interrupt starts at end of top border (not at the beginning of the border), corrupt memory on every interrupt, no idle bus

* Uses Video drivers: X-Windows, SDL, Cocoa (Mac OS X), Framebuffer, ncurses, aalib, cacalib, stdout(console), simpletext(console)

* Uses Audio Drivers: PulseAudio, Alsa, SDL, DSP(OSS), One Bit Speaker (PC Speaker and Raspberry Pi GPIO speaker, without any kind of sound card), CoreAudio (Mac OS X).

* Supports dumping audio & video to file

* Allows to save screen to formats: SCR, PBM, BMP, TXT, STL (3D Model)

* Supports loading from real tape (any external audio source, like tape player, mp3 player, etc)

* Supports real tape loading of file formats: RWA, SMP, WAV, TZX, PZX, CDT, TAP, P, P81, O. It handles loading of turbo load tapes or any type of tape loading for standard/non standard loading routines

* Handles binary tape format files (TAP, TZX, PZX, O, P, P81, CDT, CAS) on standard ROM routines for ZX Spectrum, ZX80, ZX81, Amstrad CPC, Jupiter Ace, MSX, Spectravideo

* Handles real audio loading (RWA, SMP, WAV) on standard ROM routines for ZX Spectrum, ZX80, ZX81, Amstrad CPC, Jupiter Ace

* Simulates real tape loading on standard ROM routines for ZX Spectrum, ZX80, ZX81

* Handles snapshot formats: ZSF, ZX, Z80, SP, SPG, NEX, SNA, P, P81, O, Z81, ACE

* Handles microdrive formats: MDR, RMD and MDV

* Handles floppy disk formats: DSK

* Handles MMC/SD card formats: HDF, IMG, RAW (.MMC)

* Handles IDE hard disk formats: RAW (.IDE)

* Handles cartridge formats: DCK, ROM, COL, SG

* Handles RZX playback

* Emulates the following Copy Interfaces: Defcon, Dinamid3, Hilow Barbanegra, Interface007, Microhobby Pokeador Automatico, Multiface (One, 128 and Three), Phoenix, Ramjet, Spec-Mate, Transtape

* Emulates Dinamic SD1

* Emulates Datagear/MB02 DMA, ZX-Uno DMA

* Emulates Nec PD765 floppy disk controller, used on Spectrum +3, CPC 664, CPC 6128, PCW

* Emulates ZX Microdrive on ZX Spectrum and QL: on ZX Spectrum supports MDR (standard file system format) and raw format (extension RMD). On QL is simulated by rom traps, accessing files from your computer and also from MDV (QLAY format)

* Emulates Betadisk/TR-DOS. Direct support for .trd files. scl files can be converted from file selector pressing space

* Emulates ZX Spectrum MMC Interfaces: ZXMMC, ZXMMC+, DivMMC

* Emulates ZX Spectrum IDE Interfaces: DivIDE, 8-bit simple

* Emulates ZX Spectrum ESXDOS file access using files from your computer

* Emulates Sam Coupe IDE Interface: Atom Lite

* Emulates ZXPand MMC Interface on ZX80 and ZX81

* Emulates ZX Dandanator! Mini, CPC Dandanator! Mini

* Emulates Speccy Superupgrade

* Emulates Kartusho, iFrom interfaces by Antonio Villena

* Emulates SamRam interface by Gerton Lunter (author of famous Z80 emulator)

* Emulates LEC Memory extension

* Emulates HiLow DataDrive

* Handles RAM, EPROM, Intel Flash and Hybrid (RAM+EPROM) cards on Z88

* Handles QL microdrive/floppy file access using files from your computer. Allows reading Q-emuLator file headers. Allows EXECuting files without headers

* Handles compressed formats zip (with internal decompressor), and tar, rar, gz through external utilities

* Emulates AY Audio Chip, Turbosound (2 AY Channels), 3 AY Channels, MIDI channels, different DAC: Specdrum, Covox, ACB/ABC/BAC/CBA Stereo

* Emulates General Sound

* Emulates SN76489AN Audio Chip

* Emulates Quicksilva, ZON-X81, and VSYNC-based sound on ZX80/81

* Emulates Speaker and Soundbox sound on Jupiter Ace

* Experimental simulation of the Sam Coupe Audio Chip (SAA1099)

* Emulates i8049 QL sound

* Supports RAM size up to 1024 KB on Spectrum and Pentagon

* Supports RAM packs on ZX80/81 up to 56 KB

* Supports RAM packs on Jupiter Ace up to 51 KB

* Joystick emulation with real joystick and keyboard cursors: Kempston, Sinclair 1&2, Cursor Joystick, Cursor & Shift, OPQA, Fuller, Zebra, Mikro-Gen, ZXPand, SAM Coupe Cursors, CPC, MSX, Spectravideo, Cascade, DKTronics. Autofire function too

* On Screen keyboard useful when playing with joystick, two types: one with keyboard letters, and another with words, useful for playing Text Adventures. Also a tool to extract words from text adventures (Daad, Paws, Quill and Gac)

* Emulates Spectrum keyboard ghosting error feature

* ZX Spectrum Recreated keyboard support

* Emulates native turbo modes on ZX-Uno, Chloe, Prism and ZX Spectrum Next, and manual for other machines

* ZX Printer emulation

* Lightgun emulation: Almost perfect emulation of Gunstick from MHT Ingenieros S.L and experimental emulation of Magnum Light Phaser

* Kempston mouse emulation

* Supports Network gaming using its own protocol (ZEsarUX Network Gaming protocol - ZENG), which allows you to play in two different ways: using two or more (up to 16) ZEsarUX instances, located each one on any part of the world or in a local network, or with a central server (ZENG Online). Games doesn't have to be modified, you can use any existing game

* Uart bridge emulation: allows you to use a real uart-wifi device connected to your computer, on GNU/Linux only (Windows, Mac can simulate it)

* Can browse online ZX81 games

* Can browse online Spectrum games

* Supports Input spool text file to simulate keyboard press

* Supports reading Pokes from .POK files

* Audio Chip Tools: For the 3 emulated audio chips (AY-3-8912, SN76489AN, QL i8049): Sheet, Piano, Registers, export music to midi (.mid) files

* Real Time playback from Audio Chip Sound (AY-3-8912, SN76489AN, QL i8049) to external MIDI device

* AY Player: allowing to play music from .AY files

* AY Mixer

* Visual Real Tape: you have an audio render of your tape, see tape blocks and rewind or move forward the cassette player

* Visual Casette Tape: allow to see a real tape and the movement when loading a game

* Visual Floppy: allow to see floppy activity: disk movement, reads, writes, head seek, etc

* Visual Microdrive: allow to see microdrive activity

* Supports ZEsarUX remote command protocol (ZRCP). This is a powerful communications protocol between a client and ZEsarUX, using a simple telnet client. One of the things you can do it is enhanced debugging on ZEsarUX from Visual Studio Code (see https://github.com/maziac/DeZog)

* Can use a reduced Spectrum core, with some features disabled, useful on slow devices, like Raspberry Pi 1/Zero

* Can generate Automatic Snapshots to RAM and also do a Rewind operation

* Powerful debug features: Reverse Debugging, CPU History, Assembler, Registers, Dissassemble, Conditional Breakpoints using text expressions, Watches, Step-to-step, Step-over, Runto, Show TV electron position, Load source code, Hexadecimal Editor, View Sprites, View Tiles, Find bytes, Infinite lives finder, CPU Transaction log, View BASIC, View BASIC variables, verbose messages on console

* Text adventure debugger: On a Quill/Paws/Daad/Gac text adventure you can Step to Step condact, watch flags/objects, list messages (objects, user/system messages, locations, compressed tokens, vocabulary), connections, Text Adventure Map. Can also view graphics from a Quill, Paws, Daad or Gac aventure.

* Use artificial intelligence to get realistic images for location description on Text Adventure games

* File utilities menu: Allowing to view, expand and convert some common file formats: tap, tzx, pzx, trd, dsk, mdv, hdf, ddh, etc. Can also browse inside file system images (.mmc, .img, etc)

* Accessibility support: Print char traps allows to capture generated text from almost any program or game, using standard ROM calls (RST 10H) or even non standard print character routines. Can send generated text from a game to a text-to-speech program. It's ready for text to speech support for blind or visually impaired people. Menu emulator can also be read by a text-to-speech program.

* Translation support: Can translate text from almost any program or game using external scripts

* Memory Cheat: useful to find counters of energy, bombs, ammo or any other cheat in a game

* Simulates upper RAM memory refresh on Spectrum 48Kb, losing its contents when changing R register very quickly

* Supports command line settings, configuration file settings and per-game configuration settings

* Supports execution on Docker, tested on Debian, Ubuntu and Fedora containers

* Other features: Visualmem menu, CPU Statistics, Toy ZXeyes, Toy ZXlife, Ascii Table, Sensors

* Includes seven easter eggs. Can you find them? :)



__Some screenshots__

ZX Desktop, running multitask windows, on Solarized Dark GUI Style, running ZX Spectrum OverScan demo

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_zxdesktop_multitask.png "ZX Desktop")


ZEsarUX 11.0, running multitask windows, ZEsarUX Plus GUI Style, running Sound Tracker 20th anniversary demo, some Multitask windows

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_zesarux_11_0.png "ZEsarUX 11.0")


Default clean ZX Desktop starting from ZEsarUX version 10.2

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_zxdesktop_default.png "ZX Desktop default")


ZX Desktop, running demo ny17 from TSConf, showing some windows opened

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_zxdesktop_example_windows.png "ZX Desktop example several windows")


ZX Spectrum Overscan demo

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_overscan.jpg "Overscan demo")


ZX-81 Mazogs

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_mazogs.png "Mazogs")


Sinclair QL

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_ql.png "QL")


Cambridge Z88

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_z88.png "Z88")


ZX Spectrum Sir Fred running on curses (text) driver

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_sirfred_curses.png "Sirfred curses")


ZX Spectrum The Great Escape running on curses (text) driver + utf8 extensions

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_greatescape_curses.png "The Great Escape curses")


ZX81 Mazogs running on curses (text) driver + utf8 extensions

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_mazogs_curses.png "Mazogs curses")


ZX-Uno

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_zxuno.png "ZX-Uno")


ZX-Evolution TSConf

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_tsconf.jpeg "ZX-Evolution TSConf")


ZX Spectrum Next

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_tbblue.png "TBBlue/ZX Spectrum Next")


Prism 512

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_prism.png "Prism 512")


Chloe 280SE

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_chloe280se.png "Chloe 280SE")


CPC 464

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_cpc.png "CPC 464")


MSX

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_msx.png "MSX")


Colecovision

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_coleco.png "Colecovision")


Sega SG-1000

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_sg1000.png "Sega SG-1000")


Sega Master System

![alt text](https://github.com/chernandezba/zesarux/raw/main/screenshots/screenshot_sms.png "Sega Master System")


Do you want to know how ZEsarUX looked like in the past? See this:
[PREVIOUS_SCREENSHOTS](https://github.com/chernandezba/zesarux/blob/master/PREVIOUS_SCREENSHOTS.md)


You can find some ZEsarUX videos on my [Youtube channel](https://www.youtube.com/user/chernandezba)
