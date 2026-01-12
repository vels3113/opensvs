- Paths after install (defaults):
  - Qt6 qmake: `/usr/lib/qt6/bin/qmake6`
  - Qt6 plugins: `/usr/lib/x86_64-linux-gnu/qt6/plugins`
  - Qt6 CMake configs: `/usr/lib/x86_64-linux-gnu/cmake/Qt6`
  - netgen: wherever you install it (verify with `netgen -?`); set `NETGEN_BIN` or use full path if not on PATH
  - Optional helpers: `jq` at `/usr/bin/jq`

## Headless / WSLg tips
- Use offscreen if display is unavailable: `QT_QPA_PLATFORM=offscreen ./build/src/opensvs ...`
- WSLg warnings about runtime dir: create a 0700 runtime dir first, e.g. `RDIR=/tmp/xdg-runtime-$USER; mkdir -p "$RDIR"; chmod 700 "$RDIR"; XDG_RUNTIME_DIR="$RDIR" QT_QPA_PLATFORM=offscreen ./build/src/opensvs ...`

## Troubleshooting
- `qt.qpa.plugin: Could not load the Qt platform plugin "xcb"`: install Qt xcb libs or run with `QT_QPA_PLATFORM=offscreen`
- `Qt6::Widgets not found` during configure: set `-DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6`
- netgen not on PATH: invoke with full path (e.g., `/usr/local/bin/netgen`) or export `NETGEN_BIN`

## Netgen build
- Before building and installing netgen, install these packages on Ubuntu:
```bash
sudo apt install libx11-dev libxrender1 libxrender-dev libxcb1 libxaw7-dev libx11-xcb-dev libcairo2 libcairo2-dev tcl8.6 tcl8.6-dev tk8.6 tk8.6-dev libxpm4 libxpm-dev
```

## ccache tips
`ccache` doesn't work with relative paths on cmake configuration or build in Github Actions runner.

## clang-tidy
`.clang-tidy` checks are tuned to be compatible with Qt builds.