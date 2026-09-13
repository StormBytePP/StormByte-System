# Changelog

All notable changes to **StormByte-System** will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- Ported System exception messages to the `StormByte::Component` format and added `ProcessCreationError` for process creation failures.
- Moved process pipeline forwarding into the internal `Pipe` abstraction while preserving buffered and future output.

### Fixed

- Pipe construction now checks platform errors and moved pipes invalidate their source endpoints.
- Pipe reads, writes, polling and descriptor binding now distinguish interruption, EOF and failure.
- Process pipeline forwarding now retains pipe ownership independently of process object lifetime.
- Reconnecting a process output now joins the previous forwarder before replacing it.
- Process waiting now cancels forwarding before reaping and joining, avoiding downstream backpressure deadlocks.
- Added a public timed `Wait(std::chrono::milliseconds)` overload; the existing untimed overload remains unchanged.

## [Summary]

StormByte System is the C++26 process and environment layer of the StormByte suite.

Dependency baseline: [StormByte (base) 1.1.0](https://github.com/StormBytePP/StormByte/releases/tag/1.1.0).

Spawn children with piped stdin/stdout/stderr, chain them, suspend/resume, and expand environment strings. POSIX and Windows stay behind one API.

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
