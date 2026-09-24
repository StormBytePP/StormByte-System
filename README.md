# StormByte-System

![Multiplatform](https://img.shields.io/badge/Linux%20%7C%20Windows%20%7C%20macOS-Supported-1793D1)
![C++26](https://img.shields.io/badge/C%2B%2B-26-00599C?logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.28+-064F8C?logo=cmake&logoColor=white)
![License: LGPL v3 or commercial](https://img.shields.io/badge/License-LGPL--3.0--or--later%20OR%20Commercial-blue.svg)
[![CI](https://github.com/StormBytePP/StormByte-System/actions/workflows/ci.yml/badge.svg)](https://github.com/StormBytePP/StormByte-System/actions/workflows/ci.yml)
[![Sponsor](https://img.shields.io/badge/Sponsor-GitHub-ea4aaa?logo=github-sponsors&logoColor=white)](https://github.com/sponsors/StormBytePP)

StormByte-System is the C++26 process, device and host module of the [StormByte](https://dev.stormbyte.org/StormByte) suite.

Spawn children with piped stdin/stdout/stderr, classify the medium behind a path, resolve directories and the current executable, inspect the machine, name the calling thread, and expand environment strings. POSIX and Windows stay behind one API. Failures are `StormByte::Error::Fault` in a per-type domain (`StormByte.System.*`). Nothing in this module throws.

## Table of Contents

- [Repository](#repository)
- [Installation](#installation)
- [Why StormByte-System](#why-stormbyte-system)
- [Features](#features)
- [Dependencies](#dependencies)
- [The rest of the suite](#the-rest-of-the-suite)
- [Public API](#public-api)
- [Examples](#examples)
	- [Run a process](#run-a-process)
	- [Pipe two processes](#pipe-two-processes)
	- [Classify a device](#classify-a-device)
	- [Directories and files](#directories-and-files)
	- [Host](#host)
	- [Calling thread](#calling-thread)
	- [Expand variables](#expand-variables)
- [Design notes](#design-notes)
- [Testing](#testing)
- [Support](#support)
- [Contributing](#contributing)
- [License](#license)

## Repository

- [StormByte-System](https://github.com/StormBytePP/StormByte-System)

## Installation

```bash
git clone https://github.com/StormBytePP/StormByte-System.git
cd StormByte-System
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cmake --install build
```

`STORMBYTE_SYSTEM_SHARED` (default ON) builds the shared library. OFF is a static library; Windows consumers then do not see `dllimport`.

## Why StormByte-System

| Goal | How it is achieved |
|------|--------------------|
| **One process API** | `Process` starts on construct; pipes are private. Errors are `Fault()`, not exceptions. |
| **Shell-like chaining** | `p1 >> p2` forwards stdout to stdin on a worker thread. |
| **stdin control** | `<<` writes; `<< System::EoF` closes the write end. |
| **Medium behind a path** | `Device` probes Kind, Access, Throughput and Window on demand. |
| **This process on disk** | `Directory` and `File` resolve cwd, home, temp and the running image. |
| **This machine** | `Host` reports name, OS, kernel, CPU, ISA, RAM and bitness. |
| **This thread** | `ThisThread::Sleep` and `Name` (reject, do not truncate, if too long). |
| **Environment text** | `Variable::Expand` (`%VAR%` on Windows, `~` on UNIX). |

## Features

- Move-only `Process` (fork/exec or `CreateProcessW`)
- Piped stdin, stdout, stderr; chaining; `Wait` / timed `Wait`; `Suspend` / `Resume`
- `Device` classification (HDD, SSD, NVMe generations, USB, Network)
- `Directory` / `File` locations of this process
- `Host` identity and capacity
- `ThisThread` sleep and name
- `Variable::Expand`
- Private `Pipe` (`pipe2` / `CreatePipe`)
- Dual license on original sources

## Dependencies

| Dependency | Role |
|------------|------|
| [StormByte-String 1.0.0](https://github.com/StormBytePP/StormByte-String/releases/tag/1.0.0) | UTF-8 / UTF-16 text across the DLL boundary (vendors [Base 2.0.0](https://github.com/StormBytePP/StormByte/releases/tag/2.0.0)) |

## The rest of the suite

| Module | Role | API |
| --- | --- | --- |
| [Base](https://github.com/StormBytePP/StormByte) | Error/Fault, visibility, Size, Bitmask, serialization, UUID, concepts | [/StormByte](https://dev.stormbyte.org/StormByte) |
| [Buffer](https://github.com/StormBytePP/StormByte-Buffer) | FIFO, SharedFIFO, Ring, Producer/Consumer and multi-stage pipelines | [/StormByte-Buffer](https://dev.stormbyte.org/StormByte-Buffer) |
| [Config](https://github.com/StormBytePP/StormByte-Config) | Human-readable text and versioned binary documents | [/StormByte-Config](https://dev.stormbyte.org/StormByte-Config) |
| [Crypto](https://github.com/StormBytePP/StormByte-Crypto) | Hash, compress, encrypt, sign and key agreement | [/StormByte-Crypto](https://dev.stormbyte.org/StormByte-Crypto) |
| [Database](https://github.com/StormBytePP/StormByte-Database) | One API over SQLite, PostgreSQL and MariaDB | [/StormByte-Database](https://dev.stormbyte.org/StormByte-Database) |
| [Logger](https://github.com/StormBytePP/StormByte-Logger) | Stream logger with levels, headers and redaction | [/StormByte-Logger](https://dev.stormbyte.org/StormByte-Logger) |
| [Multimedia](https://github.com/StormBytePP/StormByte-Multimedia) | Decode, encode and containers without raw FFmpeg types | [/StormByte-Multimedia](https://dev.stormbyte.org/StormByte-Multimedia) |
| [Network](https://github.com/StormBytePP/StormByte-Network) | Framed packets, Client/Server, IPv4/IPv6 TCP | [/StormByte-Network](https://dev.stormbyte.org/StormByte-Network) |
| [String](https://github.com/StormBytePP/StormByte-String) | Owned UTF-8 / UTF-16 text safe across a DLL boundary | [/StormByte-String](https://dev.stormbyte.org/StormByte-String) |
| **System** | This repository | [/StormByte-System](https://dev.stormbyte.org/StormByte-System) |

## Public API

| Name | Role |
|------|------|
| `Process` | Spawn and talk to a child. `operator bool` is true only while the child is live. `Fault()` is `StormByte.System.Process`. |
| `Device` | Medium behind a path. `operator bool` is probe success, not permission. `Fault()` is `StormByte.System.Device`. `Throughput` / `Window` are virtual. |
| `Directory` | `Current`, `Home`, `Temporary`, `CurrentExecutable`. `bool` + out `String`. `LastError()` is TLS in this module. |
| `File` | `Temporary(prefix, suffix)` (caller unlinks) and `CurrentExecutable`. Same `bool` + `LastError` contract. |
| `Host` | `Name`, `Architecture`, `CPU`, `OS`, `Kernel`, `PageSize`, `PhysicalMemory`, `AvailableMemory`, `LogicalProcessors`, `Bitness`. |
| `ThisThread` | `Sleep`; `Name` get/set. Set returns `false` and `TooLong` if the platform limit is exceeded. |
| `Variable` | Expand environment strings to `StormByte::String::String`. |
| `System::EoF` | Close process stdin. |

`Pipe` is private. There is no `StormByte/system/exception.hxx` and no generic `StormByte.System` error domain.

## Examples

### Run a process

```cpp
#include <StormByte/system/process.hxx>

using StormByte::System::Process;

Process missing("/no/such/stormbyte-tool");
if (!missing) {
	if (missing.Fault().code() == make_error_code(Process::Error::ExecutableNotFound))
		/* spawn failed */;
}

Process echo("echo", {StormByte::String::String("hello")});
if (!echo)
	return;
std::string out;
echo >> out;
if (echo.Wait() != 0)
	/* child status */;
if (echo.Fault().code() == make_error_code(Process::Error::AlreadyExited))
	/* second Wait */;
```

On Windows use a real binary (`cmd.exe`, `where.exe`) instead of `echo` if it is not on `PATH` the way you expect.

### Pipe two processes

```cpp
Process producer("printf", {StormByte::String::String("%s"), StormByte::String::String("hello\n")});
Process consumer("tr", {StormByte::String::String("a-z"), StormByte::String::String("A-Z")});
producer >> consumer;
producer << StormByte::System::EoF;
std::string out;
consumer >> out;
producer.Wait();
consumer.Wait();
```

### Classify a device

```cpp
#include <StormByte/system/device.hxx>

StormByte::System::Device root("/");
if (!root) {
	/* BrokenSymlink, DeviceNotFound, Permission, ProbeFailed */
	return;
}
auto kind = root.Kind();
auto access = root.Access();
auto rate = root.Throughput();
auto window = root.Window();
```

On Windows pass `C:\\`. `operator bool` is not “can write”. A special device node is never writable. Symlinks are followed; a dangling link is `BrokenSymlink`.

### Directories and files

```cpp
#include <StormByte/system/directory.hxx>
#include <StormByte/system/file.hxx>

StormByte::String::String cwd, home, tmpdir, exe_dir, exe, scratch;
if (!StormByte::System::Directory::Current(cwd))
	/* Directory::LastError() */;
StormByte::System::Directory::Home(home);
StormByte::System::Directory::Temporary(tmpdir);
StormByte::System::Directory::CurrentExecutable(exe_dir);
StormByte::System::File::CurrentExecutable(exe);
if (StormByte::System::File::Temporary(scratch, "SB", ".tmp")) {
	/* use scratch; caller unlinks */
}
```

Windows `File::Temporary` only uses the first three characters of the prefix (`GetTempFileNameW`). The suffix is appended after the generated name.

### Host

```cpp
#include <StormByte/system/host.hxx>

StormByte::String::String hostname;
StormByte::System::Host::Name(hostname);
auto os = StormByte::System::Host::OS();       // "Gentoo 2.18", "Windows 11", "macOS 15.1"
auto kernel = StormByte::System::Host::Kernel(); // "Linux … SMP PREEMPT_DYNAMIC", "NT 10.0.26100", "Darwin …"
auto cpu = StormByte::System::Host::CPU();
auto arch = StormByte::System::Host::Architecture();
auto page = StormByte::System::Host::PageSize();
auto ram = StormByte::System::Host::PhysicalMemory();
unsigned bits = StormByte::System::Host::Bitness();
```

`LogicalProcessors` and `Bitness` do not update `LastError`. The others do. A failed `Size` is zero; a failed `String` is empty.

### Calling thread

```cpp
#include <StormByte/system/this_thread.hxx>

StormByte::System::ThisThread::Sleep(std::chrono::milliseconds(10));
if (!StormByte::System::ThisThread::Name("worker-1")) {
	/* TooLong or Failed; LastError() */
}
StormByte::String::String name;
StormByte::System::ThisThread::Name(name);
```

On Linux/macOS the pthread name limit is 15 characters. Windows `SetThreadDescription` does not use that limit.

### Expand variables

```cpp
#include <StormByte/system/variable.hxx>

auto home = StormByte::System::Variable::Expand("~");
auto tmp = StormByte::System::Variable::Expand("%TEMP%");
```

A failed Windows expand returns the original text (same idea as a missing UNIX `$HOME`).

## Design notes

- `Process` construction starts the child immediately and does not throw. `operator bool` is live status only.
- Timed `Wait` sets `TimedOut` and leaves the child running.
- On UNIX, System ignores `SIGPIPE` once process-wide so a closed pipe peer reports write failure instead of killing the host.
- Windows `Suspend()` / `Resume()` snapshot the child threads; a thread created during enumeration may be missed.
- Windows stdio handles are made non-inheritable after `CreateProcessW`; a short inheritance window exists during creation.
- `Device` stores only the caller accessor. Kind/Access/Throughput/Window are valid only when the Device converts to `true`.
- `Directory` / `File` / `Host` / `ThisThread` `LastError()` is `thread_local` inside this module, exposed by an exported getter. Do not put `thread_local` in a public header.
- Public text across a DLL boundary is `StormByte::String::String` / `CString`.
- Destructor of `Process` waits if the child is still owned. Move invalidates the source.

## Testing

Enable tests in CMake (`ENABLE_TEST`) and run CTest from the build tree. Device, Directory, File, Host and ThisThread print probe data and always return success (the machine is not a fixture). Process tests assert error codes. Use `ctest -V` to see stdout.

## Support

If StormByte is useful to you, you can sponsor development on [GitHub Sponsors](https://github.com/sponsors/StormBytePP).

## Contributing

Issues on GitHub. No wiki, no discussions. See [CONTRIBUTING.md](CONTRIBUTING.md) for coding style (StormByte flavor) and copyright assignment to the owner.

## License

Original StormByte-System sources are dual-licensed: GNU Lesser General Public License v3 or later **or** a commercial license from the copyright holder.

- [LICENSE](LICENSE) — dual-license notice
- [COPYING.LGPLv3](COPYING.LGPLv3) — LGPL-3.0 text

Bundled third-party trees (including Base under `thirdparty/`) keep their own licenses.
