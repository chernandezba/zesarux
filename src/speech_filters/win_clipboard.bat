echo off
rem This copies the text sent by the emulator to the clipboard.
rem Use this filter with NVDA and this add-on: https://nvda-addons.org/addon.php?id=314

type %1 | clip
rem Delete temporary file
del %2
