1 GO SUB 31:GO SUB 30:LET a=1:LET co=0:LET li=6:MODE 8:WINDOW 400,224,56,16:CSIZE 2,0:BORDER 1,7:PAPER 7:INK 0:RESTORE 1600:CLS:PRINT "(c) Cesar Hernandez Bano (24/8/94)":PRINT "Canciones recopiladas de la":PRINT "revista MICROHOBBY":PRINT "--------------------------------":AT #0,0,0:PRINT #0;"ELIGE CANCION CON CURSORES Y"\"{ ENTER }"
2 zxmenu
3 LET VO=0:GO TO 10
4 zxselect:RUN
5 IF a=16 THEN LET li=6:LET co=16
6 GO TO 11
7 IF a=15 THEN LET li=20:LET co=0
8 GO TO 11
9 GO SUB 31:CLEAR:SAVE "mdv1_CANCIONES_QL_QSOUND":GO TO 9
10 LET vez=0
11 GO SUB 12:GO TO 22
12 PAPER 4:INK 7:AT li,co:PRINT "               ";:RETURN
13 PAPER 7:INK 0:AT li,co:PRINT "               ";:RETURN
15 LET c=zxkey:IF c=10 AND a<26 THEN GO SUB 13:LET a=a+1:LET li=li+1:GO TO 5
16 IF c=13 THEN GO TO 24
17 IF c=11 AND a>1 THEN GO SUB 13:LET a=a-1:LET li=li-1:GO TO 7
18 IF c=8 AND co THEN GO SUB 13:LET co=0:LET a=a-15:GO TO 11
19 IF c=9 AND NOT co AND a<12 THEN GO SUB 13:LET co=16:LET a=a+15:GO TO 11
20 IF vo THEN RETURN
21 GO TO 15
22 IF vez=1 AND vo THEN RETURN
23 LET vez=1:GO TO 15
24 IF vo AND a<n THEN GO SUB 13:GO TO 4
25 IF NOT vo THEN GO SUB 13:GO TO 4
26 RETURN
30 LET li=33:GO TO 32
31 LET li=34:GO TO 32
32 RESTORE li:zxregs:RETURN
33 DATA 100,0,0,0,0,0,11,119,16,0,0,0,40,8
34 DATA 0,0,0,0,0,0,14,255,0,0,0,255,255,0
35 LET M$=T$:LET Y=INT(RND*5)+7:GO SUB 37:IF A$="" THEN RETURN
36 AT 17,0:PRINT "Autor:":LET Y=19:LET M$=A$:GO TO 37
37 zxcentre:GO TO 66
38 zxanim1:RETURN
39 zxanim2:RETURN
40 zxanim3:RETURN
41 zxanim4:RETURN
42 zxanim5:RETURN
43 zxanim6:RETURN
66 LET E=INT(RND*6):GO TO 38+E
67 LET E=INT(RND*5):GO TO 38+E
68 LET a$="T160O2V12 5A(((5DDDDD)DA))(((DDDDD)DA)(aaaaa)aA(DDDDD)DA)((bbbbb)bb(#f#f#f#f#f)#fA)(ggggg)gA(DDDDD)DA(aaaaa)aA(DDDDD)"
69 LET b$="UX10000W0O5N9&&&&&&&&&&&8&((4g1#f4e1d9a&7&4g1#f4g1a8g#f9&))(4g1#f4e1d9a&7&4g1#f4g1a8g#f9&)4g1#f4#e1d9a&7&4g1#f4g1a8g#f9&4g1#f4e1d9a&7&4g1#f4e1#f8ed9&(4D1#C4b1#C9D&7&4#C1b4a1b8a&9&)4b1a4g1a9b&7&4a1g4#f1g9a&7&4g1#f4e1d9a&7&4g1#f4e1#f8ed9&"
70 LET c$="UO4N"&b$(12 TO )
80 LET d$="T160O2V12 5DA(((DDDDD)DA)(aaaaa)aA(DDDDD)DA)(((bbbbb)bB(#F#F#F#F#F)#F#C)(GGGGG)Ga(DDDDD)DA(aaaaa)aA(DDDDD)DA)((aaaaa)aA(DDDDD)DA)9_9DV7N4DV5DV3DV1D"
90 LET e$="UX10000W0O4(4g1#f4e1d9a&7&4g1#f4g1a8g#f9&)4g1#f4e1d9a&7&4g1#f4g1a8g#f9&4g1#f4e1d9a&7&4g1#f4e1#f8ed9&((4D1#C4b1#C9D&7&4#C1b4a1b8a&9&)4b1a4g1a9b&7&4a1g4#f1g9a&7&4g1#f4e1d9a&7&4g1#f4e1#f8ed9&)(4g1#f4e1d9a&7&4g1#f4e1#f8ed9&)4g1#f4e1dV11N9_9dV7N4dV5dV3dV1d"
100 LET f$="7&O3V11N7_9_9#F7_9E9_9A7_9_9E7_9#F9_9A7_9_9#F7_9G9_9A7_9_9E7_9G9#FE(9_9_9#F9_9_9#C)9_9_9D9_9_9A9_9_9E9_9_9#FO4N7b#fO5N7b7_9_9_9#F7#Cab#fb7_9_9_9#F7#C#fO3N5_9B5D7_9B5_9A5a7_9A(5_9E5e7_9E5_9A5a7_9A)5_9E5e7_9E5_9A5a7_9A9_9AV9N4AV7AV6AV4AV2A"
110 zxplay3 a$,b$,c$
120 zxplay3 d$,e$,f$:RETURN
140 LET a$="O5V14T140N6#D5#C3#ab5#C3#ab5#C3b#a5#gT145N3#D#D4#C1b3#ab5#C3#ab5#C3b#a5#g"
150 LET b$="O5V14T140N6#g(5#a3g#g)5#a3#gg5#gT145N3bb4#a1#g3g#g5#a3g#g5#a3#gg5#g"
160 LET c$="O3V13T140N6#g(3#D#A#D#A)3#D#A#D#A#GBT145b#G#D#A#D#A(3#D#A#D#A)#GB"
170 LET d$="O5V14T150N5#D5#C3#ab5#C3#ab5#C3b#a5#gT155N3#D#D4#C1b3#ab5#C3#ab5#C3b#a5&6#GN7#F"
180 LET e$="O5V14T150N5#g(5#a3g#g)5#a3#gg5#gT155N3bb4#a1#g3g#g5#a3g#g5#a3#gg5#g6&V15N6#CV13N3E"
190 LET f$="O3V13T150N5#g(5#D3#D#A)5#D3#D#A#GBT155#gb(#D#A)5#D3#D#A3#D#A#D#D5#G6&7#f"
200 LET g$="O5V14T145N3#D#F3#D1#D#C5b#f3#D#FE1#D#C5b#f#g3#g#a#Cb#a#g5#f#f7#f7#F"
210 LET h$="O5V14T145N5b#C3b#D#C1b#a5#f1#c$$c$c#c5#d$c7#e#g#f9#f"
220 LET i$="O3V14T145N5b3#f1#g#a5b#D3b1#Cb#a#f#g#a5b#D7E#C5#f#g9a"
230 LET j$="O5V14T160N(3#D#F#C#D5b#f)5#g3#g#a#cb#a#g5#FE7#D"
240 LET k$="O5V14T160N(3b#D#a#a5#f#d)5e3ee5#g#g#a#a7#a"
250 LET l$="O3V14T160N(5b3#F#F1B#A#G#A5#F)5E3EE5#C#C#C#C7#D"
260 LET m$="O5V14T150N5#D5#C3#ab5#C3#ab5#C3b#a5#gT155N3#D6#D4#C1b3#ab5#C3#ab5#C3b#a5#gV15N3#G&H"
270 LET n$="O5V14T150N5#g(5#a3g#g)5#a3#gg5#gT155N3bb4#a1#g3g#g5#a3g#g5#a3#gg5#gV15N3#b&H"
280 LET o$="O3V13T150N5#g(5#D3#D#A)5#D3#D#A#GBT155#gb(#D#A)5#D3#D#A3#D#A#D#D5#gV15N3#G&H"
290 zxplay3 a$,b$,c$:zxplay3 d$,e$,f$:zxplay3 g$,h$,i$
300 zxplay3 j$,k$,l$:zxplay3 a$,b$,c$:zxplay3 m$,n$,o$
310 RETURN
330 AT 0,0:PRINT "Procesando Datos"
340 DATA "3e","3c","3e","3c","7g","7e","3e","3c","3e","3c","7g","7e","3e","3c","3g","3e","5C","5a","6b","6g","3a","3f","5a","5f","5g","5e","3d","3$c","3e","3c","5f","5d","5d","5$c","3d","3$c","3e","3c","7f","7d","3d","3$c","3f","3d","3b","3g","3a","3f","5g","5e","5b","5g","7C","7e"
350 LET m$="":LET j$=""
360 RESTORE 340:LET m$="":LET j$="":FOR zn=1 TO 27:zxreadpair
370 LET a$="T80"
380 AT 0,0:PRINT "                                ";
390 zxplay3 a$,m$,j$
400 RETURN
420 LET a$="O3T120V13N3aECEaECE(aFDFaECE)a#GD#GaECE(A#BE#B)A###BF###BA#BE#BA#BGBFAEGE#GBEAEA&"
430 LET b$="O5T120V15N6e3a6C3a6#g1ab7a6f1ga7e1dcc$c4$c1e7$$$c6E1A#BO6N6E3C6$C1CD7C1fgafefgedefdcdecdcc$c4$c1e5$$$c3a&"
440 LET c$="O6T110ON5EE6F3E&5D1#CDED#CE3D&5DD6E3D&5C1bCDCbD3C&5bb3D5Cba1#gaba#gb5a3e&4&O5N1e3#f&#g&a5a1#GABA#GB3A&"
450 LET d$="O3T105V13N(3#CAEA)DAFADACA($CGDG)(CGEG)(#GO4EbE)(fDaD)e&4&1e3#f&#g&aO3N5a3&5A3#B&"
460 LET e$="O3V13N3aE)"
470 LET f$="O5T140V15N3AEEEED#CD5EE3AEEEED#Cb5aa3AEEEED#CD5EE3AEEEED#Cb5aaH"
480 LET g$="O5N9&&&3#C#C#C#C#C&&&5#C#C3#C#C#C#C9&H"
490 LET h$="O5T135V15N3#C#CDD#C#Cbb#C#CDDEA5E3#C#CDDEEEEED#CbaAa&H"
500 LET i$="O5T100V15N3#F&E&5A&"
510 LET j$="O5T100V15N3#F&E&5A&"
520 LET k$="O3T100V15N3D&E&5a&"
530 zxplay2 a$,b$:zxplay2 c$,d$:zxplay3 e$,f$,g$:zxplay2 e$,h$:zxplay2 c$,d$:zxplay3 i$,j$,k$
540 RETURN
560 LET a$="O5(3f&4$a1_1ff3$bf$ef&4C1_1ff3$DC$afCF1f1_1$e$e3cg3_5f7&)"
570 LET b$="O3(3f&4F1_1$e$E3cC$ef&F&1&c3$EF$d&4$D1_1$e$E3c$ef5F&1&$E3C$b$a)"
580 LET c$="O3(3f4F1_1$e$E3cC$ef&4F1&&c3C$EF$d&4$D1_1c$E3c$ef5F&1&$E3C$b$a)"
590 LET d$="O5(3&CC1C$E&$E3$E#$D#$D$#DC1CC$E&$E3#$DC&$a$a$a1$a3$b1$b&3$b1$b3$bCCC1$b3C1C&C3&)"
600 LET e$="O5(3&$#a$#a1$#a$b&$b3$b$b$b$b$#a1$#a$#a$b&$b3$b$#a&fff1f3g1g&3g1g3g$#a$#a$#a1g3$#a1$#a&$#a3&)"
610 LET f$="O5(3&ff1fg&g3gffff1ffg&g3ff&#c$d$d1$d3$e1$e&3$e1$e3$efff1$e3f1f&f3&)"
620 LET g$="V10NO5N9_9_3#$b&7&"
630 LET h$="V10NO5N9_9_3#$e&7&"
640 LET i$="O3(3f&4F1$e$e$E3cC$e)"
650 LET j$="O3N3f&5&7&":LET k$=i$&j$
660 LET l$="O6N7&&&&6F3_6$E3_6#C#b5$a7f&9&"
670 LET m$="O3N3f&4F1_1$e$E3cC$ef&F&1&$E3C$b$af&4F1_1$e$E3cC$ef&F&1&c3C$EF$d&4$D1_1$e$E3c$ef5F&1&$E3C$b$a"
680 LET n$="O3N3f4F1_1$e$E3cC$ef&4F1&&c3C$EF$d&4$D1_1c$E3c$ef5F&1&$E3C$b$a3f4F1_1$E3cC$ef&4F1&&c3C$EF$d&4$D1_1c$E3c$ef5F&1&f3Ffg"
690 LET o$="O5N4$a1_3$e3_1$a4$e3$a$e4$a1_3$e3_1$a4$e1$a$a3$e4$a1_3$e3_1$a4$e3$a$e4$a1_3$e3_1$a4$e3$a$e4$a1_3$E3_1$a4$E3$a$E4$a1_3$E3_1$a4$E1$a$a3$E4$a1_3$E3_1$a4$E3$a$E4$a1_3$E3_1$a4$E3$a$E"
700 LET p$="O5N9&&&&4$A1_3&3_1$A4&3$A&4$A1_3&3_1$A4&1$A$A3&4$A1_3&3_1$A4&3$A&4$A1_3&3_1$A4&3$A&"
710 LET q$="O3N5$a3$A3_1&4&3&&5$e3F3_1&4&1&&3&5$g3$G3_1&4&3&&5$e3$E3_1&4&3&&5$a3$A3_1&4&3&&5f3$A3_1&4&1&&3&5$g3$A3_1&4&3&&5$e3$A3_1&4&3&&"
720 LET r$="O5(1$EC$b1_1$a3$b1C3$EC$b$a)"
730 LET r$=r$&r$&r$&r$
740 LET s$="O3(4$a1_7$A5$a4f1_7F5f4$g1_7$G5$g4$e1_7$E5$e)"
750 LET t$="O5N3&$a$a$a1$a3$b1$b&3$b1$b3$bCCC1$b3C1C&C3&"
760 LET u$="O5N3&fff1f3g1g&3g1g3g$#a$#a$#a1g3$#a1$#a&$#a3&"
770 LET v$="O5N3&#c$d$d1$d3$e1$e&3$e1$e3$efff1$e3f1f&f3&"
780 LET x$="O5N5F"
790 LET y$="O5N5f"
800 LET z$="O3N5f"
810 zxplay1 a$:zxplay1 b$:zxplay2 a$,c$:zxplay3 d$,e$,f$:zxplay3 g$,h$,k$:zxplay2 l$,m$:zxplay2 a$,n$:zxplay3 o$,p$,q$:zxplay2 r$,s$:zxplay3 d$,e$,f$:zxplay3 t$,u$,v$:zxplay2 a$,c$:zxplay2 a$,c$:zxplay3 z$,y$,x$
820 RETURN
840 LET a$="O3T100V13N((4D1a3Fa) )4D1a3Fa((4D1$b3G$b))4D1a3#Fa"
850 LET b$="O5N7&5&3D#C1C3C1C3b$ba1aa3#ggf1ef3gfe&D#C1C3C1C3b#b1aa&a3gfe1de3fe5d3D#C"
860 LET c$="O3T100N(4D1a3#Fa)4D1a3#Fa((4D1b3Gb))((4D1a3#Fa))((4D1b3Gb))((4D1a3#Fa))(4D1b3Gb)4D1b3Gb3A&aG#F1&a3Da5d&7d"
870 LET d$="O5T100N1C3C1C3b$ba1aa3#gg#f1e#f3g#fe&D#C1C3C1C3b$ba1aa3g#fe1de3#fed1&a3de4#f1a3#fed1de3#fg1aaaa3bag1&b3e#fg1&b3g#f4e1#f3ga1bbbb3#Cb3a1&a3DE#F1&a3#fe4d1e3#fg1aaaa3D#C4g1b3E#FG1&b3g#f4e1#f3ga1#Cb#ga3#F10E#FE3D&5&#F&7D"
880 zxplay2 a$,b$
890 zxplay2 d$,c$:RETURN
1060 LET c$="4g1E6D3C4e1b5a3g&1d#cdefefgagabDCge6g3&4g1E6D3C4e1C5b3a&1g#fgabCDCbagfga#fg6e3&1de4d1Cba#gaba"
1070 LET a$=c$&"1de4d1bag#fgag3e1aC3Ee1g#fef5a3g&1bC3D1CD3_3E1DCbCDC3bgg&1bC3D1CD3_3$E1DCbCDC3b5gg3g4g1E5_1DCbC4e1b5_1ag#fgd#cdefefgagabDCge5g3&3_4a1F6E3D4g1E5D3C&1d#cdefefgagabDCge3c&&&"
1080 LET d$="O3C&CC&3CC5ggg6C3gab5C&Ca&3aa5bbb3EbEBGE5#FDF"
1090 LET b$=d$&"O3GDbCa5_1DDE#F3G&5$F3E&a&d&5g3G&5F3$E&a&D&5GFDC&CC&3cc5ggg6C3$ba#C5D&DG&3GG5gggC3C&&&"
1100 zxplay2 a$,b$
1110 RETURN
1130 LET t$="T120"
1140 LET a$=t$&"bbbbagg#feegbEEEEDCCbaabCbCb#DCbbagg#fe#f#f#f#fg#f"
1150 LET b$="V14O4((((11&bg))))(11&bg)((11&EC))(11&EC)((11&#F#D))(11&#F#D)((11&bg))(11&bg)((11&Ca))(11&Ca)(3&Ebg)3&Ebg"
1160 LET c$="V14O4((ee&))(ee&)(aa&)(bb&)(ee&)#D#D&bb&O3N5E5&5b5&5g5&5e"
1170 zxplay3 a$,b$,c$:RETURN
1180 REM op.27.N 2
1190 LET t$="T80"
1200 LET a$=t$&"V14O4(((11#g#CE)))(11a#CE)(11aD#F)11#g#b#F11#g#CE11#g#C#D11#fb#D(((11#g#CE))((11#g#D#F))(11#g#CE)(11a#C#F)(11#gbE)(11ab#D)((11#gbE)))"
1210 LET b$="V13O3N9#C#b7a#f#g#g(9#C#g7#C#fbb9E)"
1220 LET c$=t$&"O6N9&&&&(7&5&4#g1#g8#g4#g1#g7#ga#g5#fb9e)"
1230 zxplay3 a$,b$,c$
1240 zxplay3 "O6e","O3E","O4E":RETURN
1260 LET t$="T60"
1270 LET b$=t$&"V14O2N9CFG7FG)"
1280 LET a$="8E5D8C11DE&8D5b6C1DE7D4E1E(5&1&DE3G1ED&4C1C5&&3&1DE4G1G5&1&DE3G1ED&4C1C5&1&DE3G1EDC4G1G)H"
1290 LET c$="V14N9&&&((3&1cc3&1c&)(3&1ff3&1f&)(3&1gg3&1g&)3&1ff3&1f&3&1gg3&1g&)"
1300 zxplay3 a$,b$,c$:RETURN
1320 LET A$="5d5g5b7C3b7b5d5g5b5C6b3a6D4b5&3D3E3#F3G3#g3b3#g3b5a5&3#f3g3#g3a3b3D3b3D5C5&3a3b3C3D3#D3#F3E3#D3E3#F3A3G3#F3G3E3C6b6a3& "
1330 LET b$="3g3#f3e3$e"
1340 LET c$="3d3e3#f3g3a6b5#f6a5e6g5c5#f3#f1&3#f1&6C5g5#f6a5d5g3#f1&3g6b5#f6a5e6g5c5f3#f1&3#f1&6C5g6C5#g6C5a3D3d3e3#f3g3a6b5#f6a5e6g5c5g5#f3#f1&3f1&6C5g6b5#f6a5d5g3#f1&3g1&6b5#f6a5e6g5c5#f3#f1&3#f1&6E5D6E5D5#f3D3#D3E3#F5G5&5&"
1350 zxplay1 a$:zxplay1 b$:zxplay1 a$:zxplay1 c$:RETURN
1370 LET a$="T120UX3000(O5 6G3#F3G3A5$B3$BO6 3D3CO5 3$B3$B3A5$B)(5$B3A3A3G3G5#F3D1E1#F3G3G3G3#F5G)H"
1380 LET c$="2&T240UX3000(O5 6G6G3#F3#F3G3G3A3A5$B5$B3$B3$BO6 3D3D3C3CO5 3$B3$B3$B3$B3A3A5$B5$B)(5$B5$B3A3A3A3G3G3G3G5#F5#F3D3D1E1E1#F1#F3G3G3G3G3G3G3#F3#F5G5G)H"
1390 LET b$="V9UX4000O3W7 3G1G1G3G1G1G3G1G1G)"
1400 FOR n=1 TO 2:zxplay2 a$,b$:zxplay1 c$
1410 NEXT n:RETURN
1420 LET a$="(V1N5Cgf3eg7C3bD5GF7E5DV15N3CbCbCb5CCgg3DCDCDC5Dgg(3bG5DCV12)V14N5b3aba#f8g)":LET f$="(V12N3GbCD5g3FbCD5gEDC1CbCbCb3ab5gV15N4g1#f5ggCgg4g1#f5ggDgg(4E1#B5GF)V14E3DCDb8c)"
1430 LET b$="V12O4N7e5d7c5e7g5$ccegV15N3CbCbCb5Cgg3DCDCDC5Dgg(5gbaV12)V14CDd3g#fgbgf":LET b$=b$&b$(TO LEN(b$)-8)+"N8g"
1440 LET c$="(O4V12(7gO3N5gO4)O3N5CDdGDdO4V15N4g1#f5ggCgg4g1#f5ggDgg(5ced)V14O3N5CGg8C)"
1450 zxplay2 a$&f$,b$&c$:RETURN
1460 LET a$="(3FA5CC3DF5$b$b3aC5fe7e5f3ce5gg3cf5aa11ceg$ba7a5g)3C$E5aa3$bE5gg3aC5#f#f7#f5g3$bE5ff3aC5ff3g$b5ee7e5f(3FA5aa3DF5$b$b3aC5fe7e5f)"
1470 LET b$="(O4N7f5a7$b5$b7C5c5fcO3N5f8CC5CEF#BGC)O4N8#fg5CDdgdO3N5g8EF5$B#BCFCf8A$B7#B5C8D8A$B7#B5CFCc"
1480 zxplay2 a$,b$:RETURN
1490 LET a$="T240V13O6N3afdfefdf)":LET b$="T240(UX10000W0N7aaV12N5ga)(UX10000W0N7$b$bV12N5ga))":LET c$="T240V13O3N9_9_9d9_9_9$e)"
1500 LET p$="(":zxplay1 p$&b$
1510 zxplay2 p$&b$,p$&c$
1520 zxplay3 a$,p$&b$&"H",c$:RETURN
1540 LET t$="T180"
1550 LET a$=t$&"V14O3N5C3&E&EG&5a3&C&CE&5f3&a&aCf5g3&g&gab)"
1560 LET b$="O6N(5&7g5a6b6a3&f5ff3g&e6d5&&&5&7g5a6ba3&f5ff3g&a6a5g7&)H"
1570 LET d$="(5&3ee5&3e&)5&3ff5&3f&5&3gg5&3g&)"
1580 zxplay3 a$,b$,d$:RETURN
1600 DATA 5610,"WHAM!","","WHAM!"
1610 DATA 68,"TO THE UNKNOWN MAN","VANGELIS","UNKNOWN"
1620 DATA 140,"KALINKA,DANZA POPULAR RUSA","","KALINKA"
1630 DATA 420,"DANZA HUNGARA","J.BRAHMS","DANZA HUNGARA"
1640 DATA 330,"CANCION DE CUNA","J.BRAHMS","CANCION DE CUNA"
1650 DATA 560,"AXEL F.","H.FALTERMEYER","AXEL F."
1660 DATA 840,"HABANERA DE CARMEN","G.BIZET","CARMEN"
1680 DATA 1060,"ANDANTE","","ANDANTE"
1690 DATA 1130,"ROMANCE ANONIMO","","ROMANCE ANONIMO"
1700 DATA 1180,"CLARO DE LUNA","L.Van BEETHOVEN","CLARO DE LUNA"
1710 DATA 1260,"WALK OF LIFE","DIRE STRAITS","WALK OF LIFE"
1720 DATA 1320,"VALS DE LAS FLORES","","VALS FLORES"
1730 DATA 1370,"TIRANT LO BLANC","","TIRANT LO BLANC"
1740 DATA 1420,"CANCION 1","","CANCION 1"
1750 DATA 1460,"CANCION 2","","CANCION 2"
1760 DATA 1490,"CANCION 3","","CANCION 3"
1770 DATA 1540,"PEQUE\ECOS","","PEQUE\ECOS"
1780 DATA 2020,"CHARIOTS OF FIRE","VANGELIS","CHARIOTS OF F."
1785 DATA 4410,"INDUSTRIAL REVOLUTION P.1","JEAN-MICHEL JARRE","INDUSTRIAL 1"
1790 DATA 2330,"INDUSTRIAL REVOLUTION P.2","JEAN-MICHEL JARRE","INDUSTRIAL 2"
1800 DATA 2630,"ISLANDS","MIKE OLDFIELD","ISLANDS"
1810 DATA 3030,"GIMME HOPE JOHANNA","EDDY GRANT","JOHANNA"
1820 DATA 3470,"MAGNETIC FIELDS","JEAN-MICHEL JARRE","MAGNETIC FIELDS"
1825 DATA 5230,"MAGNETIC FIELDS 5,LAST RUMBA",A$,"M. FIELDS 5"
1830 DATA 3810,"MARCHA TURCA","W.A. MOZART","MARCHA TURCA"
1840 DATA 4130,"SUNSHINE","","SUNSHINE"
2020 LET C$="O4V11N1F&V12F&V13F&V14F&V15F&V14F&V13F&V12F&)"
2030 LET A$="T150N9&&&7&V14(7f9_9_9_5C&)7f9_9_9_5C8&V12N9_9_9a9$b9_9_9CH"
2040 LET b$="((9&&&&))V12N9_9_9c9d9_9_9a"
2050 LET d$="T150(5&O6N5f3$bC5D7C7_5a5f3$bC5D9_5C5f12$bCD7C7_5a5a12$baf9f)H"
2060 LET e$="V13(7a7$b9_7a7$b9_7a7$b9_7a7$b9a)"
2070 LET f$="T150O6(5&F12EDC6E3C6D3$b6C3F12EDC7EC5aF12EDC6E3C6D3$b6C3a12$baf9f)H"
2080 LET g$="(V13N9aN7a$b9a7a5CC9a7a$b7ac9a)"
2090 LET h$="T150(5&O6N5f12$bCD7C8a5f12$bCD9_5C5f12$bCD7C8a5a12$baf9f)H"
2100 LET i$="T150O6N8a3FC8a3FC8a3FCH"
2110 LET j$="V13N9cfa"
2120 zxplay3 a$,b$,c$
2130 zxplay3 d$,e$,c$
2140 zxplay3 f$,g$,c$
2150 zxplay3 h$,e$,c$
2160 zxplay3 f$,g$,c$
2170 LET d$(5)=" ":LET d$(61)=" ":LET d$=d$(TO 58)&"8f3FCH"
2180 LET e$(4)=" ":LET e$(33)=" "
2190 zxplay3 d$,e$,c$
2200 zxplay3 i$,j$,c$
2210 zxplay3 "UX50000W2N9_9_9A","UX50000W2N9_9_9C","UX50000W2N9_9_9f"
2220 zxwait 50
2230 RETURN
2330 LET a$="T160O6V10(((3EeeEeEee)))((DddDdDdd))((EeeEeEee))"
2340 LET b$="O3V11(9_7e3e6d9_7c3c6d)9_7g3g6d9_7dO2N7b(9_7_7a)"
2350 LET c$="((3&&&&&&&&))O6(3&V11 3bgegegb)(&CgegegC)O5((3dD1dbag))((3dD1dag#f))(3aAaeaE1bD#Cb)O6(3aAaeaE1AED#C)"
2360 LET d$="O4V12(9_7g#f9_7e#f9_7g#f9_7e5dc7_7_7d5cO3N5b7b7_7ab9_7_7a9_7_7A"
2370 LET e$="O5V12N9_7ba9_7g#f9_7ba9_7ga9_7b5ag7g#f#fg9_9e9_9E"
2380 LET f$="O5V12N9_7G#F9_7ED9_7G5#Fa9_7E5DC9_7D5Cb7baab9_7_7a9_7_7E"
2390 LET g$="O5V12N9_7Ba9_7G#F9_7BA9_7GA9_7B5AG7G9#F7G9_7_7E9_7_7e"
2400 LET j$="UX10000W0O2M21N7eM7O3V12N7_7e3e6d9_7c3c6d9_7e3e6d9_7c5dc"&b$(24 TO )
2410 LET k$="UX10000W0O2M21N7eM7O3V12N7_7e3e6d9_7c3c6d9_7e3e6d9_7c5dcUM21N7gM7O3V12N7_"&b$(26 TO )
2420 LET l$=k$(TO 41)&"UM21O2N7eM7O3V12N7_"&k$(44 TO 88)&"UM21O2N7eM7O3V12N7_7_7a9_7_7a"
2430 LET m$="T160UX40000W0O3N9_9_9e"
2440 LET n$="UO4N9_9_9e":LET o$="UO3N9_9_9b"
2450 zxplay3 a$,b$,c$
2460 zxplay3 a$,b$,d$
2470 zxplay3 a$,j$,e$
2480 zxplay3 a$,k$,f$
2490 zxplay3 a$,l$,g$
2500 zxplay3 m$,n$,o$
2510 RETURN
2630 LET z$="UW0X10000"
2640 LET a$="V10T80O5N5AA1G#FG3A1&3DAGG#F#FEE1#FG5AA1G#FG3A1&3DAGG#FED5D"
2650 LET b$="V11O2N5DDGDGGDGAAGADGDD"
2660 LET c$="V11O3N5#F#FGDGGbGA#Cb#CDbgD"
2670 LET d$="T80O4N1aDa5#F1&DG#F5#F1&"
2680 LET e$="O3N5d2&3d5d2&"
2690 LET f$="T80V13O4N7A5D3&1DE3#FGA7D1E#F3GDA5G3&ED#FG5AD"
2700 LET g$="O2((3DD))((bb))((gg))((DD))"
2710 LET h$="O3((3aa))((#F#F))((DD))((aa))"
2720 LET i$="O2((3DD))"
2730 LET j$="O3N5DDg1a"
2740 LET k$="O3N5#F#FD1E"
2750 LET l$="T80V13O5N1de5#fad3bb5a1e#f3gg#f#f1de5#fad3&bba5a3&"
2760 LET m$="O4N3d5#f#f1#f3d1d5dee1e&3#f1_1#f3d5#f#fd#ceed1e&"
2770 LET n$="O3N3a(3DD)(gg)(aa)((DD))(gg)(aa)gga"
2780 LET o$="T80O4N1DE5#F3#F1#F#FEDE4#F1E#F5GGb3&1#CD5E#F3E1D#C3&1DE5#FG3#F1ED3&"
2790 LET p$="O2N3a5DD1D3D1D5DEE1E3E1E5Eaa1a3a1a5aDG3DD1DD"
2800 LET q$="O4N1#FG5A3A1AAG#FG4A1GA5BBG3&1#FG5AB3A1GE3&1#FG5AB3A1G#A3&"
2810 zxplay2 a$,c$
2820 zxplay3 a$,b$,c$
2830 zxplay3 d$,z$&e$,z$&i$
2840 GO SUB 2850:zxplay1 "T80O2N3a":GO SUB 2850:GO TO 2930
2850 zxplay3 f$,z$&g$,z$&h$
2860 zxplay3 f$,z$&g$,z$&h$
2870 zxplay3 a$(TO 21),j$,k$
2880 zxplay2 l$,z$&n$
2890 zxplay3 l$,z$&m$,z$&n$
2900 zxplay3 o$,p$,q$
2910 zxplay3 o$,p$,q$
2920 RETURN
2930 FOR n=1 TO 2
2940 zxplay3 o$,p$,q$
2950 NEXT n
2960 zxplay1 "T80O2N3D"
2970 zxplay2 "T110UW0X12000O4N8#F","UO4N8D"
2980 zxplay2 "T90UX14000W0O3N8a","UO3N8D"
2990 RETURN
3030 LET a$="T140O5V10(3DD5#C&3#C#CDD5D&&3DD5E&3#C#CDD5D&&)3DD5#C&3#C#CDD5D&&3DD5E&3#C#CDD5D&&3DD5E&3#C#CDD5D&&3DD5#C&3#g#gaa5a&&H"
3040 LET b$="O5V10(3bb5a&3aabb5b&&3bb5#C&3aabb5b&&)3bb5a&3aabb5b&&3bb5#C&3aabb5b&&3bb5#C&3aabb5b&&3bb5a&3eeee5e&&"
3050 LET c$="M35O8UX2000W0N5&&C&))"
3060 LET d$="M35O3UX800W0(1C&cc)O8X2000N3CO4X800N1ccC&cc))"
3070 LET e$="V12O3N3a1ee((3a1ee))((3D1aa))((3a1ee))((3b1ee)(3a1ee))((3D1aa))(3a1ee)(3b1ee)(ea1ee)3a1ee"
3080 LET f$="T140O4V13N3DD(#C)#cb5a3#g5#fa&3&DD(#C)#Cab5#Cb&&5D3D(#C)#Cb5a3#g5#faD3DDD(#C)#Ca5b3ba&7_3EH"
3090 LET g$="T140O4V13N3#CD5E3&a5a3aa&#F&aaa#CD5E3&aaaaab5#Cb3&#CD5E3&a5a3aa&#F&aaa&5#C3#C#Cab5Ea&&3&H":LET j$=f$(TO LEN(f$)-5)&"5&&3&H"
3100 LET h$="T140O7V10N3A#FEEE#C5E3E#F&DE#F&A#G#FEEE#C5E3Eb&5&&e3ea#gab#C#Cb#CD#CDE&#F#F&E&AE#C5ba&&3&H"
3110 LET i$=e$(TO LEN(e$)-5)&"3b1ee"
3120 LET k$="O3V12N3b1ee"&e$(12 TO )
3130 LET l$="T140O4V13N3DD(#C)#Cb5a3#g5#fa6&3DD(#C)#Cab5#Cb7&5D3D(#C)#Cb5a3#g5#faD3DDD(#C)#Cab5E3a&7_3EH"
3140 zxplay3 a$,b$,c$
3150 zxplay3 a$,b$,d$
3160 zxplay3 j$,e$,d$
3170 zxplay3 f$,i$,d$
3180 zxplay3 g$,k$,d$
3190 zxplay3 j$,e$,d$
3200 zxplay3 l$,i$,d$
3210 zxplay3 g$,k$,d$
3220 zxplay3 h$,e$,d$
3230 zxplay3 j$,e$,d$
3240 zxplay3 f$,i$,d$
3250 zxplay3 g$,k$,d$
3260 zxplay3 g$,e$,d$
3270 zxplay3 j$,e$,d$
3280 zxplay3 l$,i$,d$
3290 zxplay3 g$,k$,d$
3300 zxplay3 g$,e$,d$
3310 zxplay3 g$,e$,d$
3320 zxplay3 "T130N3&&UX12000W0T100O4N8a","O3V12N3a1eeUO4N8e","M35UX800W0O3N1C&ccX12000O4M7N8#c"
3330 RETURN
3470 LET a$="T100N1&&O6V11(1#dfgf#d3g1&)(1d#df#dd3f1&)(1cd#ddc3#d1&)O5N1&&DC3&#a1&3#a1&3#a#g1&3#a1&3#gg1&3g1&3gb1&3b1&3bD1&3D1&3DF1&3F1&3F(#G1&3#G1&3#G)#G1&3#G1&3#G"
3480 LET c$="UX1000W0O1M35N1aaO7X1600N1AAO1X1000N1aaO7X1600N1A&))"
3490 LET b$="UO4(((1CC)))(((gg)))(((#g#g)))((#g#g))(((#a#a)))(((gggggg)))((gg))H"
3500 LET d$="UO4(((1#D#D)))(((#a#a)))(((CC)))(((#g#g)))(((ff)))(((gg)))((((gg))))(((gg)))H"
3510 LET e$="T100N1&&O6V11(1#dfgf#d3g1&)(1d#df#dd3f1&)((1cd#ddc3#d1&))1cd#ddc3#d1&cd#dd3cd1&3d1&3d1&3d1&3ddg1&3g1&3g1&3g1&3ggb1&3b1&3b1&3b1&3bbD1&3D1&3D1&3D1&3DD"
3520 LET f$="T100N1&&O6V10(1#D#D#D#D&&#D#D#D3#D#D&1&)(1DDDD&&DDD3DD&1&)(1#D#D#D#D&&#D#D#D3#D#D&1&)1DDDD&&DDD3DD&1&DDDD&&DDD3DD1&H"
3530 LET G$="UO4((((1cc))))((((gg))))(((#g#g)))(((ff)))((((gg))))"
3540 LET h$="T100N1&O6V12(1#DD#D3CcC1c3gC&)(1DCD3bO5N3bG1b3DGO6N3&)1#DD#D3D#d#g1c3#d#g&1#DD#D3Cf#g1c3f#g&1DCD3bdgO5N1b3DG&1dgbD3GO6N1d3b1g3D&1&H"
3550 zxplay3 "T100((1&&&&&&&&))","((1&&&&&&&&))UO4((((1CC))))H",C$
3560 zxplay3 A$,B$,C$
3570 zxplay3 A$,B$,C$
3580 zxplay3 E$,D$,C$
3590 zxplay3 F$,G$,C$
3600 zxplay3 H$,G$,C$
3610 zxplay3 "T100(((1&&)))H","(((1&&)))",C$
3620 zxplay3 A$,B$,C$
3630 zxplay3 A$,B$,C$
3640 zxplay3 E$,D$,C$
3650 zxplay3 F$,G$,C$
3660 zxplay3 H$,G$,C$
3670 zxplay3 A$,B$,C$
3680 zxplay3 "T100O6V11N9_7_7cH","UO4N1c))",c$
3690 zxplay3 "T60UX24000W0O5N9c","UO4N9g","UO4N9c"
3700 RETURN
3810 LET a$="T150V13N(1gfef3$a&1$b$ag$a3C&1$DCbCGFEFGFEF3$A&F$AGF$EFGF$EFGF$EDC&)(V14O6N3C$D$E$E1F$E$DC3$b$EV13C$D$E$E1F$E$DC3$b&V14$a$bCC1$DC$b$a3gCV13$a$bCC1$DC$b$a3g&O5N1gfef3$a&1$b$ag$a3C&1$DCbCGFEFGFEF3$A&FG$AGFEFC$D$b$a&5g3f&)"
3820 LET b$="T150(V15O5N3FG5A3FGAGFEDEFGECFG5A3FGAGFEDGECF&)"
3830 LET d$="T150(O5V14N1A$BAGFGFEDFED$DDE$Dab#CaD$DDEFEFGA$AA$AA$BAGFGFEDFEDCDECabCabCDb#gab#g3a&)"
3840 LET e$="T150(O6V15N1C$bagfga$bCDEFFEDCC$bagfga$bCDEF3#FG1C$bagfga$bCDEFFEDCC$bagaCfag$beg3f&O5V14N1A$BAGFGFEDFED#CDE#Cab#CaD#CDEFEFGA#GA#GA#GA#F($BA$BA)$BAGFEFGEFGAD#CDE#C3D&)"
3850 LET f$="T150(O5V15N3FG5A3FGAGFEDEFGECFG5A3FGAGFEDGECF&)"
3860 LET g$="T150O5V15N4A1A7AA(1$BAGA)7$B3AAAA6GO6N3CO5N7AA(1$BAGA)7$BA3GGGGF&V13N4A1A7AA(1$BAGA)7$B3AAAA6GO6N3CO5N7AA(1$BAGA)7$BA3GGGGV14N6F3A6FO6N3CO5V15N6F3AFAFO6Cf&a&f"
3870 LET h$="T150NO3V14N5f&C&fC3fCCC$aCCC5fCFC3f&"
3880 LET i$="T150(5&O4V13N(3f6&)3f&f&f6&(3c6&)3c&f&c&)(5&O3V14N3$a$aCC5$E&3$a$aCC3$E&5&V14O4N3ff$a$a5C&3ff$a$aC&5&(3f6&)3f&f&$d6&O3N3C&$b&$a&$b&C&C&F&)"
3890 LET j$="(5&V15O4N5f&f&$bbC&f&f&$bC3f&)"
3900 LET k$="(5&O4V15N3d6&3e6&3d6&3#c6&3d6&(3e6&)3a&)"
3910 LET l$="(5&V15O4N5f&3g&e&5f&c&f&3g&e&fdO3$bO4cf6&3d6&3e6&3d6&O3N3a6&3g6&3g6&3a&a&D&)"
3920 LET m$="5&O3V15N5F&F&FF$b&FFC&F&F&FF$b&C&CC3F6&V12N5F&F&FF$b&FFC&V14F&F&FF$b&V15CCCC7FFF3FFFFF&F&F&"
3930 LET n$="T150(5&(3&O4V12NCCC)&C&C(&CCC)&CCC&C&bC&)(V14O5N3$a$bCC1#CC$b$a3g$eV13$a$bCC1#CC$b$a3g&V14fg$a$a1$b$agf3ecV13fg$a$a1$b$agf3e&5&O4N(3&CCC)&C&C&bbb&C&$D&C&$D&C&CC&)"
3940 LET o$="(5&O4V15N3ffffffff$b$bbb5C&3ffffffff$b$bCCf&)"
3950 LET p$="(5&O4V13N3&DDD&#C#C#C&DDD&#C#C#C&DDD&CCC&DDD&)"
3960 LET q$="(5&O4V15N3&CCC&C&C&CCC&CCC&CCC&C&C&C&CC&V13N5&3&DDD&#C#C#C&DDD&#C#C#C&DDD&EEE&D&#CD&)"
3970 LET r$="(5&O4V15N5c&c&3$b$bbbCccc5c&c&3$b$bCCc&)"
3980 LET t$="(5&O4V15N3FFFFFFFFFFFF$b$b$b$bFFFFCCCCFFFFFFFFFFF$b$b$b$bCCCCCCCCF&&&V12(FFFF)FFFF$b$b$b$bFFFFCCCCFFFFFFFFFFFF$b$b$b$bCCCCCCCCV14(FFFFFFFF)F&F&V15F&"
3990 zxplay1 h$
4000 zxplay3 a$,i$,n$
4010 zxplay3 b$,j$,o$
4020 zxplay3 d$,k$,p$
4030 zxplay3 e$,l$,q$
4040 zxplay3 f$,j$,r$
4050 zxplay3 a$,i$,n$
4060 zxplay3 b$,j$,o$
4070 zxplay3 g$,j$,o$
4090 RETURN
4130 LET a$="T200O5V12((3eg8C)3eg5CDDEE&&3CD5EGECagg3ef5bC3CaCD5C&&)3&&O4N7E5DCaggegedc5d&&5e3dc5dfgg&&&7G5FEFEDCagCDG&&GAO5N5CGEDCaage3agCD5C&&H"
4140 LET b$="5&M21O4V9N1c&6&V13N1C&3&)"
4150 LET c$="5&O3V12(7cfcccgcccfcccgcc)cfcccgggcfcccgccffccggcccfc"
4160 LET d$="T200N5&O5V12N7D#FDa5Cbab7b&D#FABO6N3CEDabDCbab5g&H"
4170 LET e$="5&O3V12((7d))((g))((d))gdgg"
4180 LET f$="T200O6N5&V12N3gecd5fdedc&3gecd5fdeag&O5N3gaCD5FDEDC&gC3CaCD5C7&H"
4190 LET g$="5&O3V12(7cfcccf5cg7c)"
4200 LET h$="T200N5&O6V10N9_7_7gUX500W4N9_7_7gH"
4210 LET i$="O4V11N5c9_7_7c"
4220 zxplay3 a$,b$,c$
4230 zxplay3 a$,b$,c$
4240 zxplay3 d$,b$,e$
4250 zxplay3 a$,b$,c$
4260 zxplay3 d$,b$,e$
4270 zxplay3 f$,b$,g$
4280 zxplay3 a$,b$,c$
4290 zxplay3 h$,b$,i$
4300 RETURN
4410 LET A$="T80UX5000W0O5(3b1&3C1&&&3b1&3a1&&&)(3g1&3a1&&&3g1&3#f1&&&)3b1&3C1&&&3b1&3a1&&&3b1&3C1&&&3b1&3a1eEC"
4420 LET b$="UO5(3g1&3a1&&&3g1&3#f1&&&)(3e1&3#f1&&&3e1&3#d1&&&)3g1&3a1&&&3g1&3#f1&&&3g1&3a1&&&3g1&3#f1cCa"
4430 LET c$="V12O3((((1ee))))(((cc)))(((aa)))(((gg)))(((bb)))"
4440 LET d$="T80UX5000W0O5N3bCa1g3b&&1eEC3bCa1g3b&&(1eag3#fge1#d3#f&&)(1#fga3bCa1g3b&&)1eEC"
4450 LET e$="UO4N3GA#F1E3G&&O5N1cCa3ga#f1e3g&&O4(1a#FE3#DEC1b3#D&&)1#DE#F3GA#F1E3G&&1#DE#F3GA#F1E3#D&&O5N1cCa"
4460 LET f$=d$(TO 57)&"1#fga3bCa1g3b&&1#fga3bCag"
4470 LET g$=e$(TO 54)&"1#DE#F3GA#F1E3G&&1#DE#F3GA#FE"
4480 LET h$=c$(TO 32)&"(((gg)))((gg))"
4490 LET i$="T80V12O4N1BAG#FE#DCb":LET j$="V12O4N1#FE#DCbag#f":LET k$="V12O3((1bb))"
4500 LET l$="T80UX10000W0O4N1bAG#F6E#DE#FO5N6aDC4#aa6G4#FA6G#Daa4bC6Cb4agX50000N6_6#dX10000N4e6#f&O4((1b))((#D))((#F))((#F))3#F&&X2600W3M8N3b"
4510 LET m$="UO3 1bAG#F6E#DE#FO4N6aDC4#aa6G4#FA6G#Daa4bC6Cb4ag6_6#d4e6#f&O3((1b))((#D))((#F))((A))3B&"
4520 LET n$="V12O3N1bE#DC6aaaaa6ggggggCCCCgggCC4C6C&((1b))((b))((b))((b))3b"
4530 LET r$="T80UX5000W0O5(3b1&C3a1&b3g1&a3#f1&&)":LET s$="UO4(3#F1&G3E1&#F3#D1&E3b1&&)":LET t$="V12O3((((1bb))))"
4540 LET u$="T80UX5000W0O5N3bT85N3CT80N3aT75N3bT70N3gT65N3aT60N3#f&":LET v$="UO5N3#fge#f#de#c#d&":LET w$="V12O3(((1bb)))3&"
4550 LET x$="T70UX5000W0O4N3#g&(((#g#g#g))(ggg)(#f#f#f))(eee)#f#f#f5#f"
4560 LET y$="UO5N3e&(O4 3b&&&&&b&&&&&b&&b&a#f&&#d&&)b&&b&a#f&&5#f"
4570 LET z$="UO3N3ee(EEEM35N1E&3EM7N3EDDDM35N1d&3DM7N3DCCCM35N1c&3CM7N3CbbbM35N1b&3BM7N3b)aaaM35N1a&3AM7N3abbbM35N5b"
4580 zxplay3 a$,b$,h$&"((gg))"
4590 zxplay3 d$,e$,c$
4600 zxplay3 f$,g$,h$
4610 zxplay3 i$,j$,k$
4620 zxplay3 l$,m$,n$
4630 zxplay3 a$,b$,h$&"((gg))"
4640 zxplay3 d$,e$,c$
4650 zxplay3 d$(TO LEN(d$)-3)&"#fga",e$(TO LEN(e$)-11)&"G&&O5N1#de#f",h$&"((gg))"
4660 zxplay3 r$,s$,t$
4670 zxplay3 u$,v$,w$
4680 zxplay3 x$,y$,z$
4690 RETURN
5230 LET a$="O4V10N3g&&g&dg&f&&f&cf&c&&c&cc&c"
5240 LET b$="M35O2V9N1c&V7O8N4C1&V9O2N1c&V6N1c&c&V8N1C&C&)"
5250 LET c$="O4V11(((1b&)))(((a&)))((g&))g&g&g&&&gH"
5260 LET e$="O5V10N7g3gabC8e5C7g3gabC8f5D7b3fgga8a3def5g3ab5C3b8g5C7g3gabC8e5C7g3gabC8f5D7b3fgga8a3def5g3ab5C3b8C5C3a#g7a5f7D3C6a3g#f7g5e7C3b6a7g&3&5d3efgabH":LET f$="O5V12N9E5D":LET g$="O5V12N9C3CbCDCbba6a3g7g3gefgagfe6e3d7d3ddefgfed7c&1&H"
5270 LET h$="O3V9(3cC&cC&C&)cC&cC&C&((gD&gD&D&))((cC&cC&C&))((gD&gD&D&))cC&cC&C&(fC&fC&C&)(cC&cC&C&)(gD&gD&D&)"
5280 LET i$="O3V9(3cf&cf&f&)(cC&cC&C&)(gD&gD&D&)cC&cC&C&c"
5290 LET j$="O5V9(((3C#DG)))4#G1G3F8#G3#AG#GF5G3#DF1F#D3D7F5&3G#DFD5#D3b8C5&((3C#DG))C#DGC4#G1G3F8#G3#AG#GF5G3#DF1F#D3D8F3G#DFD5#D3b8C5&H":LET l$="O5N3&V9N3#D#DDDCC5C#g6g&3#D#DDDCC1b5Cg#a#g1g#g6#g&4&3&#a#a#g#ggggg5ff&&&&&&b#g6g3gff#d#ddd#d7f3fggff#d#ddd#d#dddccO4N3b7C&&1&H"
5300 LET k$="O3V9(3#dC&#dC&C&)#dC&#dC&C&(fC&fC&C&)(gD&gD&D&)(#dC&#dC&C)#dC&#dC&C&(fC&fC&C&)(gD&gD&D&)#dC&#dC&C&"
5310 LET m$="O3V9((3#dC&#dC&C&))(fC&fC&C&)((gD&gD&D&))(gD&gD&D&)gD&gD&D&#dC&#dC&#dC&C&c&"
5320 LET n$="O5V9N3C#D4G1C3#DG(C#DG)C#DGC#G1#GG3F8#G&5&3#AG#GFGG#DF1F#D3D8F3G#DFD#D#Db8C5CH"
5330 LET o$="O3V9(3#dC&#dC&C&)(CF&CF&F&)CF&CF&F&(DG&DG&G&)#dC&#dC&C&"
5440 zxplay3 a$,c$,b$
5450 zxplay1 "7&O5V10N7C"
5460 zxplay3 e$,h$,b$
5470 zxplay1 f$
5480 zxplay3 g$,i$,b$
5490 zxplay1 "1&6&O5V10N7C"
5500 zxplay3 e$,h$,b$
5510 zxplay1 "O5V10N8E5D"
5520 zxplay3 g$,i$,b$
5530 zxplay1 "1&8&"
5540 zxplay3 j$,k$,b$
5550 zxplay3 l$,m$,b$
5560 zxplay1 "6&"
5570 zxplay3 n$,o$,b$
5580 zxplay2 "O5V10N3CV9CV8CV7CV5CV3CV1C","O3V9N3C"
5590 RETURN
5610 LET a$="O4N1dO5EDDaaO4dFdO5EDaaaO4FO3gO5C#a#affO3gO5dO3gO5C#afffO3gO5fO3#aO5#aaaffO3#a#A#a#A#A#A#A#A#a#AaA#A#AO5ccO3aO5dO3aO5efgggO3aO5aO4dO5EDDaaO4dFdO5EDaaaO4dFO3gO5C#a#affO3gO5dO3gO5C#afffO3gO5fO3#aO5#aaaffO3#a#A#a#A#A#A#A#A#a#AaA#A#AO5ccO3aO5dO3dO5efgggO3aO5aO4d&a&&&ggdgffffddO3gDD&&&g&g&&&a&D&#aA#A#A#A#A#aG#aGD&F&#a&a&&&G&a&a&&&E&a&D&DEF&DA&&"
5620 zxplay1 a$:RETURN
8000 REMark ===== ZX Spectrum 128 -> QL/QSound compatibility =====
8010 DEFine PROCedure zxmenu
8020  LET VO=1:LET lin=6:LET col=0
8030  FOR n=1 TO 26
8040   GO SUB 10:READ aa,a$,a$,a$
8050   AT lin,col:OVER 0:PRINT a$;
8060   LET lin=lin+1:IF lin=21 THEN LET lin=6:LET col=16
8070  END FOR n
8080 END DEFine zxmenu
8100 DEFine PROCedure zxselect
8110  GO SUB 31:RESTORE 1600
8120  FOR n=1 TO a
8130   READ lin,t$,a$,b$
8140  END FOR n
8150  CLS:GO SUB 35:GO SUB lin:PAUSE
8160 END DEFine zxselect
8180 DEFine PROCedure zxregs
8190  FOR zn=0 TO 13
8200   READ zm:POKE_AY zn,zm
8210  END FOR zn
8220 END DEFine zxregs
8240 DEFine PROCedure zxreadpair
8250  READ b$,c$:LET m$=m$&b$:LET j$=j$&c$
8260 END DEFine zxreadpair
8300 DEFine FuNction zxkey
8310  zk$=INKEY$(-1):zc=CODE(zk$)
8320  IF zc=192 THEN RETurn 8
8330  IF zc=200 THEN RETurn 9
8340  IF zc=216 THEN RETurn 10
8350  IF zc=208 THEN RETurn 11
8360  IF zc=10 OR zc=13 THEN RETurn 13
8370  RETurn zc
8380 END DEFine zxkey
8400 DEFine PROCedure zxanim1
8410  FOR zn=1 TO LEN(M$)
8420   FOR zp=30 TO zn-1+X STEP -1
8430    AT Y,zp:PRINT M$(zn);
8440   END FOR zp
8450  END FOR zn
8460 END DEFine zxanim1
8480 DEFine PROCedure zxanim2
8490  FOR zn=1 TO LEN(M$)
8500   FOR zp=1 TO Y-1
8510    AT zp,zn-1+X:OVER -1:PRINT M$(zn);:OVER 0
8520   END FOR zp
8530   AT Y,zn-1+X:PRINT M$(zn);
8540  END FOR zn
8550 END DEFine zxanim2
8570 DEFine PROCedure zxanim3
8580  FOR zn=1 TO INT(LEN(M$)/2)+1
8590   AT Y,X+zn-1:PRINT M$(zn);
8600   AT Y,X+LEN(M$)-zn:PRINT M$(LEN(M$)+1-zn);
8610  END FOR zn
8620 END DEFine zxanim3
8640 DEFine PROCedure zxanim4
8650  FOR zn=1 TO LEN(M$)
8660   zx1=31*INT(RND*2):zy1=21*INT(RND*2):zdx=(X-zx1)/10:zdy=(Y-zy1)/10
8670   FOR zp=1 TO 10
8680    AT INT(zy1),INT(zx1):OVER -1:PRINT M$(zn);:OVER 0
8690    zx1=zx1+zdx:zy1=zy1+zdy
8700   END FOR zp
8710   AT Y,X:PRINT M$(zn);:LET X=X+1
8720  END FOR zn
8730 END DEFine zxanim4
8750 DEFine PROCedure zxanim5
8760  zaa=LEN(M$):AT Y,X+INT(zaa/2):PRINT M$(1);
8770  FOR zn=1 TO INT(zaa/2)
8780   FOR zm=1 TO zn
8790    AT Y,X+INT(zaa/2)-zn+zm:PRINT M$(zm);
8800    AT Y,X+INT(zaa/2)-zm+zn+1:PRINT M$(zaa+1-zm);
8810   END FOR zm
8820  END FOR zn
8830 END DEFine zxanim5
8850 DEFine PROCedure zxanim6
8860  zn$=M$:zx0=X
8870  FOR zz=1 TO LEN(zn$)
8880   X=zx0+zz-1:M$=zn$(zz):GO SUB 67
8890  END FOR zz
8900 END DEFine zxanim6

