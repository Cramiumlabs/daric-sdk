:: ==============================================================================
:: JLink Firmware Download Script for Daric NTO under Windows
:: ==============================================================================
:: Description: Downloads firmware to Daric NTO chip via JLink V7.92
:: Author: robin.wan@crossbar-inc.com
:: ==============================================================================

@echo off
setlocal

:: Default Configuration
if "%JLINK_EXE%"=="" (
    where JLink.exe >nul 2>&1
    if %errorlevel% equ 0 (
        set "JLINK_EXE=JLink.exe"
    ) else (
        set "JLINK_EXE=C:\Program Files\SEGGER\JLink_V792h\JLink.exe"
    )
)
set "PROJ_DIR=%~dp0"
set "BIN_FILE=bin\BootROM.bin"
set "FLASH_ADDR=0x60000000"
set "DEVICE=DARIC_NTO"
set "JLINK_SCRIPT=flash.jlink"
set "JLINK_DEVICES_XML_PATH=JLinkDevices.xml"

:: Change to project directory
cd /d "%PROJ_DIR%"

:parse_args
if "%~1"=="" goto done_args
set "PARAM=%~1"
set "NEXT_PARAM=%~2"

if /i "%PARAM%"=="-b" (
    if not "%NEXT_PARAM%"=="" (
        set "BIN_FILE=%NEXT_PARAM%"
        shift
    )
    goto continue_args
)
if /i "%PARAM%"=="-a" (
    if not "%NEXT_PARAM%"=="" (
        set "FLASH_ADDR=%NEXT_PARAM%"
        shift
    )
    goto continue_args
)
if /i "%PARAM%"=="-j" (
    if not "%NEXT_PARAM%"=="" (
        set "JLINK_EXE=%NEXT_PARAM%"
        shift
    )
    goto continue_args
)

:: Positional logic
if /i "%PARAM:~0,2%"=="0x" (
    set "FLASH_ADDR=%PARAM%"
) else (
    set "BIN_FILE=%PARAM%"
)

:continue_args
shift
goto parse_args
:done_args

:: Smart defaulting for uart_demo address
set "TEST_UART=%BIN_FILE:uart_demo=%"
if not "%TEST_UART%"=="%BIN_FILE%" (
    if "%FLASH_ADDR%"=="0x60000000" (
        set "FLASH_ADDR=0x60020000"
        echo [INFO] Detected uart_demo, using default flash address 0x60020000
    )
)

:: Check JLink
where "%JLINK_EXE%" >nul 2>&1
if errorlevel 1 (
    if not exist "%JLINK_EXE%" (
        echo [ERROR] JLink.exe not found at %JLINK_EXE% or in PATH.
        exit /b 1
    )
)

:: Check Binary
if not exist "%BIN_FILE%" (
    echo [ERROR] Binary file not found: %BIN_FILE%
    exit /b 1
)

:: Create JLink Command File
echo [INFO] Generating J-Link Script...
(
echo Exec JLinkDevicesXMLPath="%PROJ_DIR%%JLINK_DEVICES_XML_PATH%"
echo device %DEVICE%
echo si SWD
echo speed 6000
echo connect
echo exec SetCompareMode=0
echo loadfile "%BIN_FILE%", %FLASH_ADDR%
echo r
echo g
echo exit
) > "%JLINK_SCRIPT%"

:: Run JLink
echo [INFO] Flashing firmware to %DEVICE% at %FLASH_ADDR% using J-Link V7.x...
"%JLINK_EXE%" -commandfile "%JLINK_SCRIPT%"

:: Cleanup
if exist "%JLINK_SCRIPT%" del "%JLINK_SCRIPT%"

echo [INFO] Flashing Process Finished.
endlocal
