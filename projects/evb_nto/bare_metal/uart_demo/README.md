# UART Demo - Daric EVB

This project provides a simple UART communication example for the Daric EVB (Evaluation Board). 

### Key Features:
- **Interactive Shell**: Implements a command-line interface on **UART0**.
- **Debug Trace**: System logs and debug information are output to **DUART**.
- **Modular Build**: Supports both Keil MDK (ARMClang) and GCC build environments.

### HAL Driver Implementation
This project is built using the **Daric SDK UART HAL driver**. It demonstrates the usage of the following key HAL APIs:
- `HAL_UART_Init()`: Initializes the UART peripheral with specified baud rate and frame format.
- `HAL_UART_Transmit()`: Sends data over UART in polling mode.
- `HAL_UART_Receive()`: Receives data from UART in polling mode (with timeout).
- `HAL_PINMAP_init()`: Configures the GPIO pins (PA3/PA4) for UART0 functionality.
### Global Macro Definitions
The project relies on several key macros defined in the build system (`uart_demo.uvprojx` for MDK or `project.cmake` for GCC) to enable features and configure the hardware:

| Macro | Description |
| :--- | :--- |
| `HAL_UART_MODULE_ENABLED` | Enables the UART HAL driver module. |
| `HAL_PINMAP_MODULE_ENABLED` | Enables the Pinmux/Pinmap HAL driver module. |
| `HAL_GPIO_MODULE_ENABLED` | Enables the GPIO HAL driver module. |
| `HAL_UDMA_MODULE_ENABLED` | Enables uDMA support for efficient data transfer. |
| `CONFIG_SOC_DARIC_NTO_A=1` | Targets the Daric NTO-A SoC revision. |
| `CONFIG_SUPPORT_800MFREQ` | Enables support for 800MHz CPU frequency. |
| `HAL_NVIC_MODULE_ENABLED` | Enables Nested Vectored Interrupt Controller support. |
| `HAL_ATIMER_MODULE_ENABLED` | Enables the Advanced Timer HAL module. |
| `CONFIG_PMIC_I2C_ID=1` | Sets the I2C ID (1) for PMIC communication. |
| `CONFIG_PMIC_I2C_SPEED=100` | Sets the I2C bus speed (100kHz) for PMIC communication. |
| `CONFIG_PMIC_I2C_ADDR=0x34` | Sets the I2C slave address (0x34) for the PMIC. |

These macros ensure that only the necessary drivers are compiled and that the hardware is correctly configured from startup.

## Shell Commands

The UART0 shell supports the following preset commands:
- `help`: Display a list of available commands.
- `hello`: Print a friendly greeting from the Daric chip.
- `version`: Show the current bare-metal demo version.

### Terminal Configuration

The project uses two serial ports for communication:

| Function | Port | Baud Rate | Configuration |
| :--- | :--- | :--- | :--- |
| **Interactive Shell** | UART0 | 115200 | 8N1, No Flow Control |
| **Debug Trace / Logs** | DUART | 921600 | 8N1, No Flow Control |

## Project Structure

- `src/`: Source files including `main.c`.
- `inc/`: Header files and configuration.
- `MDK-ARM/`: Keil MDK project files and build scripts.
- `GCC/`: CMake and GNU Arm Toolchain build configuration.

## Environment Setup

To build and flash this project, you need one of the following environments:

### 1. Keil MDK (MDK-ARM)
- **Tool**: [Keil MDK-ARM](https://www.keil.com/download/product/) (Version 5.x or higher).
- **Toolchain**: ARM Compiler 6 (armclang).

### 2. GNU Arm Toolchain (GCC)
- **Toolchain**: [GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (`arm-none-eabi-gcc`).
- **Build Tools**: [CMake](https://cmake.org/download/) and [Ninja](https://ninja-build.org/).
- **Shell**: Bash (e.g., Msys2, Git Bash, or native Linux).

## Build Methods

### Using Keil MDK (MDK-ARM)
Navigate to the `MDK-ARM` directory and run the build script:
```batch
cd MDK-ARM
.\build_mdk.bat -r
```
The output binaries will be generated in `MDK-ARM/bin/`.
- `-r`: Rebuild all.
- `-c`: Clean objects.
- `-p <PROJ_FILE>`: Specify a project file (optional).

### Using GNU Arm Toolchain (GCC)
Navigate to the `GCC/scripts` directory and run the build script:
```bash
cd GCC/scripts
./build.sh
```
The output binaries will be generated in `GCC/out/bin/`.

## Flash and Download

Once the project is built, you can flash the generated `.bin` file to the target board using J-Link.

### On Windows (Batch)
Use the `flash_mdk.bat` script located in `utilities/flash/`:
```batch
cd utilities/flash
.\flash_mdk.bat ..\..\projects\evb_nto\bare_metal\uart_demo\MDK-ARM\bin\uart_demo.bin
```
**Note**: If flashing `uart_demo.bin`, the script automatically defaults to the correct flash address `0x60020000`.

### On Linux or Bash (Shell)
Use the `jlink_flash_daric.sh` script:
```bash
cd utilities/flash
./jlink_flash_daric.sh ../../projects/evb_nto/bare_metal/uart_demo/MDK-ARM/bin/uart_demo.bin
```

### J-Link Configuration
Ensure `JLink.exe` (Windows) or `JLinkExe` (Linux) is in your system `PATH`. The scripts are designed to find J-Link automatically without hardcoded paths.
