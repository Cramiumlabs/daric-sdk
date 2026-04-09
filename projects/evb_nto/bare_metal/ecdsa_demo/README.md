# ECDSA Demo - Daric EVB

This project provides an ECDSA (Elliptic Curve Digital Signature Algorithm) demo for the Daric EVB (Evaluation Board). It demonstrates how to use the hardware-accelerated SCE (Security Control Engine) to perform elliptic curve cryptographic operations (Keygen, Sign, Verify) in a bare-metal environment.

### Key Features:
- **Curve Support**: Supports industry-standard curves **SECP256K1** and **SECP256R1**.
- **Operation Suite**: Covers the full ECDSA lifecycle:
    - **Key Generation**: Deriving public keys from private keys.
    - **Signing**: Generating ECDSA signatures for message digests.
    - **Verification**: Validating signatures against public keys and digests.
- **Hardware Acceleration**: Utilizes the SCE hardware for efficient PKE (Public Key Engine) operations.
- **Automated Verification**: Automatically compares results with NIST/RFC reference vectors.
- **Debug Trace**: Test results and logs are output to the **DUART** console.

### API Implementation
This project utilizes the **Daric SDK SCE drivers**. Key functions demonstrated include:
- `ecdsa_get_pubkey()`: Derives a 65-byte uncompressed public key.
- `ecdsa_sign_digest()`: Signs a message digest with a private key and nonce.
- `ecdsa_verify()` / `ecdsa_verify_ext()`: Verifies the validity of an ECDSA signature.

## Test Execution

Upon startup, the demo will automatically run a suite of tests for both SECP256K1 and SECP256R1. The results are printed to the DUART console.

### Terminal Configuration

The test results are output via the DUART:

| Function | Port | Baud Rate | Configuration |
| :--- | :--- | :--- | :--- |
| **Debug Trace / Logs** | DUART | 921600 | 8N1, No Flow Control |

## Project Structure

- `src/main.c`: Contains the ECDSA test vectors and execution logic.
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
./jlink_flash_daric.sh ../../projects/evb_nto/bare_metal/ecdsa_demo/GCC/out/ecdsa_demo.bin
```

## J-Link Configuration
Ensure `JLinkExe` is in your system `PATH`.
