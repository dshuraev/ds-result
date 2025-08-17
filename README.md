# `ds-result` – Tiny `std::expected` Helpers for Modern C++

Tiny, dependency-free helpers around `std::expected` that make integrating other
codebases more elegant and less verbose.

❌ Without it, every call site needs boilerplate conversions  
✅ With it, you can just write `ds::Ok()` or `ds::Err(code)`

This library was born out of integrating **Vulkan** into a project
with exceptions disabled. Adapters were needed to unify `vk::Result` and
other APIs into a single `std::expected`-based interface.

## Features

- **Ergonomics**: `ds::Ok()`, `ds::Ok(value)`, `ds::Err(code)` cover common cases
- **Generic**: works with any success type; supports `int32_t`-convertible and `enum` error codes
- **Header-only**: everything is inlined
- **No exceptions**: does not throw; if the wrapped type constructor is `noexcept`, all helpers are too

## Motivation & Comparison

Why this library, when `std::expected` already exists?

- **Vanilla `std::expected`**

  - Great for modeling success/error.
  - But constructing results repeatedly is verbose:

    ```cpp
    return std::unexpected(ErrorInfo{1, 42});
    // or
    return std::expected<int, ErrorInfo>(std::unexpect, ErrorInfo{1, 42});
    ```

    vs.

    ```cpp

    return ds::Err(42, Facilities::vulkan);
    ```

- **Exceptions**

  - Not always available (e.g., embedded, game engines, performance-critical systems).
  - Exceptions can hide control flow; `std::expected` keeps it explicit.

- **Other libraries (e.g. `tl::expected`)**
  - Useful before C++23, but now redundant.
  - This helper builds on the standard type with minimal footprint.

In short:  
`ds-result` does not replace `std::expected` — it _reduces the ceremony_ of using it in real-world code, especially when bridging APIs that expose enums or integral error codes.

## When to Use

- You don’t want exceptions
- You need to make various APIs consistent
- You want to use `std::expected` semantics

## When _Not_ to Use

- Your project already uses exceptions
- You don’t use error codes
- You don’t want errors to “bubble up”

---

## Requirements

- Modern compiler with **C++23** support (`-std=c++2b`)

---

## Usage

```cpp
// assuming checkout in ds_result directory
#include "ds_result/result.hpp"
```

### Types

```cpp
// holds information about an error
struct ErrorInfo {
  // identifies the originator, e.g. Vulkan, GLFW, server, etc.
  // defaults to 0 (generic facility)
  std::int32_t facility {0};
  // error code specific to your facility
  std::int32_t error_code {0};
};

// success/failure result
template <typename T>
using Result = std::expected<T, ErrorInfo>;

// void-value result; carries only error info on failure
using Status = Result<std::monostate>;
```

### Functions

Provided helpers:

```cpp
// success constructors
ds::Status    ds::Ok();                // void success
ds::Result<T> ds::Ok(T&& value);       // forwards value
ds::Result<T> ds::Ok(Args&&... args);  // in-place constructor

// error constructors (E/F must be integral or enum)
ds::Status    ds::Err(E value, F facility = F{0});
ds::Result<T> ds::Err<T>(E value, F facility = F{0});
```

### Examples

Declaring facilities (error originators):

```cpp
enum class Facilities : std::int32_t { general = 0, vulkan = 1 };

// converting Vulkan results
[[nodiscard]] constexpr inline ds::Status to_result(vk::Result r) noexcept {
    return (r == vk::Result::eSuccess) ? ds::Ok() : ds::Err(r, Facilities::vulkan);
}

template <typename T>
[[nodiscard]] constexpr inline ds::Result<T> to_result(vk::ResultValue<T>&& rv) noexcept {
    return (rv.result == vk::Result::eSuccess)
        ? ds::Ok(std::move(rv.value))
        : ds::Err<T>(rv.result, Facilities::vulkan);
}

```

This way you can wrap Vulkan call sites with `to_result` and work with `std::expected`.
You can add more `to_result` adapters for other libraries as needed.

## Building & Testing

A minimal test suite is provided in `./test/test.cpp`:

- `Status` success/failure
- `Result<T>` with values
- Enum/integral
- Move semantics

```sh
# initialize submodules
make submodule
# build & run tests
make test
```
