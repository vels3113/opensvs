
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
