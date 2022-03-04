echo off

rem important use %1 and %2 without "". The "" are already sent by the emulator

if defined ProgramFiles(x86) (
  cscript "c:\Program Files (x86)\Jampal\ptts.vbs" -u %1
) else (
  cscript "c:\Program Files\Jampal\ptts.vbs" -u %1
)

rem Tell speech finished
del %2
