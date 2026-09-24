# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Summary]

StormByte System is the C++26 process and environment layer of the StormByte suite.

It depends on [StormByte Base 2.0.0](https://github.com/StormBytePP/StormByte/releases/tag/2.0.0) or newer. This repository is not Base, Buffer, Config, Crypto, Database, Logger, Multimedia, Network or String.

Spawn children with piped stdin/stdout/stderr, chain them, suspend/resume, and expand environment strings. POSIX and Windows stay behind one API.

From 2.0.0, original System sources are dual-licensed: GNU Lesser General Public License v3.0 or later, or a commercial license from the copyright holder. That change does not cover other StormByte modules or third-party material under `thirdparty/` (including bundled StormByte Base).

If you landed here from a release link and have not read the tree:

- What this module is, how to build it, and short examples: [README.md](https://github.com/StormBytePP/StormByte-System/blob/master/README.md)
- License: dual license LGPL-3.0-or-later or commercial, [LICENSE](https://github.com/StormBytePP/StormByte-System/blob/master/LICENSE)

## [Unreleased]

### Changed

- **License:** original System sources are dual-licensed LGPL-3.0-or-later or commercial. Third-party trees under `thirdparty/` keep their own licenses. Neither license grants patent rights.
- Declared Base requirement is [StormByte Base 2.0.0](https://github.com/StormBytePP/StormByte/releases/tag/2.0.0) or newer.
- Doxygen (`ENABLE_DOC`) resolves Base headers via `INCLUDE_PATH` and skips `thirdparty`.

[Unreleased]: https://github.com/StormBytePP/StormByte-System/compare/1.1.0...HEAD

## [1.1.0] - 2026-09-13

### Changed

- **Public process behavior**
    - Ported System exception messages to the `StormByte::Component` format and added `ProcessCreationError` for process creation failures.
    - Added a public timed `Wait(std::chrono::milliseconds)` overload; the existing untimed overload remains unchanged.
- **Process pipeline internals**
    - Moved forwarding into the internal `Pipe` abstraction while preserving buffered and future output.
    - Moved Process state into the private process implementation header, reducing public-header ABI exposure.
- **Dependencies and build configuration**
    - Updated the StormByte/base dependency to 1.1.0.
    - Switched Windows release optimization handling to CMake interprocedural optimization without duplicate manual compiler/linker flags.

### Fixed

- **Pipe and process lifecycle**
    - Pipe construction now checks platform errors, normalizes UNIX descriptors, and moved pipes invalidate their source endpoints.
    - Pipe reads, writes, polling, EOF handling, and descriptor binding now distinguish interruption, EOF, and failure.
    - Process pipeline forwarding retains pipe ownership independently of Process lifetime, supports safe reconnection, and cancels without cross-thread descriptor closure.
    - Process waiting no longer deadlocks on downstream backpressure; lifecycle joins handle unexpected thread errors without escaping `noexcept` cleanup paths.
    - Direct writes, interrupted waits, and Windows wait failures now preserve error and ownership semantics.
- **Process startup and platform handling**
    - UNIX startup reports `execvp` failures to the parent and throws the appropriate exception eagerly.
    - Windows startup quotes command-line arguments and distinguishes missing executables from other creation failures.
    - Environment expansion handles missing home directories safely, expands only leading `~` paths, and grows Windows buffers as needed.
- **Regression coverage**
    - Added coverage for pipelines, process moves, signal termination, interrupted waits, descriptor reuse, consumer exit, direct write failures, and oversized Windows environment expansion.
- **CI portability**
    - Fixed Windows-only Process implementation initialization.
    - Made UNIX process tests resolve utilities through `PATH` for macOS portability.

[1.1.0]: https://github.com/StormBytePP/StormByte-System/releases/tag/1.1.0

## [1.0.0] - 2026-09-05

Initial public release of StormByte-System.

### Added

- **Process**: run external programs with piped stdin / stdout / stderr
    - Move-only ownership; starts on construction
    - `Wait()` for exit code (blocking, no timeout)
    - `Suspend()` / `Resume()`
    - Stream operators: write stdin, read stdout, `<< System::EoF` to close stdin
    - Process chaining (`p1 >> p2`) via background forwarder
    - `Stderr()` to read the stderr pipe
    - Cross-platform (POSIX fork/exec and Windows `CreateProcessW`)
- **Pipe** (internal): anonymous pipes for IPC (UNIX `pipe`/`pipe2`, Windows `CreatePipe`)
    - Atomic chunked writes, bind/dup helpers, handle inheritance flags on Windows
- **Variable**: expand environment strings (Windows `ExpandEnvironmentStrings`; UNIX `~` → home)
- **Exceptions**: `Exception`, `FileIOError`, `ExecutableNotFound`
- Unit tests for Linux, macOS and Windows (pipelines, stdin, exit codes, move)

### Fixed

- Process move no longer double-waits the same child (ownership is fully transferred)
- Failed `execvp` in the child uses `_exit(127)` instead of throwing across `fork`
- Removed unimplemented `Pipe::BindRead(Pipe&)` / `BindWrite(Pipe&)` declarations
- Pipes owned with `std::unique_ptr` instead of raw `new`/`delete`
- `SIGPIPE` ignored once per process (not on every Pipe construction)
- `WriteAtomic` treats empty input as success
- Windows command line built without a trailing space

### Notes

- On UNIX, if the executable cannot be started, the child exits with status **127**; the parent does not throw from the child path.
- `Wait()` has no timeout; it blocks until the process ends.

[1.0.0]: https://github.com/StormBytePP/StormByte-System/releases/tag/1.0.0
