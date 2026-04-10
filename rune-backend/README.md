# Rune Backend — C++20 + SQLite + pthreads

A threaded C++ backend node for the Rune System.

## Dependencies (Windows)

```powershell
winget install --id Kitware.CMake     -e
winget install --id Ninja-build.Ninja -e
winget install --id LLVM.LLVM         -e   # clang-cl, or use MSVC
```

SQLite3 is either found via `find_package(SQLite3)` or compiled from
a vendored single-file amalgamation placed at `vendor/sqlite3.c` +
`vendor/sqlite3.h` (download from https://sqlite.org/amalgamation.html).

## Build

```powershell
cd rune-backend
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run

```powershell
.\build\rune.exe rune.db
```

Press **ENTER** to gracefully stop the node threads.

## What it does

- Opens (or creates) `rune.db` with the full schema.
- Seeds three default runes on first run.
- Starts 2 background worker threads that log a heartbeat every 5 s.
