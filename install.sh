#!/bin/sh
set -eu
ROOT=/opt/pi-guardian
mkdir -p "$ROOT/web" /var/lib/pi-guardian
if command -v apt-get >/dev/null 2>&1; then
  apt-get update
  DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential libcurl4-openssl-dev libssl-dev ca-certificates
fi
make clean all
install -m 755 pi-guardian "$ROOT/pi-guardian"
install -m 644 web/index.html web/manifest.json web/sw.js web/icon.svg "$ROOT/web/"
install -m 644 systemd/pi-guardian.service /etc/systemd/system/pi-guardian.service
systemctl daemon-reload
systemctl enable --now pi-guardian.service
echo "Pi Guardian is running on http://$(hostname -I | awk '{print $1}'):81"
echo "Background Web Push requires an HTTPS origin (or localhost)."
