#!/usr/bin/env bash
set -euo pipefail

# Smoke test: run opensvs in offscreen mode and print any warnings.

RDIR="/tmp/xdg-runtime-${USER}"
mkdir -p "${RDIR}"
chmod 700 "${RDIR}"

XDG_RUNTIME_DIR="${RDIR}" QT_QPA_PLATFORM=offscreen ./build/src/opensvs >/tmp/opensvs_offscreen.log 2>&1 &
pid=$!
sleep 1
kill "${pid}" >/dev/null 2>&1 || true
wait "${pid}" 2>/dev/null || true

cat /tmp/opensvs_offscreen.log
