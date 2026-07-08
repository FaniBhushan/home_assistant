# Roadmap

## Phase 1: Core Foundation & Software Prototype (M1 Pro Host)
*Goal: Build the C++ core daemon base and test the complete local voice agent loop on the MacBook using mock data.*

*   **Core C++ Foundation**
    *   [x] Basic Logging Infrastructure (Boost.Log)
    *   [x] Service Architecture (HAService class)
    *   [x] Threading Model (Boost.Asio `io_context` with worker threads)
    *   [ ] Signal Handling (Graceful Shutdown)
    *   [ ] Watchdog / Heartbeat mechanism
*   **Configuration & State Management**
    *   [ ] JSON/YAML Configuration Loader
    *   [ ] State Registry (In-memory database of device states)
    *   [ ] Persistence Layer (Save state to disk/DB)
*   **Local AI / Speech Pipeline**
    *   [ ] Local Ollama Setup (macOS Metal GPU)
    *   [ ] Local Whisper (ASR) & Piper (TTS) wrappers
    *   [ ] Python Agent Loop (Whisper -> Ollama -> Tool Calls -> Piper)
    *   [ ] Mock/Virtual Sensor API endpoints in C++ (Temp/CO₂)

## Phase 2: Device Integration & Edge Satellites (iPad/ESP32)
*Goal: Connect physical inputs/outputs and the iPad wall display without migrating the central server.*

*   **Device & IPC Integration**
    *   [ ] Plugin/Module System structure
    *   [ ] Virtual Switch/Sensor implementations
    *   [ ] HTTP API / WebSocket for external control (HAService)
    *   [ ] MQTT Client Integration
*   **Edge Satellite & Kiosk Deployment**
    *   [ ] iPad Kiosk Display (Guided Access & Battery Management Smart Plug)
    *   [ ] ESP32-S3 Voice Satellite (microWakeWord & ESPHome)
    *   [ ] Sensirion SCD4x CO₂ Sensor Integration
    *   [ ] Zigbee Environmental Sensors & USB Coordinator Integration

## Phase 3: Automation Engine & Production Migration (Dedicated SFF)
*Goal: Deploy the automation rules engine and migrate the central server to a 24/7 dedicated Proxmox host.*

*   **Automation Engine**
    *   [ ] Event Bus implementation (Pub/Sub)
    *   [ ] Rule Engine (Basic "If This Then That" logic)
    *   [ ] Scripting integration (Lua/Python)
*   **Production Server Migration**
    *   [ ] Proxmox VE Server Setup
    *   [ ] HAOS VM & Ollama LXC container with PCIe GPU Passthrough
    *   [ ] Production Database & Daemon Migration
*   **User Interface & Docs**
    *   [ ] Production Web Dashboard
    *   [ ] REST API Docs

