# numsim-core


## Building and options

```sh
cmake -S . -B build && cmake --build build && ctest --test-dir build
```

| Option | Default | |
|---|---|---|
| `NUMSIM_CORE_BUILD_TESTS` | ON if top level | GoogleTest suite, each test also under ASan+UBSan (`NUMSIM_SANITIZER_TESTS`) |
| `NUMSIM_CORE_BUILD_EXAMPLES` | ON if top level | |
| `NUMSIM_CORE_INSTALL` | ON | install + CMake package config |

The old unprefixed names (`BUILD_TESTS`, …) are still honoured with a deprecation warning.
CMake helpers come from [numsim-cmake](https://github.com/NumSim-Stack/numsim-cmake)
(`cmake/numsim_bootstrap.cmake`).

The library lives in `namespace numsim::core`; `numsim_core` remains as an alias.
Toolchain: C++23 with `std::expected`, i.e. GCC ≥ 13 or Clang ≥ 19.
