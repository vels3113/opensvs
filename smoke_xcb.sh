#!/usr/bin/env bash
set -euo pipefail

# Smoke test: run opensvs forcing xcb platform and print any warnings.

RDIR="/tmp/xdg-runtime-${USER}"
mkdir -p "${RDIR}"
chmod 700 "${RDIR}"

XDG_RUNTIME_DIR="${RDIR}" QT_QPA_PLATFORM=xcb ./build/src/opensvs >/tmp/opensvs_xcb.log 2>&1 &
pid=$!
sleep 2
kill "${pid}" >/dev/null 2>&1 || true
wait "${pid}" 2>/dev/null || true

cat /tmp/opensvs_xcb.log
