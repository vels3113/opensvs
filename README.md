## About
OpenSVS is a desktop GUI for inspecting netgen JSON reports. It provides summary and a filterable table view for the reports.

## Run
```bash
# Launch with no file (blank summary)
./build/src/opensvs

# Launch and load a JSON report
./build/src/opensvs path/to/netgen_output.json
```

## Build guide
### Prerequisites (Ubuntu 22.04)
- Install toolchain and Qt6 dev packages:
  ```bash
  sudo apt-get update
  sudo apt-get install -y qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools cmake ninja-build
  ```
- netgen: build from source if not packaged for your distro. Build instructions: https://github.com/RTimothyEdwards/netgen?tab=readme-ov-file#building-netgen

### Build setup
Use the helper to configure with Ninja (Qt6 prefix defaults are in CMakeLists):

```bash
./configure.sh
cmake --build build
```

Pass extra args as needed, e.g.:
```bash
./configure.sh -DCMAKE_BUILD_TYPE=Debug
```

## Tests

```bash
ctest --test-dir build --output-on-failure
```
### Smoke tests (headless)
- Offscreen (no display):
  ```bash
  ./tests/smoke_offscreen.sh
  ```
- Force xcb (bypass Wayland warning):
  ```bash
  ./tests/smoke_xcb.sh
  ```
These create a temporary XDG runtime dir and capture stdout/stderr to /tmp logs, then show the log contents.
