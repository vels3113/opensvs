
## Build setup (Ninja default)
Use the helper to configure with Ninja (Qt6 prefix defaults are in CMakeLists):

```bash
./configure.sh
cmake --build build
```

Pass extra args as needed, e.g.:
```bash
./configure.sh -DCMAKE_BUILD_TYPE=Debug
```

## Smoke tests (headless/WSLg-friendly)
- Offscreen (no display):
  ```bash
  ./smoke_offscreen.sh
  ```
- Force xcb (bypass Wayland warning):
  ```bash
  ./smoke_xcb.sh
  ```
These create a temporary XDG runtime dir and capture stdout/stderr to /tmp logs, then show the log contents.