8920 DEFine PROCedure zxcentre
8930  IF (LEN(M$) MOD 2)<>0 THEN LET M$=M$&" "
8940  LET X=15-LEN(M$)/2
8950 END DEFine zxcentre

9000 DEFine FuNction zxtempo(zs$)
9010  zt=120:zi=1:zcomment=0
9020  REPeat ztl
9030   IF zi>LEN(zs$) THEN EXIT ztl
9040   zc$=zs$(zi)
9050   IF zc$="!" THEN zcomment=1-zcomment:zi=zi+1
9060   IF zcomment=0 AND (zc$="T" OR zc$="t") THEN zxnum zs$,zi:IF zj>zi+1 THEN zt=zn:EXIT ztl
9070   zi=zi+1
9080  END REPeat ztl
9090  RETurn zt
9100 END DEFine zxtempo

9120 DEFine PROCedure zxnum(zs$,zp)
9130  zj=zp+1
9140  REPeat znp
9150   IF zj>LEN(zs$) THEN EXIT znp
9160   zcode=CODE(zs$(zj)):IF zcode<48 OR zcode>57 THEN EXIT znp
9170   zj=zj+1
9180  END REPeat znp
9190  zn=0:IF zj>zp+1 THEN zn=zs$(zp+1 TO zj-1)
9200 END DEFine zxnum

