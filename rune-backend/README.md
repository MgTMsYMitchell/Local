# Rune Backend — C++20 · SQLite · cpp-httplib · nlohmann/json · pthreads

A threaded C++ backend node for the Rune System.  
All C++ dependencies are fetched automatically at CMake configure time via
`FetchContent` — **no manual installs needed beyond the build toolchain**.

## Cloned dependency sources

| Library | Source | Version |
|---------|--------|---------|
| [sqlite/sqlite](https://github.com/sqlite/sqlite) | amalgamation (C) | 3.45.2 |
| [yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib) | header-only C++ | v0.15.3 |
| [nlohmann/json](https://github.com/nlohmann/json) | header-only C++ | v3.11.3 |

## Windows toolchain (one-time)

```powershell
winget install --id Kitware.CMake        -e   # >= 3.20
winget install --id Ninja-build.Ninja    -e
winget install --id Microsoft.VisualStudio.2022.BuildTools -e
# or: winget install --id LLVM.LLVM -e  (for clang-cl)
```

## Build

```powershell
cd rune-backend
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

CMake will clone SQLite, cpp-httplib, and nlohmann/json into `build/_deps/`
the first time it runs (requires internet access on the build machine).

## Run

```powershell
.\build\rune.exe              # uses rune.db on port 7070 (defaults)
.\build\rune.exe mydata.db 8080   # custom db path + port
```

Press **ENTER** to gracefully shut down the node threads and HTTP server.

## Endpoints (served by the C++ process itself)

| Method | Path | Description |
|--------|------|-------------|
| GET | `/health` | JSON: status + row counts |
| GET | `/runes`  | JSON array of all runes |
| POST | `/shutdown` | Graceful stop via HTTP |

