#!/usr/bin/env bash
# Compile + upload the tunnel firmware, pausing SteinerGate's background
# logger (steinergate-watchdog.service) around the upload since it holds
# /dev/ttyUSB0 open and blocks esptool's bootloader handshake otherwise.
#
# Usage: ./upload.sh [sketch_dir] [port] [fqbn]
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKETCH_DIR="${1:-$SCRIPT_DIR/FT_V9.1_SD_Ignore}"
PORT="${2:-/dev/ttyUSB0}"
FQBN="${3:-esp32:esp32:esp32}"
SERVICE="steinergate-watchdog"

echo "==> Compiling $SKETCH_DIR ..."
arduino-cli compile --fqbn "$FQBN" "$SKETCH_DIR"

WAS_ACTIVE=0
if systemctl is-active --quiet "$SERVICE"; then
  WAS_ACTIVE=1
fi

restore_service() {
  if [ "$WAS_ACTIVE" -eq 1 ]; then
    echo "==> Restarting $SERVICE ..."
    systemctl start "$SERVICE"
    sleep 1
    if systemctl is-active --quiet "$SERVICE"; then
      echo "==> $SERVICE is running again."
    else
      echo "!! $SERVICE did NOT come back up — check 'systemctl status $SERVICE' and 'journalctl -u $SERVICE'" >&2
    fi
  fi
}
trap restore_service EXIT

if [ "$WAS_ACTIVE" -eq 1 ]; then
  echo "==> Stopping $SERVICE to free $PORT ..."
  systemctl stop "$SERVICE"
  sleep 1
else
  echo "==> $SERVICE was not running — nothing to pause."
fi

echo "==> Uploading to $PORT ..."
arduino-cli upload -p "$PORT" --fqbn "$FQBN" "$SKETCH_DIR"

echo "==> Upload complete."