9220 DEFine FuNction zxdur(zd,zt)
9230  zu=24
9240  IF zd=1 THEN zu=6
9250  IF zd=2 THEN zu=9
9260  IF zd=3 THEN zu=12
9270  IF zd=4 THEN zu=18
9280  IF zd=5 THEN zu=24
9290  IF zd=6 THEN zu=36
9300  IF zd=7 THEN zu=48
9310  IF zd=8 THEN zu=72
9320  IF zd=9 THEN zu=96
9330  IF zd=10 THEN zu=4
9340  IF zd=11 THEN zu=8
9350  IF zd=12 THEN zu=16
9360  zf=INT(zu*125/zt+.5):IF zf<1 THEN zf=1
9370  IF zf>255 THEN zf=255
9380  RETurn zf
9390 END DEFine zxdur

9410 DEFine FuNction zxexpand$(zs$)
9420  ze$=zs$
9430  REPeat zel
9440   zopen=0:zclose=0:zi=1
9450   REPeat zes
9460    IF zi>LEN(ze$) THEN EXIT zes
9470    IF ze$(zi)="(" THEN zopen=zi
9480    IF ze$(zi)=")" AND zopen>0 THEN zclose=zi:EXIT zes
9490    zi=zi+1
9500   END REPeat zes
9510   IF zclose=0 THEN EXIT zel
9520   za$="":IF zopen>1 THEN za$=ze$(1 TO zopen-1)
9530   zb$="":IF zclose>zopen+1 THEN zb$=ze$(zopen+1 TO zclose-1)
9540   zc$="":IF zclose<LEN(ze$) THEN zc$=ze$(zclose+1 TO)
9550   ze$=za$&"N"&zb$&"N"&zb$&"N"&zc$
9560  END REPeat zel
9570  RETurn ze$
9580 END DEFine zxexpand$

