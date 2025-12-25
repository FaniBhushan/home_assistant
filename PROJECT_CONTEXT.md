# Project Context – Home AI Assistant

## Goal
Build a local, AI-based home voice assistant with:
- Offline-first AI
- Voice interaction
- Sensor integration (temp, CO₂)
- UI display
- Android-based deployment later

## Phases
### Phase 1 – Linux-first (current)
- Platform: Ubuntu Server (ARM64) on UTM (MacBook M1)d
- Purpose: Core system + AI pipeline
- Status:
  - SSH working
  - systemd service running
  - No UI yet

### Phase 2 – Android realism (future)
- Platform: ARM SBC (RK3588)
- Purpose: Android system integration + UI

## Architecture Principles
- Core logic runs as a headless service
- UI is a client, not the core
- AI models are modular and replaceable
- CPU-first, NPU later

## Decisions Made
- Start on Linux, not Android
- Avoid emulators for audio/AI
- Use systemd for lifecycle management

## Next Steps
- Add REST API to core service
- Integrate ASR
- Integrate LLM
