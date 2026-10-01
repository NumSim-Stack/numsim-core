# numsim-core


## Building

```sh
cmake -S . -B build && cmake --build build && ctest --test-dir build
```

| Option | Default |
|---|---|
| `NUMSIM_CORE_BUILD_TESTS` | ON if top level (each test also runs under ASan+UBSan, `NUMSIM_SANITIZER_TESTS`) |
| `NUMSIM_CORE_BUILD_EXAMPLES` | ON if top level |
| `NUMSIM_CORE_INSTALL` | ON |

CMake helpers come from [numsim-cmake](https://github.com/NumSim-Stack/numsim-cmake).
Toolchain: C++23 with `std::expected`, i.e. GCC ≥ 13 or Clang ≥ 19.
