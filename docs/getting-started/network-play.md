# Playing on a network

## 1. Start the server

On the machine that hosts the game:

```sh
./r-type_server -p 4242
```

It prints the address to give to the other players:

```text
Server listening on UDP port 4242
Players on your network can join with: -h 192.168.1.67 -p 4242
```

If the line is missing (no network found), find the address yourself: `ipconfig` on Windows (IPv4 address),
`ip addr` on Linux.

## 2. Start the clients

A machine that only plays does not need the sources or a compiler: copy the whole `bin` folder of a build
(`build/bin/` on Linux, `out\windows\bin\Release\` on Windows) with the executable, the libraries, `assets`,
`data`, `plugins`, `python` and `scripts`. The client and the server must come from the same commit.

On every player's machine, including the host if it plays:

```sh
./r-type_client -h 192.168.1.67 -p 4242
```

`-p` can be left out when the server uses the default port 4242. Without `-h`, the client uses
`serverAddress` from `rtype_settings.json`.

## 3. Firewall

The server receives UDP on its port. If clients stay on "CONNECTING...":

| System | What to do |
|---|---|
| Windows | Accept the firewall popup the first time the server starts, for **private networks**. Otherwise: Windows Defender Firewall, Allow an app, add `r-type_server.exe` |
| Linux (ufw) | `sudo ufw allow 4242/udp` |
| School or public Wi-Fi | Client isolation may block traffic between machines: use a phone hotspot or a cable switch |

## 4. WSL

A server started inside WSL 2 is behind a virtual network by default. Either run the Windows build of the
server, or enable mirrored networking so that WSL uses the Windows address:

```ini
# %UserProfile%\.wslconfig
[wsl2]
networkingMode=mirrored
```

Run `wsl --shutdown` after the change. In mirrored mode the Hyper-V firewall also filters WSL; allow the port
from an administrator PowerShell:

```powershell
New-NetFirewallRule -DisplayName "R-Type UDP 4242" -Direction Inbound -Protocol UDP -LocalPort 4242 -Action Allow
New-NetFirewallHyperVRule -Name RTypeUDP4242 -DisplayName "R-Type UDP 4242 (WSL)" -Direction Inbound `
  -VMCreatorId '{40E0AC32-46A5-438A-A0B2-2B479E8F2E90}' -Protocol UDP -LocalPorts 4242
```

## 5. Outside a local network

Use the local address printed by the server, never the public one given by "what is my IP" websites. When
machines are not on the same network, or a school network isolates them, put them on a phone hotspot or on a
virtual network such as Tailscale, and use the address it gives (`100.x.x.x`) with `-h`.

## Checks

| Symptom | Cause |
|---|---|
| "CONNECTING..." then "SERVER UNREACHABLE" | Wrong address, firewall, or server not started |
| "SERVER UNREACHABLE OR CONNECTION LOST" right away, server running | Client and server built from different versions (the server answers `REJECT` version mismatch): rebuild both from the same commit |
| Lag spikes | Wi-Fi interference: prefer a cable for the host |
| Room list empty on one machine only | It is connected to another server (check `-h`) |
