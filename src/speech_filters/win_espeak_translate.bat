echo off

rem important use %1 and %2 without "". The "" are already sent by the emulator

if defined ProgramFiles(x86) (
  "c:\Program Files (x86)\eSpeak\command_line\espeak.exe" -f %1
) else (
  "c:\Program Files\eSpeak\command_line\espeak.exe" -f %1
)

rem Translation on this script is not implemented
rem You must add here the translation tool execution (reading from file %1) and put the translated text on file parameter %3

rem Tell speech finished
del %2
