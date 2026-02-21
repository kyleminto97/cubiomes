@echo off
REM Cubiomes Rebuild - Test Runner
REM Runs all test executables

echo ========================================
echo Cubiomes Rebuild - Test Runner
echo ========================================
echo.

if not exist "bin\test_noise.exe" (
    echo Tests not built! Run: build.bat test
    goto end
)

set PASS=0
set FAIL=0

echo Running test_noise...
bin\test_noise.exe
if errorlevel 1 (set /a FAIL+=1) else (set /a PASS+=1)
echo.

echo Running test_biomenoise...
bin\test_biomenoise.exe
if errorlevel 1 (set /a FAIL+=1) else (set /a PASS+=1)
echo.

echo Running test_generator...
bin\test_generator.exe
if errorlevel 1 (set /a FAIL+=1) else (set /a PASS+=1)
echo.

echo Running test_biomes...
bin\test_biomes.exe
if errorlevel 1 (set /a FAIL+=1) else (set /a PASS+=1)
echo.

echo Running test_finders...
bin\test_finders.exe
if errorlevel 1 (set /a FAIL+=1) else (set /a PASS+=1)
echo.

echo Running test_util...
bin\test_util.exe
if errorlevel 1 (set /a FAIL+=1) else (set /a PASS+=1)
echo.

echo Running test_validation...
bin\test_validation.exe
if errorlevel 1 (set /a FAIL+=1) else (set /a PASS+=1)
echo.

echo ========================================
echo Test Summary
echo ========================================
echo Passed: %PASS%
echo Failed: %FAIL%
echo.

if %FAIL% GTR 0 (
    echo Some tests failed!
    exit /b 1
) else (
    echo All tests passed!
    exit /b 0
)

:end
