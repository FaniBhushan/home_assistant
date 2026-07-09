# Phase 1 System Architecture

This document contains four layered architecture diagrams for the Phase 1 home assistant system,
from the whole-system view down to the internal agentic loop.

---

## Diagram 1 — Whole System Overview

> What processes exist and how do they connect?

```mermaid
graph TD
    subgraph USER ["User (MacBook)"]
        MIC["Microphone (audio in)"]
        SPK["Speakers (audio out)"]
    end

    subgraph PYAGENT ["Python Agent Process (macOS)"]
        WW["Wake Trigger (push-to-talk or wake word)"]
        ASR["Whisper ASR (audio → text)"]
        ORCH["Agent Orchestrator (agentic loop)"]
        TTS["Piper TTS (text → speech audio)"]
        OLLCLIENT["Ollama HTTP Client"]
        CPPHTTP["C++ HTTP Client (sensor tool calls)"]
    end

    subgraph CPPD ["C++ Core Daemon (Ubuntu Server ARM64 / UTM VM)"]
        MAIN["main.cpp (entry point)"]
        HASVC["HAService (io_context + thread pool)"]
        SIG["SignalHandler (SIGINT / SIGTERM)"]
        CFG["ConfigLoader (JSON/YAML)"]
        STATE["StateRegistry (in-memory room state)"]
        MOCK["MockSensorService (timer-driven fake data)"]
        CROW["CrowHttpServer (REST API)"]
        LOG["logger:: (Boost.Log rotating file)"]
        LOGFILE[("core.log")]
    end

    subgraph LLMSRV ["LLM Server (macOS, Metal GPU)"]
        OLLAMA["Ollama (llama3 / mistral)"]
    end

    subgraph WEATHER ["External (optional)"]
        WAPI["Weather API (wttr.in / open-meteo)"]
    end

    MIC -->|"raw audio"| WW
    WW -->|"triggered audio clip"| ASR
    ASR -->|"text transcript"| ORCH
    ORCH -->|"tool: get_sensor_data(room)"| CPPHTTP
    ORCH -->|"tool: get_weather()"| WAPI
    CPPHTTP -->|"HTTP GET /sensors/{room}"| CROW
    CROW -->|"reads"| STATE
    STATE -->|"JSON snapshot"| CROW
    CROW -->|"HTTP 200 JSON"| CPPHTTP
    CPPHTTP -->|"tool result"| ORCH
    WAPI -->|"weather JSON"| ORCH
    ORCH -->|"messages + tool schemas"| OLLCLIENT
    OLLCLIENT -->|"HTTP POST /api/chat"| OLLAMA
    OLLAMA -->|"tool_call JSON OR final text"| OLLCLIENT
    OLLCLIENT -->|"parsed response"| ORCH
    ORCH -->|"final answer text"| TTS
    TTS -->|"synthesized audio"| SPK
    MOCK -->|"writes fake data every 5s"| STATE
    CFG -->|"config values"| HASVC
    HASVC --> SIG
    HASVC --> CFG
    HASVC --> STATE
    HASVC --> MOCK
    HASVC --> CROW
    MAIN --> HASVC
    HASVC -.->|"log"| LOG
    CROW -.->|"log"| LOG
    MOCK -.->|"log"| LOG
    SIG -.->|"log"| LOG
    LOG --> LOGFILE
```

Three separate runtime processes — C++ daemon (VM), Python agent (macOS), Ollama (macOS Metal GPU)
— communicate entirely over HTTP. This makes each independently restartable and replaceable.
C++ never touches AI; Python never touches hardware.

---

## Diagram 2 — C++ Internal Services Architecture

> How does the C++ daemon organize its own services?

