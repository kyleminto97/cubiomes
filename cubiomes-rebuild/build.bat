@echo off
REM Cubiomes Rebuild - Build Script
REM Builds the library and all tests with full optimizations

echo ========================================
echo Cubiomes Rebuild - Build Script
echo ========================================
echo.

REM Compiler settings - aggressive optimization
set CC=gcc
set CFLAGS=-fwrapv -O3 -Wall -Wextra -march=native -mtune=native -flto -fopenmp -ffast-math -funroll-loops
set INCLUDES=-I.
set LDFLAGS=-flto -fopenmp -lm

REM Create output directory
if not exist "lib" mkdir lib
if not exist "bin" mkdir bin

echo [1/8] Compiling noise.c...
%CC% -c noise.c %INCLUDES% -o lib/noise.o %CFLAGS%
if errorlevel 1 goto error

echo [2/8] Compiling biomenoise.c...
%CC% -c biomenoise.c %INCLUDES% -o lib/biomenoise.o %CFLAGS%
if errorlevel 1 goto error

echo [3/8] Compiling biomes.c...
%CC% -c biomes.c %INCLUDES% -o lib/biomes.o %CFLAGS%
if errorlevel 1 goto error

echo [4/8] Compiling generator.c...
%CC% -c generator.c %INCLUDES% -o lib/generator.o %CFLAGS%
if errorlevel 1 goto error

echo [5/8] Compiling finders.c...
%CC% -c finders.c %INCLUDES% -o lib/finders.o %CFLAGS%
if errorlevel 1 goto error

echo [6/8] Compiling util.c...
%CC% -c util.c %INCLUDES% -o lib/util.o %CFLAGS%
if errorlevel 1 goto error

echo [7/8] Creating static library...
gcc-ar rcs lib/libcubiomes.a lib/noise.o lib/biomenoise.o lib/biomes.o lib/generator.o lib/finders.o lib/util.o
if errorlevel 1 goto error

echo [8/8] Library built successfully!
echo.
echo Output: lib/libcubiomes.a
echo.

REM Optional: Build tests
if "%1"=="test" goto build_tests
if "%1"=="tests" goto build_tests
goto success

:build_tests
echo ========================================
echo Building Tests
echo ========================================
echo.

echo Building test_noise...
%CC% tests/test_noise_rebuild.c lib/noise.o %INCLUDES% -o bin/test_noise.exe %CFLAGS% %LDFLAGS%
if errorlevel 1 goto error

echo Building test_biomenoise...
%CC% tests/test_biomenoise.c lib/biomenoise.o lib/noise.o lib/biomes.o %INCLUDES% -o bin/test_biomenoise.exe %CFLAGS% %LDFLAGS%
if errorlevel 1 goto error

echo Building test_generator...
%CC% tests/test_generator.c lib/generator.o lib/biomenoise.o lib/noise.o lib/biomes.o %INCLUDES% -o bin/test_generator.exe %CFLAGS% %LDFLAGS%
if errorlevel 1 goto error

echo Building test_biomes...
%CC% tests/test_biomes.c lib/biomes.o %INCLUDES% -o bin/test_biomes.exe %CFLAGS% %LDFLAGS%
if errorlevel 1 goto error

echo Building test_finders...
%CC% tests/test_finders.c lib/finders.o lib/generator.o lib/biomenoise.o lib/noise.o lib/biomes.o %INCLUDES% -o bin/test_finders.exe %CFLAGS% %LDFLAGS%
if errorlevel 1 goto error

echo Building test_util...
%CC% tests/test_util.c lib/util.o lib/biomes.o %INCLUDES% -o bin/test_util.exe %CFLAGS% %LDFLAGS%
if errorlevel 1 goto error

echo Building test_validation...
%CC% tests/test_validation.c lib/generator.o lib/biomenoise.o lib/noise.o lib/biomes.o lib/finders.o %INCLUDES% -o bin/test_validation.exe %CFLAGS% %LDFLAGS%
if errorlevel 1 goto error

echo.
echo All tests built successfully!
echo Tests are in: bin/
echo.

:success
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Library: lib/libcubiomes.a
if "%1"=="test" echo Tests:   bin/*.exe
if "%1"=="tests" echo Tests:   bin/*.exe
echo.
echo Usage:
echo   build.bat         - Build library only
echo   build.bat test    - Build library and tests
echo   clean.bat         - Clean build artifacts
echo.
goto end

:error
echo.
echo ========================================
echo Build Failed!
echo ========================================
echo.
goto end

:end
