@echo off
REM Cubiomes Rebuild - Clean Script
REM Removes all build artifacts

echo Cleaning build artifacts...

if exist "lib" (
    rmdir /s /q lib
    echo Removed lib/
)

if exist "bin" (
    rmdir /s /q bin
    echo Removed bin/
)

echo.
echo Clean complete!
