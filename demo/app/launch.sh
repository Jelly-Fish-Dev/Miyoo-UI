#!/bin/sh
# Miyoo-UI demo launcher for OnionOS (/mnt/SDCARD/App/MiyooUIDemo/launch.sh)
cd "$(dirname "$0")" || exit 1

LOG=log.txt
[ -f "$LOG" ] && mv -f "$LOG" log.prev.txt

exec ./bin/muidemo >> "$LOG" 2>&1
