# Roadmap

## Phase 1: Core Foundation (Current)
- [x] Basic Logging Infrastructure (Boost.Log)
- [x] Service Architecture (HAService class)
- [x] Threading Model (Boost.Asio `io_context` with worker threads)
- [ ] Signal Handling (Graceful Shutdown)
- [ ] Watchdog / Heartbeat mechanism

## Phase 2: Configuration & State Management
- [ ] JSON/YAML Configuration Loader
- [ ] State Registry (In-memory database of device states)
- [ ] Persistence Layer (Save state to disk/DB)

## Phase 3: Device Integration
- [ ] Plugin/Module System structure
- [ ] Virtual Switch/Sensor implementations
- [ ] MQTT Client Integration
- [ ] HTTP API for external control

## Phase 4: Automation Engine
- [ ] Event Bus implementation (Pub/Sub)
- [ ] Rule Engine (Basic "If This Then That" logic)
- [ ] Scripting grouping (Lua/Python integration)

## Phase 5: User Interface (Future)
- [ ] Web Dashboard
- [ ] REST API Docs
