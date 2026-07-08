# Project Rules and Guidelines - Home Assistant Core

This file outlines the behavior constraints, coding standards, and architectural patterns for developing the `home_assistant` core daemon.

## Project Context
The `home_assistant` core is a headless, event-driven C++ system designed to monitor weather and indoor environment quality (sensor integration, voice control, ASR/TTS/LLM processing) on edge devices.

- **Current OS / Platform**: Ubuntu Server (ARM64) running inside UTM (M1 MacBook).
- **Build System**: CMake (minimum 3.20) with Ninja generator and CMakePresets.json.
- **Language**: C++17, utilizing standard libraries, Boost.Log, and Boost.Asio for async/threading operations.
- **Service Lifecycle**: Managed via systemd service on the Ubuntu Server.

---

## Architectural Principles
1. **Headless Core**: Core logic runs as a headless service (`home_assistant` daemon). UI displays or external widgets should act as separate clients communicating via REST, WebSocket, or MQTT.
2. **CPU-First execution**: AI pipelines (ASR, LLM, TTS) must be optimized for CPU-first execution (e.g., llama.cpp / whisper.cpp), keeping NPU/GPU optimization for later stages.
3. **Graceful Shutdown**: The core service must cleanly catch termination signals (SIGINT, SIGTERM) and shutdown threads, flush logger logs, and cleanup resources.
4. **Error Handling**: Use RAII (Resource Acquisition Is Initialization) for resource management. Avoid raw pointers. Prefer `std::unique_ptr` and `std::shared_ptr`. Correctly capture exceptions at the service borders.

---

## Coding Standards & Style
1. **Style Guide**: Follow Google/LLVM C++ coding style.
2. **Namespaces**:
   - Source code must reside in appropriate namespaces (e.g., `logger::`, `service::`, `event::`).
3. **Logging**:
   - Use the `logger::` wrapper (defined in `src/logger/logger.hpp`) rather than raw `std::cout` or direct `BOOST_LOG_SEV`.
   - Logging levels: `trace`, `debug`, `info`, `warning`, `error`.
   - Ensure logs are flushed and not lost during crash/termination.
4. **Boost.Asio**:
   - Prefer passing functions or using `boost::asio::post` to delegate work to the thread pool (`HAService`).
   - Use `boost::asio::executor_work_guard` to keep the context running until explicitly stopped.

---

## Building and Compiling
1. **CMake Presets**:
   - Run configure and build commands utilizing `CMakePresets.json`.
   - Native build: `cmake --preset default`
   - Cross-compilation (targeting ARM64 Ubuntu VM): `cmake --preset cross-arm64`
2. **Toolchain**:
   - Cross-compilation utilizes `toolchain/linux-aarch64.cmake` with target LLVM/Clang from Homebrew on macOS, pointing to a sysroot at `sysroots/ubuntu-arm64`.
   - Do not alter `toolchainFile` or compilers unless resolving cross-compilation errors.

---

## Deployment Workflow
- Binaries built via `cross-arm64` preset must be deployed to the Ubuntu Server inside UTM.
- Deployment involves:
  1. Synchronizing/transferring the binary and any assets via SSH/SCP.
  2. Starting/restarting the `home_assistant.service` systemd unit on the VM.
  3. Tailoring logs using `journalctl -u home_assistant.service -f`.

---

## Collaboration & Code Generation Rules
1. **Explicit Implementation Instruction Required**: Do not write, modify, or delete project source code unless explicitly asked to implement it. Do not assume or run modifying tools preemptively.
2. **Interactive Algorithmic Suggestions**: When coding tasks are requested, suggest clear, step-by-step algorithms and logical designs first. The user will review these and explicitly decide whether to write the code themselves or ask the agent to implement it.
3. **Planning & Discussion**: Prioritize clarifying requirements, outlining design files, and asking for user feedback before any code modification.
