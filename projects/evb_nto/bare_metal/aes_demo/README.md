# AES Demo - Daric EVB

This project provides an AES (Advanced Encryption Standard) cryptographic API demo for the Daric EVB (Evaluation Board). It demonstrates how to use the hardware-accelerated SCE (Security Control Engine) to perform various AES encryption and decryption operations in a bare-metal environment.

### Key Features:
- **Comprehensive Mode Support**: Implements functional verification for **ECB, CBC, CTR, CFB, and OFB** modes.
- **Multiple Key Sizes**: Supports **128-bit, 192-bit, and 256-bit** AES keys.
- **Hardware Acceleration**: Re-uses the SCE hardware for efficient cryptographic processing via the Low-Level API (LL API).
- **Automated Verification**: Automatically compares decryption results with original plaintext to ensure correctness.
- **Debug Trace**: Detailed test results and memory dumps are output to the **DUART** console.

### HAL Driver & LL API Implementation
This project utilizes the **Daric SDK SCE drivers** and the **Low-Level API (LL API)**. Key functions demonstrated include:
- `aes_encrypt_le()`: Hardware-accelerated AES encryption.
- `aes_decrypt_le()`: Hardware-accelerated AES decryption.

## Test Execution

Upon startup, the demo will automatically run a suite of AES tests. The results are printed to the DUART console.

### Terminal Configuration

The test results are output via the DUART:

| Function | Port | Baud Rate | Configuration |
| :--- | :--- | :--- | :--- |
| **Debug Trace / Logs** | DUART | 921600 | 8N1, No Flow Control |

## Project Structure

- `src/main.c`: Contains the AES test vectors and execution logic.
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
./jlink_flash_daric.sh ../../projects/evb_nto/bare_metal/aes_demo/GCC/out/aes_demo.bin
```

## J-Link Configuration
Ensure `JLinkExe` is in your system `PATH`.
