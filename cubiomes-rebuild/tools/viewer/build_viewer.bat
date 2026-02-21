@echo off
REM Build the cubiomes viewer
REM Run from cubiomes-rebuild/tools/viewer/

set CC=gcc
set CFLAGS=-O3 -march=native -mtune=native -fwrapv -fopenmp -Wall
set CUBIOMES_INC=-I../..
set CUBIOMES_LIB=../../lib/libcubiomes.a
set RAYLIB_INC=-Iraylib/include
set RAYLIB_LIB=-Lraylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm
set LIBS=-lm

if not exist "..\..\lib\libcubiomes.a" (
    echo cubiomes library not built! Run build.bat from cubiomes-rebuild first.
    goto end
)

echo Building viewer...
%CC% viewer.c %CUBIOMES_INC% %RAYLIB_INC% %CUBIOMES_LIB% %RAYLIB_LIB% %LIBS% %CFLAGS% -o viewer.exe
if errorlevel 1 goto error

echo.
echo Done! Run: viewer.exe
goto end

:error
echo Build failed!

:end
