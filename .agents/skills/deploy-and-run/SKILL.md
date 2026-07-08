---
name: deploy-and-run
description: Deploy the cross-compiled home_assistant binary to the target UTM VM (Ubuntu Server) and manage the systemd daemon. Use this when copying binaries, starting/stopping the daemon, or inspecting live logs on the target.
---

# Deploy and Run Skill

This skill outlines the process for deploying the cross-compiled C++ binary to the target ARM64 Ubuntu Server (running in UTM) and managing its systemd service lifecycle.

## Target Environment Details
- **VM Username**: `bhufani`
- **VM Target Path**: `/home/bhufani/home_assistant/`
- **Systemd Service**: `home_assistant.service`

## Step 1: Locating or Determining VM IP
The UTM VM must be running and have an active network connection.
- Check active connections or `~/.ssh/config` for hosts configured for the VM.
- If not configured, identify the VM's IP address (e.g. from the UTM console running `ip a`).
- Set environment variable `VM_IP` or SSH alias `home-assistant-vm` to streamline deployment.

## Step 2: Transferring the Binary
To deploy the cross-compiled binary to the target VM:

```bash
scp build-arm64/home_assistant bhufani@<VM_IP>:/home/bhufani/home_assistant/home_assistant
```
*(Replace `<VM_IP>` with the actual IP address or SSH alias of the VM).*

If deploying configuration files or other assets (e.g. scripts):
```bash
rsync -avz --exclude="build*" --exclude=".git*" ./ bhufani@<VM_IP>:/home/bhufani/home_assistant/
```

## Step 3: Controlling the Service
The daemon lifecycle is managed via `systemd`. Connect to the VM via SSH to run systemctl commands.

### Restart the Service
```bash
ssh bhufani@<VM_IP> "sudo systemctl restart home_assistant.service"
```

### Stop the Service
```bash
ssh bhufani@<VM_IP> "sudo systemctl stop home_assistant.service"
```

### Start the Service
```bash
ssh bhufani@<VM_IP> "sudo systemctl start home_assistant.service"
```

### Enable/Disable Service on Boot
```bash
ssh bhufani@<VM_IP> "sudo systemctl enable home_assistant.service"
```

## Step 4: Tailoring and Inspecting Logs
To view the output and logs of the running daemon:

### Using journalctl (Recommended)
```bash
ssh bhufani@<VM_IP> "journalctl -u home_assistant.service -f -n 100"
```

### Inspecting Log Files
The log path is defined in `logger.hpp` as `/home/bhufani/home_assistant/logs/core.log`.
```bash
ssh bhufani@<VM_IP> "tail -f /home/bhufani/home_assistant/logs/core.log"
```
