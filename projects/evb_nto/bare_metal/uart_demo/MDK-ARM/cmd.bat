@echo off

set "dir_path=bin"

if not exist "%dir_path%" (
    echo Directory does not exist. Creating it...
    mkdir "%dir_path%"
    if errorlevel 1 (
        echo Failed to create directory.
        exit /b 1
    )
    echo Directory created successfully.
) 

set "FROMELF=%LOCALAPPDATA%\Keil_v5\ARM\ARMCLANG\bin\fromelf.exe"

if not exist "%FROMELF%" (
    echo Error: fromelf.exe not found at %FROMELF%
    exit /b 1
)

"%FROMELF%" --text -c -o bin\uart_demo.asm Objects\uart_demo.axf
"%FROMELF%" --bin -o bin\uart_demo.bin Objects\uart_demo.axf
copy Listings\uart_demo.map bin\uart_demo.map
