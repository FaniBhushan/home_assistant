# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- `HAService` class for managing the main application life-cycle.
- `ROADMAP.md`, `CHANGELOG.md`, and `TODO.md` documentation files.

### Changed
- Refactored `HAService` to use `boost::asio::io_context` with a pool of generic worker threads instead of specialized thread pools.
- Fixed namespace and syntax errors in `src/logger/Logger.hpp` (Boost.Log integration).
- Updated `main.cpp` (implied) to initialize logger and service correctly.

### Fixed
- Resolved undeclared identifiers `keywords`, `expr`, `logging` in Logger compilation units.
