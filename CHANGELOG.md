## [Summary]

StormByte System is the C++26 process, device and environment layer of the StormByte suite.

It depends on [StormByte-String 1.0.0](https://github.com/StormBytePP/StormByte-String/releases/tag/1.0.0) or newer, which vendors [StormByte Base 2.0.0](https://github.com/StormBytePP/StormByte/releases/tag/2.0.0). This repository is not Base, Buffer, Config, Crypto, Database, Logger, Multimedia, Network or String.

Spawn children with piped stdin/stdout/stderr, chain them, suspend/resume, classify the medium behind a path, and expand environment strings. POSIX and Windows stay behind one API. Process and Device report failures as `StormByte::Error::Fault` in their own domains; they do not throw.

If you landed here from a release link and have not read the tree:

- What this module is, how to build it, and examples: [README.md](https://github.com/StormBytePP/StormByte-System/blob/master/README.md)
- License: dual LGPL-3.0-or-later or commercial. See [LICENSE](https://github.com/StormBytePP/StormByte-System/blob/master/LICENSE) and [COPYING.LGPLv3](https://github.com/StormBytePP/StormByte-System/blob/master/COPYING.LGPLv3).

## [Unreleased]

[Unreleased]: https://github.com/StormBytePP/StormByte-System/compare/2.0.0...HEAD

## [2.0.0] - 2026-09-24

### Added

- **Device**: classify the medium behind a path (`Kind`, `Access` bitmask, nominal `Throughput`, suggested `Window`).
    - Copyable; stores only the caller accessor as `StormByte::String::String`.
    - Probe is on-demand. `operator bool` is probe success, not permission.
    - Errors are `StormByte::System::Device::Error` in domain `StormByte.System.Device`, held as `StormByte::Error::Fault`.
- Dual license on original System sources: LGPL-3.0-or-later **or** commercial (`LICENSE` + `COPYING.LGPLv3`).
- `STORMBYTE_SYSTEM_SHARED` CMake option (default ON) so a static Windows consumer does not see `dllimport`.

### Changed

- **Breaking:** Process no longer throws. Spawn, wait and stdin failures are `StormByte::System::Process::Error` in domain `StormByte.System.Process`, held as `Fault()`.
    - `operator bool` is true only while a child is live (`RUNNING` or `SUSPENDED`).
    - Timed `Wait` sets `TimedOut` and leaves the child running. A second wait after a successful reap sets `AlreadyExited`.
    - A failed stdin write sets `BrokenPipe`.
- **Breaking:** `Variable::Expand` returns `StormByte::String::String`. On Windows, a failed `ExpandEnvironmentStringsW` returns the original text (same as a missing UNIX home).
- **Breaking:** Process constructor arguments are `std::vector<StormByte::String::String>`.
- Pipe construction and I/O no longer throw. Invalid pipes convert to `false`.
- Public text across a DLL boundary uses `StormByte::String::String` / `CString`.
- Depends on StormByte-String 1.0.0 (vendors Base 2.0.0).
- Visibility macros follow Base/Logger (`EXPORTS` / `STORMBYTE_SYSTEM_SHARED` / static empty).
- Windows Device probe links `iphlpapi` and `ws2_32`. macOS Device probe links IOKit and CoreFoundation.

### Removed

- **Breaking:** `StormByte/system/exception.hxx` (`Exception`, `FileIOError`, `ExecutableNotFound`, `ProcessCreationError`).
- **Breaking:** `StormByte::System::Error` and `StormByte/system/error.hxx` (domain `StormByte.System`). Device and Process keep their own domains.

[2.0.0]: https://github.com/StormBytePP/StormByte-System/releases/tag/2.0.0

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
