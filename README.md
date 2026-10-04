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
The library lives in `namespace numsim::core`; `numsim_core` remains as an alias.
Toolchain: C++23 with `std::expected`, i.e. GCC ≥ 13 or Clang ≥ 19.

## JSON input (`numsim-core::json`)

`#include <numsim-core/json/json_parameter_converter.h>` and link
`numsim-core::json`. `json_to_parameters(json, schema, params[, registry], options)`
fills a `parameter_handler` from a JSON object and validates it against an
`input_parameter_controller` schema.

- numsim-core does not bring a JSON library: link one yourself
  (e.g. `nlohmann_json::nlohmann_json`). `json_adapter<JsonType>` fits
  nlohmann::json; specialise it for another library.
- Numbers are checked: a fraction, a negative value for an unsigned type, or
  a value out of range is an error. Lists must be JSON arrays.
- Every input error is a `json_conversion_error` (an `std::invalid_argument`)
  whose `path()` names the place in the document, e.g. `materials[1].poisson`.
- Keys not in the schema warn by default (`unknown_key_policy::error` for strict
  input); `options.on_warning` collects warnings instead of printing them.
- `json_reader_registry::add<T>(fn)` adds a reader for a type,
  `add_for_key<T>(key, fn)` one for a single parameter.