9600 DEFine FuNction zxqs$(zs$,zt)
9610  zr$=zxexpand$(zs$):zq$="v15":zi=1:zo=5:zd=5:zold=5:ztrip=0:ztie=0:za$="":zxframes=0:zxloop=0:zcomment=0
9620  REPeat zql
9630   IF zi>LEN(zr$) THEN EXIT zql
9640   zch$=zr$(zi)
9650   IF zcomment THEN
9660    IF zch$="!" THEN zcomment=0
9670    zi=zi+1:NEXT zql
9680   END IF
9690   IF zch$="!" THEN zcomment=1:zi=zi+1:NEXT zql
9700   IF zch$=" " OR zch$="N" OR zch$="(" THEN zi=zi+1:NEXT zql
9710   IF zch$=")" THEN zxloop=1:EXIT zql
9720   IF zch$="H" THEN EXIT zql
9730   IF zch$="#" OR zch$="$" THEN za$=zch$:zi=zi+1:NEXT zql
9740   zcode=CODE(zch$)
9750   IF zcode>=48 AND zcode<=57 THEN
9760    zj=zi
9770    REPeat zqn
9780     IF zj>LEN(zr$) THEN EXIT zqn
9790     zcode=CODE(zr$(zj)):IF zcode<48 OR zcode>57 THEN EXIT zqn
9800     zj=zj+1
9810    END REPeat zqn
9820    zn=zr$(zi TO zj-1)
9830    IF zn>=1 AND zn<=12 THEN
9840     IF zn>=10 THEN zold=zd:ztrip=3
9850     zd=zn
9860     IF zj<=LEN(zr$) THEN IF zr$(zj)="_" THEN ztie=ztie+zxdur(zn,zt):zj=zj+1
9870    END IF
9880    zi=zj:NEXT zql
9890   END IF
9900   IF zch$="T" THEN zxnum zr$,zi:IF zn>0 THEN zt=zn
9910   IF zch$="T" THEN zi=zj:NEXT zql
9920   IF zch$="O" THEN zxnum zr$,zi:zo=zn:zi=zj:NEXT zql
9930   IF zch$="V" THEN zxnum zr$,zi:IF zn<0 THEN zn=0
9940   IF zch$="V" THEN IF zn>15 THEN zn=15
9950   IF zch$="V" THEN zq$=zq$&"v"&zn:zi=zj:NEXT zql
9960   IF zch$="U" THEN zq$=zq$&"v16":zi=zi+1:NEXT zql
9970   IF zch$="W" THEN zxnum zr$,zi:zw=0
9980   IF zch$="W" THEN IF zn=1 THEN zw=4
9990   IF zch$="W" THEN IF zn=2 THEN zw=11
10000  IF zch$="W" THEN IF zn=3 THEN zw=13
10010  IF zch$="W" THEN IF zn=4 THEN zw=8
10020  IF zch$="W" THEN IF zn=5 THEN zw=12
10030  IF zch$="W" THEN IF zn=6 THEN zw=14
10040  IF zch$="W" THEN IF zn=7 THEN zw=10
10050  IF zch$="W" THEN zq$=zq$&"w"&zw:zi=zj:NEXT zql
10060  IF zch$="X" THEN zxnum zr$,zi:IF zn>32767 THEN zn=32767
10070  IF zch$="X" THEN zq$=zq$&"x"&zn:zi=zj:NEXT zql
10080  IF zch$="M" THEN zxnum zr$,zi:zi=zj:NEXT zql
10090  IF zch$="Y" OR zch$="Z" THEN zxnum zr$,zi:zi=zj:NEXT zql
10100  IF zch$="&" THEN
10110   zf=zxdur(zd,zt)+ztie:ztie=0:IF zf>255 THEN zf=255
10120   zq$=zq$&"l"&zf&"p":zxframes=zxframes+zf
10130   IF ztrip>0 THEN ztrip=ztrip-1
10140   IF ztrip=0 AND zd>=10 THEN zd=zold
10150   zi=zi+1:NEXT zql
10160  END IF
10170  IF zch$="_" THEN zi=zi+1:NEXT zql
10180  IF (zch$>="a" AND zch$<="g") OR (zch$>="A" AND zch$<="G") THEN
10190   zupper=0:IF zch$>="A" AND zch$<="G" THEN zupper=1
10200   znote$=zch$:IF zch$>="a" AND zch$<="g" THEN znote$=CHR$(CODE(zch$)-32)
10210   IF znote$="B" THEN znote$="H"
10220   zqo=zo-1+zupper:IF zqo<0 THEN zqo=0
10230   IF zqo>7 THEN zqo=7
10240   zf=zxdur(zd,zt)+ztie:ztie=0:IF zf>255 THEN zf=255
10250   zq$=zq$&"o"&zqo&"l"&zf
10260   zq$=zq$&znote$
10270   IF za$="#" THEN zq$=zq$&"s"
10280   IF za$="$" THEN zq$=zq$&"b":za$="":zxframes=zxframes+zf
10290   IF ztrip>0 THEN ztrip=ztrip-1
10300   IF ztrip=0 AND zd>=10 THEN zd=zold
10310   zi=zi+1:NEXT zql
10320  END IF
10330  zi=zi+1
10340  END REPeat zql
10350  RETurn zq$
10360 END DEFine zxqs$

