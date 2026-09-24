# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Summary]

StormByte System is the C++26 process, environment and device layer of the StormByte suite.

It depends on [StormByte-String 1.0.0](https://github.com/StormBytePP/StormByte-String/releases/tag/1.0.0) or newer, which vendors [StormByte Base 2.0.0](https://github.com/StormBytePP/StormByte/releases/tag/2.0.0) or newer. This repository is not Base, Buffer, Config, Crypto, Database, Logger, Multimedia, Network or String.

Spawn children with piped stdin/stdout/stderr, chain them, suspend/resume, and expand environment strings. Classify the storage or network medium behind a path (`Device`) and obtain nominal throughput and transfer windows. POSIX and Windows stay behind one API. Text that crosses a DLL boundary is `StormByte::String::String` / `CString` (and wide counterparts), not `std::string` by value.

From 2.0.0, original System sources are dual-licensed: GNU Lesser General Public License v3.0 or later, or a commercial license from the copyright holder. That change does not cover other StormByte modules or third-party material under `thirdparty/` (including bundled StormByte-String and the Base tree it vendors).

If you landed here from a release link and have not read the tree:

- What this module is, how to build it, and short examples: [README.md](https://github.com/StormBytePP/StormByte-System/blob/master/README.md)
- License: dual license LGPL-3.0-or-later or commercial, [LICENSE](https://github.com/StormBytePP/StormByte-System/blob/master/LICENSE)

## [Unreleased]

### Added

- **`Device`:** query object for the medium behind a filesystem accessor. Stores only a `StormByte::String::String` path. Construction does not throw and does not open a handle. `Kind`, `Access` (`Readable` / `Writable` bitmask), `Throughput` and `Window` are computed on each call and are valid only when the Device converts to `true`. `operator bool` means a successful probe, not read or write permission. Constructors take `String`, `std::string_view`, `std::wstring_view` and `std::filesystem::path`. Symlinks are followed; a dangling symlink is `DeviceError::BrokenSymlink`. Special device nodes are never `Writable`. Suggested windows are `bps / 500` clamped to 16 KiB–1 MiB.
- **`Error` / `DeviceError`:** `std::error_code` domains (`StormByte.System`, `StormByte.System.Device`) with `StormByte::Error::Fault`. Process still throws `Exception`; the two hierarchies are not mixed.

### Changed

- **License:** original System sources are dual-licensed LGPL-3.0-or-later or commercial. Third-party trees under `thirdparty/` keep their own licenses. Neither license grants patent rights.
- Direct Base submodule replaced by [StormByte-String 1.0.0](https://github.com/StormBytePP/StormByte-String/releases/tag/1.0.0) or newer (`thirdparty/StormByte/string/src`). Base 2.0.0 or newer comes in through String.
- **Breaking:** `Variable::Expand` returns `StormByte::String::String`. Public overloads take `std::string_view`, `StormByte::String::String` and `StormByte::CString` (Windows also `std::wstring_view`, `WString`, `WCString`). `std::string` / `std::wstring` by value or as the only public input are gone.
- **Breaking:** `Process` arguments are `std::vector<StormByte::String::String>`. stdin `operator<<` takes `std::string_view`, `String` and `CString`. stdout/stderr can fill a caller `std::string&` or a `String&`.
- Pipe write operators (private) take `std::string_view`. Internal pipe and PIMPL buffers may still use `std::string` inside this module.
- Doxygen (`ENABLE_DOC`) resolves dependency headers via `INCLUDE_PATH` and skips `thirdparty`.

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