```mermaid
graph TD
    subgraph BOOT ["Bootstrap"]
        MAIN["main.cpp\nCalls logger::init()\nConstructs HAService\nCalls haService.run()"]
    end

    subgraph CORE ["Core — HAService"]
        IOCTX["boost::asio::io_context\n(event loop + task queue)"]
        WGUARD["executor_work_guard\n(keeps io_context alive when idle)"]
        TP["Thread Pool\n3x std::thread → io_context::run()"]
        POST["boost::asio::post()\n(enqueues tasks)"]
        IOCTX --> WGUARD
        IOCTX --> TP
        POST -->|"enqueues onto"| IOCTX
    end

    subgraph INIT ["Initialization Layer (startup, sequential)"]
        CFG["ConfigLoader\nReads config.json\nOutputs: port, intervals, room list"]
        SIG["SignalHandler\nboost::asio::signal_set\nSIGINT / SIGTERM\n→ posts shutdown task to io_context"]
    end

    subgraph DATA ["Data Layer (shared runtime state)"]
        STATE["StateRegistry\nstd::unordered_map\nroom → SensorSnapshot {temp, co2, humidity}\nProtected by std::mutex"]
        MOCK["MockSensorService\nboost::asio::steady_timer\nFires every 5s via post()\nWrites random-walk values to StateRegistry"]
    end

    subgraph API ["API Layer (network-facing)"]
        CROW["CrowHttpServer (Crow)\nGET /health\nGET /sensors/{room}\nGET /weather"]
    end

    subgraph LOGGING ["Shared Logging (logger:: namespace)"]
        LOG["logger::info / debug / warning / error\nBoost.Log severity logger\nRotating file: core.log\n10MB / file, 10 files max, auto_flush=true"]
    end

    MAIN -->|"1. init logger"| LOG
    MAIN -->|"2. construct + run"| CORE
    CORE -->|"3. load config"| CFG
    CFG -->|"port, intervals, room list"| STATE
    CFG -->|"port number"| CROW
    CORE -->|"4. register signals"| SIG
    SIG -->|"on signal: post(shutdown)"| POST
    CORE -->|"5. start timer loop"| MOCK
    MOCK -->|"mutex write every 5s"| STATE
    CORE -->|"6. start HTTP server"| CROW
    CROW -->|"mutex read on request"| STATE

    MAIN -.->|"log"| LOG
    CFG -.->|"log"| LOG
    SIG -.->|"log"| LOG
    MOCK -.->|"log"| LOG
    CROW -.->|"log"| LOG
```

`HAService` is the single owner and wiring point — it bootstraps services in a fixed order (1–6)
so nothing starts before its dependencies are ready. `StateRegistry` sits behind a `std::mutex`
because `MockSensorService` writes from a timer thread while `CrowHttpServer` reads from HTTP
request threads — these can be concurrent.

---

## Diagram 3 — Voice Command End-to-End Sequence

> In what exact order do data and messages flow for a single voice command?

```mermaid
sequenceDiagram
    actor User
    participant WW as Wake Trigger (Python)
    participant ASR as Whisper ASR (Python)
    participant ORCH as Agent Orchestrator (Python)
    participant OLLAMA as Ollama Server (macOS)
    participant CPP as CrowHttpServer (C++ VM)
    participant STATE as StateRegistry (C++)
    participant TTS as Piper TTS (Python)
    participant SPK as Speakers

    User->>WW: speaks "What is the bedroom temperature?"
    WW->>ASR: audio clip (WAV/PCM)
    Note over ASR: Whisper runs locally — no network call
    ASR->>ORCH: transcript: "What is the bedroom temperature?"

    Note over ORCH: Builds initial message list:<br/>[system_prompt, user_message]<br/>Attaches tool schemas JSON

    ORCH->>OLLAMA: HTTP POST /api/chat<br/>{ messages: [...], tools: [get_sensor_data, get_weather] }
    Note over OLLAMA: LLM sees question + tool menu.<br/>Decides it needs sensor data.
    OLLAMA-->>ORCH: { "tool_calls": [{ "name": "get_sensor_data", "arguments": { "room": "bedroom" } }] }

    Note over ORCH: Detects tool_call — NOT final text.<br/>Executes tool locally.

    ORCH->>CPP: HTTP GET /sensors/bedroom
    CPP->>STATE: mutex lock → lookup "bedroom"
    STATE-->>CPP: SensorSnapshot { temp: 22.5, co2: 620, humidity: 55 }
    CPP-->>ORCH: HTTP 200 { "room": "bedroom", "temp": 22.5, "co2": 620, "humidity": 55 }

    Note over ORCH: Appends tool result to history.<br/>History: [system, user, tool_call, tool_result]<br/>Sends back to Ollama.

    ORCH->>OLLAMA: HTTP POST /api/chat { messages: [system, user, tool_call, tool_result] }
    Note over OLLAMA: Now has the data it needed.<br/>Produces natural language answer.
    OLLAMA-->>ORCH: { "message": { "content": "The bedroom is 22.5°C, CO2 at 620 ppm, humidity 55%." } }

    Note over ORCH: No tool_call in response → final answer.

    ORCH->>TTS: "The bedroom is 22.5°C..."
    Note over TTS: Piper synthesizes locally — no network call
    TTS->>SPK: audio stream
    SPK->>User: spoken answer
```

