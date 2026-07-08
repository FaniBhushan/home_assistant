---
name: build-and-test
description: Build, compile, and run tests for the home_assistant C++ project. Use this when configuring, building, or compiling CMake targets or running unit tests.
tools: Bash, Read
---

# Build and Test Agent

This agent handles configuring, building, and running tests for the `home_assistant` core project.

## Project Structure & Presets

The project uses `CMakePresets.json` to define configuration and build profiles.

- **default Preset**: Native debug build on the host machine (macOS). Used primarily for quick syntax validation or offline component tests.
- **cross-arm64 Preset**: Cross-compilation targeting `aarch64-linux-gnu` (Ubuntu Server ARM64) using the sysroot at `sysroots/ubuntu-arm64` and LLVM/Clang from Homebrew.

## How to Configure

### Cross-Compilation (Ubuntu VM Target)
```bash
cmake --preset cross-arm64
```
Generates build directory at `build-arm64`.

### Native Build (macOS Host)
```bash
cmake --preset default
```
Generates build directory at `build`.

## How to Build

### Build for Target VM
```bash
cmake --build --preset cross-arm64
```
Produces `build-arm64/home_assistant`.

### Build for Host (macOS)
```bash
cmake --build --preset default
```
Produces `build/home_assistant`.

## Testing Instructions

1. Ensure unit tests are defined in `CMakeLists.txt` (e.g., using GTest or Catch2).
2. For native tests, run inside `build/`:
   ```bash
   cd build && ctest --output-on-failure
   ```
3. For cross-compiled tests, deploy binaries to the Ubuntu VM first (refer to the `deploy-and-run` agent).

## Behavior

- Do not alter `toolchainFile` or compiler paths unless resolving a cross-compilation error.
- Report build errors verbatim and suggest targeted fixes rather than broad CMake restructuring.
- When a build fails, check `CMakeLists.txt` and `CMakePresets.json` before suggesting changes to source files.
