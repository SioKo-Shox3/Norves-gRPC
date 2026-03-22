# Norves-gRPC

**Game-oriented general-purpose network library wrapping gRPC for C++20.**

Part of the [NorvesLib](https://github.com/SioKo-Shox3/NorvesLib) ecosystem.

## Overview

Norves-gRPC provides a high-level, game-friendly abstraction over gRPC, handling:
- **Server / Client lifecycle** -- start, stop, reconnect
- **Session management** -- connection tracking, heartbeat (planned)
- **Message protocol** -- routing, serialization (planned)
- **Game API** -- lobby, matchmaking, state sync (planned)

## Requirements

- **C++20** compatible compiler (MSVC 19.29+, GCC 10+, Clang 12+)
- **CMake** 3.20+
- **vcpkg** (recommended for dependency management)

## Quick Start

```bash
# Clone
git clone https://github.com/SioKo-Shox3/Norves-gRPC.git
cd Norves-gRPC

# Configure (with vcpkg)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build

# Run tests
cd build && ctest --output-on-failure

# Run echo example
./build/examples/echo_server &
./build/examples/echo_client
```

## Project Structure

```
include/norves/    Public headers
src/               Implementation
proto/             Protocol Buffer definitions
tests/             Unit tests (Google Test)
examples/          Example programs
```

## License

MIT License -- see [LICENSE](LICENSE) for details.