Notice Ollama is called **twice**: first to decide what data is needed, second to produce the
answer once that data arrives. The C++ daemon is only involved for one short synchronous exchange.
Whisper and Piper never touch the network.

---

## Diagram 4 — Agentic Loop Deep Dive

> How does the AI decide what to do, and when to stop?

```mermaid
flowchart TD
    START(["Transcript received:\n'What is the bedroom temp?'"])

    BUILD["Build Message History\n───────────────────────────────\nmessages = [\n  { role: 'system',    content: 'You are a home assistant...' },\n  { role: 'user',      content: 'What is the bedroom temp?' }\n]\ntools = [ get_sensor_data schema, get_weather schema ]"]

    CALL_LLM["HTTP POST /api/chat → Ollama\n───────────────────────────────\nSend: { messages, tools }"]

    RECV["Receive Ollama Response"]

    DECIDE{{"What did the LLM return?"}}

    TC_BRANCH["BRANCH A — Tool Call\n───────────────────────────────\nLLM output:\n{\n  tool_calls: [{\n    name: 'get_sensor_data',\n    arguments: { room: 'bedroom' }\n  }]\n}"]

    EXEC_TOOL["Execute Tool Locally\n───────────────────────────────\nOrchestrator maps name → Python function.\nCalls: get_sensor_data('bedroom')\n→ HTTP GET /sensors/bedroom on C++ daemon"]

    RECV_RESULT["Receive Tool Result\n───────────────────────────────\n{ room: 'bedroom', temp: 22.5,\n  co2: 620, humidity: 55 }"]

    APPEND["Append to Message History\n───────────────────────────────\nmessages = [\n  { role: 'system',    content: '...' },\n  { role: 'user',      content: 'What is the bedroom temp?' },\n  { role: 'assistant', tool_calls: [get_sensor_data(bedroom)] },\n  { role: 'tool',      content: '{temp:22.5, co2:620}' }\n]\nHistory now has 4 messages."]

    LOOP_BACK(["↺ Loop back to Ollama\nwith updated history"])

    FA_BRANCH["BRANCH B — Final Answer\n───────────────────────────────\nLLM output:\n{\n  message: {\n    content: 'The bedroom is 22.5°C,\n    CO2 620 ppm, humidity 55%.'\n  }\n}\n(no tool_calls field)"]

    TTS["Send to Piper TTS\n───────────────────────────────\ntext → synthesized audio"]

    DONE(["Audio played to user — Loop ends"])

    START --> BUILD
    BUILD --> CALL_LLM
    CALL_LLM --> RECV
    RECV --> DECIDE

    DECIDE -->|"tool_call present"| TC_BRANCH
    DECIDE -->|"plain text content"| FA_BRANCH

    TC_BRANCH --> EXEC_TOOL
    EXEC_TOOL --> RECV_RESULT
    RECV_RESULT --> APPEND
    APPEND --> LOOP_BACK
    LOOP_BACK --> CALL_LLM

    FA_BRANCH --> TTS
    TTS --> DONE
```

### Understanding the Agentic Loop

An LLM is strictly text-in, text-out — it cannot call a function, read a sensor, or make an HTTP
request on its own. So you give it a **tool schema list** alongside the user's question: each
schema is a small JSON object describing a function name and its parameters, like a menu of
available capabilities. When the LLM realizes it needs data to answer, instead of guessing, it
outputs a specially formatted JSON block saying *"I want to call `get_sensor_data` with
`room = bedroom`"*. This is not code execution — it is just text that happens to look like JSON.

Your Python **orchestrator** is the trusted execution layer. It reads the LLM's output, detects
whether it's a tool call or a final answer, and if it's a tool call, maps the name to a real Python
function and actually executes it (making the HTTP call to C++). The LLM never touches your sensor
— it only expresses intent.