10400 DEFine FuNction zxfill$(zq$,zf,ztarget)
10410  IF zf<=0 THEN RETurn zq$
10420  zn=INT(ztarget/zf)+2:IF zn<2 THEN zn=2
10430  zz$="":FOR zi=1 TO zn:zz$=zz$&zq$
10440  RETurn zz$
10450 END DEFine zxfill$

10500 DEFine PROCedure zxwait(zframes)
10510  IF zframes<1 THEN RETurn
10520  FOR zwait=1 TO zframes
10530   PAUSE 1
10540  END FOR zwait
10550 END DEFine zxwait

10600 DEFine PROCedure zxplay1(s1$)
10610  zt=zxtempo(s1$):q1$=zxqs$(s1$,zt):f1=zxframes:l1=zxloop
10620  IF l1 THEN q1$=zxfill$(q1$,f1,3000):f1=3000
10630  SOUND_AY:HOLD:PLAY 1,q1$:RELEASE
10640  zxwait f1:SOUND_AY
10650 END DEFine zxplay1

10700 DEFine PROCedure zxplay2(s1$,s2$)
10710  zt=zxtempo(s1$):q1$=zxqs$(s1$,zt):f1=zxframes:l1=zxloop
10720  q2$=zxqs$(s2$,zt):f2=zxframes:l2=zxloop
10730  target=f1:IF f2>target THEN target=f2
10740  IF l1 THEN q1$=zxfill$(q1$,f1,target)
10750  IF l2 THEN q2$=zxfill$(q2$,f2,target)
10760  SOUND_AY:HOLD:PLAY 1,q1$:PLAY 2,q2$:RELEASE
10770  zxwait target:SOUND_AY
10780 END DEFine zxplay2

10800 DEFine PROCedure zxplay3(s1$,s2$,s3$)
10810  zt=zxtempo(s1$):q1$=zxqs$(s1$,zt):f1=zxframes:l1=zxloop
10820  q2$=zxqs$(s2$,zt):f2=zxframes:l2=zxloop
10830  q3$=zxqs$(s3$,zt):f3=zxframes:l3=zxloop
10840  target=f1:IF f2>target THEN target=f2
10850  IF f3>target THEN target=f3
10860  IF l1 THEN q1$=zxfill$(q1$,f1,target)
10870  IF l2 THEN q2$=zxfill$(q2$,f2,target)
10880  IF l3 THEN q3$=zxfill$(q3$,f3,target)
10890  SOUND_AY:HOLD:PLAY 1,q1$:PLAY 2,q2$:PLAY 3,q3$:RELEASE
10900  zxwait target:SOUND_AY
10910 END DEFine zxplay3
