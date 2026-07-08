# Home Assistant Core — Project Rules and Guidelines

This file outlines behavior constraints, coding standards, and architectural patterns for developing the `home_assistant` core daemon.

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
4. **Error Handling**: Use RAII for resource management. Avoid raw pointers. Prefer `std::unique_ptr` and `std::shared_ptr`. Correctly capture exceptions at the service borders.
5. **Local-First & Offline**: Avoid cloud dependencies. All data, logic, and models must run locally.
6. **Modularity & Decoupling**: Keep components isolated (C++ handles system telemetry and IO, Python handles LLM orchestration, iPad/Web client handles the view layer).

---

## Coding Standards & Style

1. **Style Guide**: Follow Google/LLVM C++ coding style.
2. **Standard**: C++17.
3. **Namespaces**: Source code must reside in appropriate namespaces (e.g., `logger::`, `service::`, `event::`).
4. **Smart Pointers**:
   - Prefer `std::unique_ptr` for exclusive ownership.
   - Prefer `std::shared_ptr` / `std::weak_ptr` for shared ownership.
   - Never use raw `new` and `delete`. Use `std::make_unique` and `std::make_shared`.
5. **RAII**: Keep resource lifecycles tied to object scope (lock guards, file streams, thread objects, etc.).
6. **Logging**:
   - Use the `logger::` wrapper (defined in `src/logger/logger.hpp`) rather than raw `std::cout` or direct `BOOST_LOG_SEV`.
   - Logging levels: `trace`, `debug`, `info`, `warning`, `error`.
   - Ensure logs are flushed and not lost during crash/termination.

---

## Boost.Asio & Multi-threading

The core daemon uses Boost.Asio for asynchronous task queueing and execution.

- The `service::HAService` owns the `boost::asio::io_context` and a pool of worker threads running `io_context::run()`.
- Use `boost::asio::executor_work_guard` to prevent the `io_context` from exiting when there is no immediate work.
- Always use `boost::asio::post` to dispatch tasks asynchronously to the thread pool.
- Protect shared state with `std::mutex` and `std::lock_guard` or `std::unique_lock`.
- Use `boost::asio::signal_set` for termination signals. Do NOT perform long-running operations inside a signal handler — post a task to the `io_context` to trigger a clean shutdown.

---

## Building and Compiling

**CMake Presets** (defined in `CMakePresets.json`):

- **default**: Native debug build on the host machine (macOS). Used for quick syntax validation.
  ```bash
  cmake --preset default
  cmake --build --preset default
  ```
- **cross-arm64**: Cross-compilation targeting `aarch64-linux-gnu` (Ubuntu Server ARM64). Uses `toolchain/linux-aarch64.cmake` and sysroot at `sysroots/ubuntu-arm64`.
  ```bash
  cmake --preset cross-arm64
  cmake --build --preset cross-arm64
  ```

Do not alter `toolchainFile` or compilers unless resolving cross-compilation errors.

---

## Deployment Workflow

Target VM details:
- **VM Username**: `bhufani`
- **VM Target Path**: `/home/bhufani/home_assistant/`
- **Systemd Service**: `home_assistant.service`

Steps:
1. Transfer binary via SCP: `scp build-arm64/home_assistant bhufani@<VM_IP>:/home/bhufani/home_assistant/home_assistant`
2. Restart service: `ssh bhufani@<VM_IP> "sudo systemctl restart home_assistant.service"`
3. Tail logs: `ssh bhufani@<VM_IP> "journalctl -u home_assistant.service -f -n 100"`

Log file path on VM: `/home/bhufani/home_assistant/logs/core.log`

---

## System Interface Guidelines

- **HTTP REST**: Synchronous, request-response operations (e.g., fetching static config or querying current sensor state).
- **WebSockets**: Real-time, bidirectional state streaming (e.g., voice streaming, live sensor telemetry, UI updates).
- **MQTT**: Asynchronous pub/sub messaging across multiple physical devices.

---

## Collaboration & Code Generation Rules

1. **Explicit Implementation Instruction Required**: Do not write, modify, or delete project source code unless explicitly asked to implement it. Do not assume or run modifying tools preemptively.
2. **Interactive Algorithmic Suggestions**: When coding tasks are requested, suggest clear, step-by-step algorithms and logical designs first. The user will review these and explicitly decide whether to write the code themselves or ask you to implement it.
3. **Planning & Discussion**: Prioritize clarifying requirements, outlining design files, and asking for user feedback before any code modification.
