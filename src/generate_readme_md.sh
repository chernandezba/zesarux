#!/usr/bin/env bash

URLIMAGES_BASE="https://github.com/chernandezba/zesarux/raw/main"

cat README
echo

cat << _EOF

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

Also tested on:
* AROS Research Operating System (Amiga Compatible)

Also an experimental [Docker image](https://hub.docker.com/r/chernandezba/zesarux) 

ZEsarUX has won the "Best Emulator" award from Retrogaming Total blog on 2015 and 2017


_EOF


echo
echo
echo "__DONATE__"
echo
cat DONATE

cat << _EOF

Just click:

[ZEsarUX donation](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=E5RSRST8N7KWS&lc=ES&item_name=Donativo%20ZEsarUX&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted)


Thanks!
_EOF

echo
echo
echo "__FEATURES__"
echo
cat FEATURES

echo
echo
cat << _EOF
__Some screenshots__

ZX Desktop, running multitask windows, on Solarized Dark GUI Style, running ZX Spectrum OverScan demo

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_zxdesktop_multitask.png "ZX Desktop")


ZEsarUX 11.0, running multitask windows, ZEsarUX Plus GUI Style, running Sound Tracker 20th anniversary demo, some Multitask windows

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_zesarux_11_0.png "ZEsarUX 11.0")


Default clean ZX Desktop starting from ZEsarUX version 10.2

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_zxdesktop_default.png "ZX Desktop default")


ZX Desktop, running demo ny17 from TSConf, showing some windows opened

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_zxdesktop_example_windows.png "ZX Desktop example several windows")


ZX Spectrum Overscan demo

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_overscan.jpg "Overscan demo")


ZX-81 Mazogs

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_mazogs.png "Mazogs")


Sinclair QL

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_ql.png "QL")


Cambridge Z88

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_z88.png "Z88")


ZX Spectrum Sir Fred running on curses (text) driver

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_sirfred_curses.png "Sirfred curses")


ZX Spectrum The Great Escape running on curses (text) driver + utf8 extensions

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_greatescape_curses.png "The Great Escape curses")


ZX81 Mazogs running on curses (text) driver + utf8 extensions

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_mazogs_curses.png "Mazogs curses")


ZX-Uno

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_zxuno.png "ZX-Uno")


ZX-Evolution TSConf

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_tsconf.jpeg "ZX-Evolution TSConf")


ZX Spectrum Next

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_tbblue.png "TBBlue/ZX Spectrum Next")


Prism 512

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_prism.png "Prism 512")


Chloe 280SE

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_chloe280se.png "Chloe 280SE")


CPC 464

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_cpc.png "CPC 464")


MSX

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_msx.png "MSX")


Colecovision

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_coleco.png "Colecovision")


Sega SG-1000

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_sg1000.png "Sega SG-1000")


Sega Master System

![alt text](${URLIMAGES_BASE}/screenshots/screenshot_sms.png "Sega Master System")


Do you want to know how ZEsarUX looked like in the past? See this:
[PREVIOUS_SCREENSHOTS](https://github.com/chernandezba/zesarux/blob/master/PREVIOUS_SCREENSHOTS.md)


You can find some ZEsarUX videos on my [Youtube channel](https://www.youtube.com/user/chernandezba)
_EOF
