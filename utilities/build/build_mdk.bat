@echo off
setlocal

:: Define paths
set "UV4_PATH=%LOCALAPPDATA%\Keil_v5\UV4\UV4.exe"
set "PROJ_DIR=%~dp0"
set "PROJ_FILE=uart_demo.uvprojx"
set "LOG_FILE=build_output.log"

:: Change to project directory
cd /d "%PROJ_DIR%"

:: Parse Arguments
set DO_CLEAN=0
set DO_BUILD=1

:parse_args
if "%~1"=="" goto done_args
if /i "%~1"=="-c" (
    set DO_CLEAN=1
    set DO_BUILD=0
) else if /i "%~1"=="-r" (
    set DO_CLEAN=1
    set DO_BUILD=1
) else if /i "%~1"=="-p" (
    if not "%~2"=="" (
        set "PROJ_FILE=%~2"
        shift
    )
) else (
    set "PROJ_FILE=%~1"
)
shift
goto parse_args
:done_args

:: Perform Clean if requested
if "%DO_CLEAN%"=="1" (
    echo [INFO] Performing Clean...
    if exist "Objects" (
        echo Removing Objects...
        rd /s /q "Objects"
    )
    if exist "Listings" (
        echo Removing Listings...
        rd /s /q "Listings"
    )
    if exist "bin" (
        echo Removing bin...
        rd /s /q "bin"
    )
    if exist "%LOG_FILE%" del "%LOG_FILE%"
    echo [INFO] Clean finished.
)

:: Exit if Build is not requested
if "%DO_BUILD%"=="0" (
    exit /b 0
)

:: Check if UV4 exists before building
if not exist "%UV4_PATH%" (
    echo [ERROR] UV4.exe not found at %UV4_PATH%
    echo Please update the UV4_PATH variable in this script.
    exit /b 1
)

echo [INFO] Building %PROJ_FILE%...
echo This process may take a while. Output is being logged to %LOG_FILE%.

:: Run Build
:: -b = Batch Build
:: -j0 = Suppress GUI
:: -o = Output log file
"%UV4_PATH%" -b "%PROJ_FILE%" -j0 -o "%LOG_FILE%"

:: Check Exit Code
:: 0 = No Errors or Warnings
:: 1 = Warnings Only
:: 2 = Errors
set BUILD_STATUS=%ERRORLEVEL%

if %BUILD_STATUS% EQU 0 (
    echo ----------------------------------------------------------------------
    echo [SUCCESS] Build Successful! (0 Errors, 0 Warnings)
    echo ----------------------------------------------------------------------
) else if %BUILD_STATUS% EQU 1 (
    echo ----------------------------------------------------------------------
    echo [WARNING] Build Completed with WARNINGS.
    echo ----------------------------------------------------------------------
) else (
    echo ----------------------------------------------------------------------
    echo [FAILURE] Build FAILED with ERRORS.
    echo ----------------------------------------------------------------------
)

:: Show the build log
if exist "%LOG_FILE%" type "%LOG_FILE%"

exit /b %BUILD_STATUS%
