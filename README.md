# Tracing C++

A header-only C++20 library for structured logging and tracing.

## Header-only library

Copy [tracing.h](tracing.h) to your project and include it in your C++ files.

## Usage

```cpp
// example.cc
#include "tracing.h"

int main() {
  tracing::Builder builder = tracing::Builder::from_env("CPP_LOG");
  builder.init();

  tracing::info("Hello, {}!", "world");
  tracing::debug("1 + 1 = {}", 1 + 1);
}
```

```bash
$ c++ -std=c++20 -o example example.cc; CPP_LOG="debug" ./example
[DEBUG example.cc:8:38] Hello, world!
[DEBUG example.cc:9:37] 1 + 1 = 2
```
