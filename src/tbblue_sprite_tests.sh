#!/usr/bin/env bash

for program in SprDelay.sna SpritRel.sna SprBig4b.sna SpritBig.sna; do

./zesarux --noconfigfile --allow-background-windows-closed-menu --allow-background-windows --enable-zxdesktop --zxdesktop-width 1024 --windowgeometry tsconftbbluespritenav 55 8 43 25 --enable-restore-windows --restorewindow "tsconftbbluespritenav"  --quickexit --nowelcomemessage --disablebetawarning 9.2-SN --zoom 1 --machine tbblue --realvideo --enabletimexvideo --tbblue-fast-boot-mode --sna-no-change-machine /Users/cesarhernandez/Downloads/ZXSpectrumNextTests/release/$program

done



