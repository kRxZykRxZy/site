# Pi Guardian

Lightweight C-based Raspberry Pi infrastructure guardian. Runs a web dashboard on port 81, includes PWA support, local authentication, service monitoring/recovery, notifications, and an optional AI decision layer.

## Build

```sh
make
sudo ./pi-guardian
```

Open `http://PI_IP:81` and sign in.

Default credentials are embedded in the executable as requested. **Change them before exposing Guardian beyond a trusted LAN.**

## Install

```sh
sudo ./install.sh
```

The installer builds the daemon and registers `pi-guardian.service`.

## Design

The daemon is split into monitoring, service actions, HTTP/authentication, AI, and notification modules. AI output is advisory and validated against a small allow-list; arbitrary shell commands are never executed from model output.
