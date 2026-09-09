@echo off

REM stas, (copyright) 2026
REM make sure that you have g++ installed

CLS
ECHO *******************************************
ECHO stas, (copyright) 2026
ECHO make sure that you have g++ installed
ECHO because i am lazy to make checks in script
ECHO *******************************************
ECHO.

ECHO creating object file for library...
g++ -c -fPIC BootCamp.cpp -o BootCampObj.o
if errorlevel 1 goto error
ECHO done...

ECHO creating shared library...
g++ -shared BootCampObj.o -o MathLib.dll
if errorlevel 1 goto error
ECHO done! exiting

del BootCampObj.o
pause
exit /b 0

:error
ECHO Build failed!
pause
exit /b 1