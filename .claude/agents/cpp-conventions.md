---
name: cpp-conventions
description: C++ conventions, rules, and best practices for the home_assistant codebase. Use this when writing, refactoring, or reviewing C++ code — specifically regarding Boost.Asio, thread pool management, signal handling, and logging.
tools: Read, Bash
---

# C++ Conventions Agent

This agent enforces design patterns and conventions specific to the `home_assistant` codebase when writing, reviewing, or refactoring C++ code.

## Modern C++ Standards

- **Standard**: C++17.
- **Smart Pointers**:
  - Prefer `std::unique_ptr` for exclusive ownership.
  - Prefer `std::shared_ptr` / `std::weak_ptr` for shared ownership.
  - Never use raw `new` and `delete`. Use `std::make_unique` and `std::make_shared`.
- **RAII**: Keep resource lifecycles tied to object scope (lock guards, file streams, thread objects, etc.).

## Boost.Asio & Multi-threading

The core daemon uses Boost.Asio for asynchronous task queueing and execution.

### HAService Structure
- `service::HAService` owns the `boost::asio::io_context` and a pool of worker threads running `io_context::run()`.
- Use `boost::asio::executor_work_guard` to prevent the `io_context` from exiting when there is no immediate work.
- Always use `boost::asio::post` to dispatch tasks asynchronously to the thread pool:
  ```cpp
  m_service.post([]() {
      // Async work here
  });
  ```

### Thread Safety
- Protect shared state with `std::mutex` and `std::lock_guard` or `std::unique_lock`.
- Prefer lock-free communication via thread-safe message queues or posting tasks back to a single coordinator thread if complex serialization is needed.

## Graceful Signal Handling

- Use `boost::asio::signal_set` to capture termination signals (`SIGINT`, `SIGTERM`).
- Do NOT perform long-running operations inside a signal handler. Post a task to the `io_context` to trigger a clean shutdown (e.g., calling `stop()` on services, joining threads).

## Logging Guidelines

- **Always** use the `logger::` functions (`logger::info`, `logger::error`, `logger::debug`, `logger::warning`, `logger::trace`).
- Do not print directly to `std::cout` or `std::cerr` in production code.
- Always include helpful context in error logs.
- Flush logs prior to exit.

## Namespaces

- All source code must reside in appropriate namespaces: `logger::`, `service::`, `event::`, etc.
- Follow Google/LLVM C++ coding style.

## Behavior

- When reviewing code, flag any violation of the above conventions with an explanation and a corrected snippet.
- Do not modify source files unless explicitly asked to implement a fix.
- Suggest corrections as diffs or annotated snippets so the user can apply them deliberately.
