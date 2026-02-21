@echo off
REM Build tools against the cubiomes-rebuild library
REM Run from cubiomes-rebuild directory: tools\build_tools.bat

set CC=gcc
set CFLAGS=-fwrapv -O3 -march=native -mtune=native -flto -fopenmp -ffast-math -funroll-loops -Wall
set INCLUDES=-I..
set LIBS=../lib/libcubiomes.a -lm

if not exist "..\lib\libcubiomes.a" (
    echo Library not built! Run build.bat first.
    goto end
)

echo Building find_villages...
%CC% find_villages.c %INCLUDES% -o find_villages.exe %CFLAGS% %LIBS%
if errorlevel 1 goto error

echo Done! Run: find_villages.exe
goto end

:error
echo Build failed!

:end
