#!/usr/bin/env bash
set -euo pipefail

# Helper to configure the build tree with Ninja by default.
# Usage: ./configure.sh [additional cmake args]

cmake -S . -B build -G Ninja "$@"
