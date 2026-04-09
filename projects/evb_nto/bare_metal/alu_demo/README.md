# ALU Division Demo - Daric EVB

This project provides an ALU (Arithmetic Logic Unit) big number division demo for the Daric EVB (Evaluation Board). It demonstrates how to use the hardware-accelerated SCE (Security Control Engine) to perform large number division operations in a bare-metal environment.

### Key Features:
- **Big Number Division**: Supports division of numbers up to 4096 bits.
- **Hardware Acceleration**: Utilizes the SCE hardware for efficient arithmetic processing.
- **Automated Verification**: Automatically compares calculated Quotient (Q) and Remainder (R) with expected values to ensure correctness.
- **Debug Trace**: Test results and error details are output to the **DUART** console.

### API Implementation
This project utilizes the **Daric SDK SCE drivers** and the **Low-Level API (LL API)**. Key functions demonstrated include:
- `bndivision_le()`: Hardware-accelerated floor division for big numbers in little-endian format.

## Test Execution

Upon startup, the demo will automatically run a suite of division tests with varying bit lengths (256-bit to 4096-bit). The results are printed to the DUART console.

### Terminal Configuration

The test results are output via the DUART:

| Function | Port | Baud Rate | Configuration |
| :--- | :--- | :--- | :--- |
| **Debug Trace / Logs** | DUART | 921600 | 8N1, No Flow Control |

## Project Structure

- `src/main.c`: Contains the division test vectors and execution logic.
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
./jlink_flash_daric.sh ../../projects/evb_nto/bare_metal/alu_demo/GCC/out/alu_demo.bin
```

## J-Link Configuration
Ensure `JLinkExe` is in your system `PATH`.
