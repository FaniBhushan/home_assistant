# Todo List

## High Priority
- [ ] **Test Thread Pool**: Verify that `HAService` accurately distributes tasks across the worker threads.
- [ ] **Signal Handling**: Implement `boost::asio::signal_set` in `HAService` to handle SIGINT/SIGTERM for graceful shutdown.
- [ ] **Logger Configuration**: Allow logging level and file path to be configurable via arguments or config file.

## Core Features
- [ ] **Event System**: Define a concrete `Event` struct/class to pass data around, not just `std::function<void()>`.
- [ ] **Config Parser**: Add a dependency (like `nlohmann/json` or `yaml-cpp`) to read service configuration.
- [ ] **Unit Tests**: Set up Google Test (GTest) or Catch2 framework for testing the service and logger.

## Tech Debt / Cleanup
- [ ] **CMake Refactor**: Ensure `FindBoost` is working correctly without warnings (Policy CMP0167).
- [ ] **Code Style**: Set up `.clang-format` to enforce consistent style across `src/`.

## Ideas
- [ ] Consider separating the `HAService` into a library vs executable structure for easier testing.
- [ ] Investigate `boost::di` or similar for dependency injection if the service grows complex.
