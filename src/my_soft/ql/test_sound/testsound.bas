10 CLS
20 LET duration=15600*2
30 PRINT "simple sound"
40 BEEP duration,50
50 PAUSE 90
60 IF BEEPING THEN GO TO 60
70 PRINT "sound with no wrap"
80 BEEP duration,50,30,3000,-5
90 IF BEEPING THEN GO TO 90
100 PRINT "sound wrap 6 times"
110 BEEP duration,50,30,3000,-5,6
120 IF BEEPING THEN GO TO 120
130 PRINT "sound wrap 6 times and enough to hear"
140 BEEP duration,50,40,2000,-5,6
150 PAUSE 50
160 IF BEEPING THEN GO TO 160
170 PRINT "sound wrap forever"
180 BEEP duration,50,40,2000,-5,15
190 IF BEEPING THEN GO TO 190
200 PRINT "sound wrap forever with random"
210 BEEP duration,50,40,2000,-5,15,0,15
220 IF BEEPING THEN GO TO 220
230 PRINT "sound with fuziness"
240 BEEP duration,50,0,0,0,0,15,0
250 PAUSE 50
260 IF BEEPING THEN GO TO 260
270 PRINT "same sound without fuziness"
280 BEEP duration,50,0,0,0,0,0,0
290 STOP
300 DEFine PROCedure grabar : SAVE mdv1_testsound
310 END DEFine 
