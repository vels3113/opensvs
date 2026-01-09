#!/usr/bin/env bash
set -euo pipefail

# Helper to configure the build tree with Ninja by default.
# Usage: ./configure.sh [additional cmake args]

options=${@:---preset=release}

cmake -S . -G Ninja ${options}
