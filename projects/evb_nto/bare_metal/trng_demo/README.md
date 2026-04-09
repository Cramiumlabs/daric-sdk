# TRNG Demo - Daric EVB

This project provides a TRNG (True Random Number Generator) cryptographic API demo for the Daric EVB (Evaluation Board). It demonstrates how to use the hardware-accelerated SCE (Security Control Engine) to generate true random numbers in a bare-metal environment.

### Key Features:
- **Hardware-Based Randomness**: Generates true random numbers using the hardware TRNG peripheral within the SCE.
- **Continuous Generation**: Demonstrates the ability to produce multiple blocks of random data.
- **Configurable Rounds**: Configured to run 20 rounds of generation by default.
- **Automated Verification**: Checks for basic error states and ensures the TRNG is functional.
- **Debug Trace**: Test results and random data hex dumps are output to the **DUART** console.

### HAL Driver & LL API Implementation
This project utilizes the **Daric SDK SCE drivers** and the **Low-Level API (LL API)**. Key functions demonstrated include:
- `rng_init()`: Initializes the TRNG peripheral and its continuous context.
- `rng_buffer()`: Generates a buffer of random numbers (unit: words/uint32_t).

## Test Execution

Upon startup, the demo will automatically run 20 rounds of TRNG tests, each producing 32 bytes (8 words) of random data. The results are printed to the DUART console.

### Terminal Configuration

The test results are output via the DUART:

| Function | Port | Baud Rate | Configuration |
| :--- | :--- | :--- | :--- |
| **Debug Trace / Logs** | DUART | 921600 | 8N1, No Flow Control |

## Project Structure

- `src/main.c`: Contains the TRNG demo execution logic and configuration.
- `GCC/`: CMake and GNU Arm Toolchain build configuration.

## Environment Setup

To build and flash this project, you need the following:
- **Toolchain**: [GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (`arm-none-eabi-gcc`).
- **Build Tools**: [CMake](https://cmake.org/download/) and [Ninja](https://ninja-build.org/).
- **Shell**: Bash environment.

## Build Methods

### Using GNU Arm Toolchain (GCC)
Navigate to the `GCC/scripts` directory and run the build script:
```bash
cd GCC/scripts
./build.sh
```
The output binaries will be generated in `GCC/out/`.

## Flash and Download

Once built, flash the generated `.bin` file to the target board (default address `0x60020000`).

### On Linux or Bash (Shell)
Use the `jlink_flash_daric.sh` script:
```bash
cd utilities/flash
./jlink_flash_daric.sh ../../projects/evb_nto/bare_metal/trng_demo/GCC/out/trng_demo.bin
```

## J-Link Configuration
Ensure `JLinkExe` is in your system `PATH`.
