#!/bin/sh
set -eu
ROOT=/opt/pi-guardian
STATE=/var/lib/pi-guardian
TLS="$STATE/tls"
mkdir -p "$ROOT/web" "$TLS"
if command -v apt-get >/dev/null 2>&1; then
  apt-get update
  DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential libcurl4-openssl-dev libssl-dev ca-certificates openssl
fi
if [ ! -s "$TLS/server.key" ] || [ ! -s "$TLS/server.crt" ]; then
  HOST_IP=$(hostname -I | awk '{print $1}')
  HOST=$(hostname -s 2>/dev/null || echo pi-guardian)
  openssl req -x509 -newkey rsa:2048 -sha256 -nodes -days 825 \
    -keyout "$TLS/server.key" -out "$TLS/server.crt" \
    -subj "/CN=$HOST" \
    -addext "subjectAltName=IP:${HOST_IP:-127.0.0.1},IP:127.0.0.1,DNS:$HOST"
  chmod 600 "$TLS/server.key"
  chmod 644 "$TLS/server.crt"
fi
make clean all
install -m 755 pi-guardian "$ROOT/pi-guardian"
install -m 644 web/index.html web/manifest.json web/sw.js web/icon.svg "$ROOT/web/"
install -m 644 systemd/pi-guardian.service /etc/systemd/system/pi-guardian.service
systemctl daemon-reload
systemctl enable --now pi-guardian.service
systemctl restart pi-guardian.service
echo "Pi Guardian HTTPS: https://$(hostname -I | awk '{print $1}'):81"
echo "The first visit may show a certificate warning because this is a local self-signed certificate."
echo "For reliable phone Web Push, install/trust the generated certificate on the phone or use a certificate from a trusted CA."