The key to multi-step reasoning is **message history**. The orchestrator does not start a new
conversation after each tool call — it appends both the LLM's tool call and the real result as new
messages in the same growing list, then sends the entire history back to Ollama. After one
round-trip the history is `[system, user_question, assistant_tool_call, tool_result]`. The LLM
now has context + data, so on the next call it can produce a grounded answer. If a question needs
two tools (e.g. sensor data *and* weather), the loop runs twice before producing a final answer.

The **exit condition** is simple: the loop stops when the LLM returns a message with a plain
`content` string and no `tool_calls` field. That's the final answer, ready for TTS. This pattern
is called **ReAct (Reason + Act)** or **function calling** depending on the framework, but the
mechanic is always the same: LLM produces intent → orchestrator executes → result fed back →
repeat until final answer.

**Example of growing message history across two tool calls:**

```
Round 1 — LLM decides it needs sensor data:
  messages = [
    { role: "system",    content: "You are a home assistant..." },
    { role: "user",      content: "What is the bedroom temp and today's weather?" }
  ]
  → LLM returns: tool_call get_sensor_data(bedroom)

Round 2 — After sensor result, LLM decides it also needs weather:
  messages = [
    { role: "system",    content: "..." },
    { role: "user",      content: "..." },
    { role: "assistant", tool_calls: [get_sensor_data(bedroom)] },
    { role: "tool",      content: '{"temp": 22.5, "co2": 620}' }
  ]
  → LLM returns: tool_call get_weather()

Round 3 — After weather result, LLM produces final answer:
  messages = [
    { role: "system",    content: "..." },
    { role: "user",      content: "..." },
    { role: "assistant", tool_calls: [get_sensor_data(bedroom)] },
    { role: "tool",      content: '{"temp": 22.5, "co2": 620}' },
    { role: "assistant", tool_calls: [get_weather()] },
    { role: "tool",      content: '{"condition": "Cloudy", "temp_outside": 14}' }
  ]
  → LLM returns: "The bedroom is 22.5°C. Outside it's cloudy at 14°C."
```

---

---

## Potential Challenge 1 — Audio Input Sanitization

Raw microphone audio fed directly into Whisper produces poor transcription results in realistic
home environments. The following preprocessing stages are needed between wake word detection and ASR.

### Required preprocessing pipeline

```
Mic → [Resample to 16kHz mono 16-bit PCM] → [AEC] → [Noise Suppression] → [VAD gate] → Whisper
```

| Stage | Problem it solves | Recommended library |
|---|---|---|
| Resample + convert | Whisper requires 16kHz mono 16-bit PCM. Mics often capture 44.1kHz stereo | `libsamplerate`, `soundfile`, or SoX |
| AEC (Acoustic Echo Cancellation) | TTS speaker output leaks back into the mic. Without AEC, Whisper transcribes its own responses | WebRTC APM (C++), `webrtcvad` (Python) |
| Noise suppression | Background noise (appliances, HVAC) causes hallucinated transcripts | RNNoise (Mozilla) — lightweight, runs well on ARM |
| VAD (Voice Activity Detection) | Silence and trailing noise fed to Whisper increases latency and error rate | Silero VAD, WebRTC VAD |
| AGC (Automatic Gain Control) | Quiet speech from across a room drops below Whisper's reliable threshold | WebRTC APM or `pyaudio` gain stage |

### Key notes

- **AEC is the most critical** for this architecture. Because TTS playback and mic input share the same
  room, every spoken response is re-captured. AEC requires a reference signal from the playback
  device so it can subtract the known output from the mic input.
- VAD prevents the wake word clip from containing long silence tails, which inflate Whisper's
  processing time significantly.
- This preprocessing layer sits entirely within the Python voice pipeline — no C++ changes required
  for Phase 1. Can be added as a `AudioPreprocessor` stage between `WakeWordDetector` and `WhisperASR`.

---

## Potential Challenge 2 — Linux Satellite Audio Devices

Deploying the voice pipeline to a Linux machine with external (satellite) mic/speaker devices
introduces a distinct set of challenges compared to a single macOS machine.

### Audio subsystem choice

Linux's audio stack is layered: ALSA at the kernel level, PulseAudio or PipeWire on top.

| Stack | Notes |
|---|---|
| ALSA alone | Lowest latency, no automatic mixing or routing. Manual device selection required. |
| PulseAudio | Stable, widely supported. Being phased out on newer Ubuntu. |
| **PipeWire** (recommended) | Unified graph handling USB, Bluetooth, HDMI, and ALSA. Default on Ubuntu 22.04+. Supports virtual nodes and filter chains for routing. |

