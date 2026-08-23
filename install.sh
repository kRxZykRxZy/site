#!/bin/sh
set -eu
ROOT=/opt/pi-guardian
mkdir -p "$ROOT/web"
make clean all
install -m 755 pi-guardian "$ROOT/pi-guardian"
install -m 644 web/index.html web/manifest.json web/sw.js "$ROOT/web/"
install -m 644 systemd/pi-guardian.service /etc/systemd/system/pi-guardian.service
systemctl daemon-reload
systemctl enable --now pi-guardian.service
echo "Pi Guardian is running on http://$(hostname -I | awk '{print $1}'):81"
