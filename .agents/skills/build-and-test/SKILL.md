---
name: build-and-test
description: Build, compile, and run tests for the home_assistant C++ project. Use this when configured, building, or compiling CMake targets or running unit tests.
---

# Build and Test Skill

This skill provides step-by-step instructions on configuring, building, and running tests for the `home_assistant` core project.

## Project Structure & Presets
The project uses CMakePresets.json to define how it should be configured and built.

- **default Preset**: Configures a native debug build on the host machine (macOS). Used primarily for quick syntax validation or offline component tests that do not depend on Linux specific APIs.
- **cross-arm64 Preset**: Configures cross-compilation targeting `aarch64-linux-gnu` (Ubuntu Server ARM64) using the sysroot at `sysroots/ubuntu-arm64` and LLVM/Clang from Homebrew.

## How to Configure
Before building, run CMake configuration.

### For Cross-Compilation (Ubuntu VM Target)
```bash
cmake --preset cross-arm64
```
This generates the build directory at `build-arm64`.

### For Native Build (macOS Host)
```bash
cmake --preset default
```
This generates the build directory at `build`.

## How to Build
To compile the targets, run the following:

### Build for Target VM
```bash
cmake --build --preset cross-arm64
```
This produces the executable `build-arm64/home_assistant`.

### Build for Host (macOS)
```bash
cmake --build --preset default
```
This produces the executable `build/home_assistant`.

## Testing Instructions
1. Ensure unit tests are defined in CMakeLists.txt (e.g., using GTest or Catch2).
2. If building natively, run unit tests inside the `build/` directory using `ctest` or executing test binaries directly:
   ```bash
   cd build && ctest --output-on-failure
   ```
3. For cross-compiled tests, test binaries must be deployed to the Ubuntu VM and run there (refer to the `deploy-and-run` skill).