### Stable device addressing

USB mic and speaker device indices (`/dev/snd/pcmC1D0`) are not stable across reboots or
hot-plug cycles. Selection must use PipeWire/PulseAudio node names or ALSA card names, not
numeric indices. This requires a device registry config (see below).

### Bluetooth satellite devices

- **A2DP** profile: high-quality audio output only — cannot be used for mic input.
- **HFP/HSP** profile: provides both mic and speaker but at degraded quality (8–16kHz). Not suitable for Whisper without resampling.
- SBC codec adds ~150–200ms latency to TTS playback.
- BT connections drop and renegotiate codec unpredictably — the audio session must handle reconnection gracefully without crashing the pipeline.

### Hot-plug handling

Devices connect and disconnect at runtime. The Python voice pipeline must subscribe to PipeWire
or udev events to detect new devices and reassign audio source/sink assignments dynamically,
rather than hard-coding device handles at startup.

### Far-field microphone arrays

Devices such as ReSpeaker or Matrix Creator expose multi-channel ALSA input (e.g., 6 channels).
The processed beamformed audio is typically on a specific channel (channel 0 or a dedicated
capture stream). This requires driver setup and a custom ALSA/PipeWire capture configuration —
the pipeline cannot treat these as a standard single-channel mic.

### Multi-room routing

If satellite devices exist in multiple rooms, the orchestrator must route TTS output to the
speaker closest to the room where the wake word was detected. This is non-trivial and requires:

1. A **device registry** config mapping room names to PipeWire node names.
2. Orchestrator logic to select the target output device per response.
3. Possibly a dedicated `AudioRouter` component in the Python agent process.

### Summary table

| Challenge | Severity | Mitigation |
|---|---|---|
| Stable device addressing | Medium | Use PipeWire node names in config; avoid ALSA indices |
| AEC with satellite speaker | High | WebRTC APM with reference signal from playback device |
| Bluetooth latency + dropout | Medium | Prefer USB/wired for satellite audio; BT for speakers only |
| Hot-plug handling | Medium | PipeWire event subscription or udev watcher in Python |
| Multi-room routing | High | Room-to-device map in config + `AudioRouter` in orchestrator |
| Far-field mic arrays | Medium | Per-device ALSA config + driver setup; document per device model |

### Architectural addition required

A **device registry** — a config-driven map of room names to audio device identifiers — is needed
before satellite deployment. It belongs either as a section in `config.json` (read by `ConfigLoader`
and exposed via a REST endpoint) or as a standalone config file consumed by the Python agent.

```json
{
  "audio_devices": {
    "bedroom":  { "mic": "alsa_input.usb-bedroom-mic",  "speaker": "alsa_output.usb-bedroom-spk" },
    "kitchen":  { "mic": "alsa_input.usb-kitchen-mic",  "speaker": "bluez_output.kitchen-bt-spk" },
    "living_room": { "mic": "alsa_input.usb-lr-mic",   "speaker": "alsa_output.hdmi-lr-tv" }
  }
}
```

---

## Reading Guide

| Diagram | Zoom Level | Primary Question Answered |
|---|---|---|
| 1 — System Overview | Whole system, all processes | What processes exist and how do they connect? |
| 2 — C++ Internals | Inside the daemon | How does the C++ daemon organize its own services? |
| 3 — Voice Sequence | One complete request, chronological | In what exact order do messages and data move? |
| 4 — Agentic Loop | One sub-component, iterative logic | How does the AI decide what to do and when to stop? |

## Phase 1 C++ Services Summary

| Service | Responsibility | Key Dependency |
|---|---|---|
| `HAService` | Owns io_context, thread pool, wires all services | Boost.Asio |
| `SignalHandler` | SIGINT/SIGTERM → post clean shutdown | Boost.Asio signal_set |
| `ConfigLoader` | Reads config.json at startup | nlohmann/json or yaml-cpp |
| `StateRegistry` | In-memory room → sensor snapshot map | std::mutex |
| `MockSensorService` | Timer-driven random-walk sensor data | Boost.Asio steady_timer |
| `CrowHttpServer` | REST API: /health, /sensors/{room}, /weather | Crow (header-only) |
| `logger::` | Shared logging sink (already implemented) | Boost.Log |
