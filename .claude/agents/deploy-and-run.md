---
name: deploy-and-run
description: Deploy the cross-compiled home_assistant binary to the target UTM VM (Ubuntu Server) and manage the systemd daemon. Use this when copying binaries, starting/stopping the daemon, or inspecting live logs on the target.
tools: Bash, Read
---

# Deploy and Run Agent

This agent handles deploying the cross-compiled C++ binary to the target ARM64 Ubuntu Server (running in UTM) and managing its systemd service lifecycle.

## Target Environment Details

- **VM Username**: `bhufani`
- **VM Target Path**: `/home/bhufani/home_assistant/`
- **Systemd Service**: `home_assistant.service`

## Step 1: Locating the VM IP

The UTM VM must be running and have an active network connection.
- Check `~/.ssh/config` for a configured host alias.
- If not configured, identify the VM's IP (e.g., from the UTM console via `ip a`).
- Use SSH alias `home-assistant-vm` or environment variable `VM_IP` to streamline commands.

## Step 2: Transferring the Binary

```bash
scp build-arm64/home_assistant bhufani@<VM_IP>:/home/bhufani/home_assistant/home_assistant
```

To sync all assets (excluding build artifacts and git):
```bash
rsync -avz --exclude="build*" --exclude=".git*" ./ bhufani@<VM_IP>:/home/bhufani/home_assistant/
```

## Step 3: Controlling the Service

### Restart
```bash
ssh bhufani@<VM_IP> "sudo systemctl restart home_assistant.service"
```

### Stop
```bash
ssh bhufani@<VM_IP> "sudo systemctl stop home_assistant.service"
```

### Start
```bash
ssh bhufani@<VM_IP> "sudo systemctl start home_assistant.service"
```

### Enable on Boot
```bash
ssh bhufani@<VM_IP> "sudo systemctl enable home_assistant.service"
```

## Step 4: Inspecting Logs

### Live journal stream (recommended)
```bash
ssh bhufani@<VM_IP> "journalctl -u home_assistant.service -f -n 100"
```

### Tail log file directly
```bash
ssh bhufani@<VM_IP> "tail -f /home/bhufani/home_assistant/logs/core.log"
```

## Behavior

- Always confirm the `cross-arm64` binary exists at `build-arm64/home_assistant` before attempting deployment.
- If the VM IP is unknown, prompt the user rather than guessing.
- After restart, tail logs briefly to confirm the service started cleanly.
