echo off

rem important use %1 and %2 without "". The "" are already sent by the emulator

if defined ProgramFiles(x86) (
  "c:\Program Files (x86)\eSpeak\command_line\espeak.exe" -f %1
) else (
  "c:\Program Files\eSpeak\command_line\espeak.exe" -f %1
)

rem Tell speech finished
del %2
