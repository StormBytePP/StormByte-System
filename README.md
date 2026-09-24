# StormByte-System

![Multiplatform](https://img.shields.io/badge/Linux%20%7C%20Windows%20%7C%20macOS-Supported-1793D1)
![C++26](https://img.shields.io/badge/C%2B%2B-26-00599C?logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.28+-064F8C?logo=cmake&logoColor=white)
![License: LGPL v3 or commercial](https://img.shields.io/badge/License-LGPL_v3_or_commercial-blue.svg)
[![CI](https://github.com/StormBytePP/StormByte-System/actions/workflows/ci.yml/badge.svg)](https://github.com/StormBytePP/StormByte-System/actions/workflows/ci.yml)
[![Sponsor](https://img.shields.io/badge/Sponsor-GitHub-ea4aaa?logo=github-sponsors&logoColor=white)](https://github.com/sponsors/StormBytePP)

StormByte-System is the C++26 system module of the [StormByte](https://dev.stormbyte.org/StormByte) suite.

Spawn processes with piped stdin/stdout/stderr, chain them, suspend/resume, and expand environment variables. Classify the storage or network medium behind a path. POSIX and Windows stay behind one API.

It depends on [StormByte-String 1.0.0](https://github.com/StormBytePP/StormByte-String/releases/tag/1.0.0) or newer, which vendors [StormByte Base 2.0.0](https://github.com/StormBytePP/StormByte/releases/tag/2.0.0) or newer.

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
	- [Expand variables](#expand-variables)
	- [Classify a device](#classify-a-device)
- [Design notes](#design-notes)
- [Testing](#testing)
- [Contributing](#contributing)
- [License](#license)

## Repository

- [StormByte-System](https://github.com/StormBytePP/StormByte-System)

## Installation

```bash
git clone --recursive https://github.com/StormBytePP/StormByte-System.git
cd StormByte-System
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cmake --install build
```

## Why StormByte-System

| Goal | How it is achieved |
|------|--------------------|
| **One process API** | `Process` starts on construct; pipes are private. |
| **Shell-like chaining** | `p1 >> p2` forwards stdout to stdin on a worker thread. |
| **stdin control** | `<<` writes; `<< System::EoF` closes the write end. |
| **Environment paths** | `Variable::Expand` (`%VAR%` on Windows, `~` on UNIX). |
| **Device probe** | `Device` classifies the medium of a path and yields nominal rates and transfer windows. |
| **DLL-safe text** | Public text is `StormByte::String::String` / `CString`, not `std::string` by value. |

## Features

- Move-only `Process` (fork/exec or CreateProcess)
- Piped stdin, stdout, stderr
- `Wait`, `Pid`, `Suspend`, `Resume`
- Process chaining
- `FileIOError`, `ExecutableNotFound`, `ProcessCreationError`
- Private `Pipe` (pipe2 / CreatePipe)
- `Device`: medium kind, access bitmask, throughput, windows
- `Error` / `DeviceError` as `std::error_code` + `StormByte::Error::Fault`

## Dependencies

| Dependency | Role |
|------------|------|
| [StormByte-String 1.0.0](https://github.com/StormBytePP/StormByte-String/releases/tag/1.0.0) | Owned UTF-8 / wide text across a DLL boundary |
| [StormByte (base) 2.0.0](https://github.com/StormBytePP/StormByte/releases/tag/2.0.0) | Exceptions, visibility, `CString` / `WCString`, `Size`, `Bitmask`, `Error::Fault` (vendored by String) |

## The rest of the suite

| Module | Role | API |
| --- | --- | --- |
| [Base](https://github.com/StormBytePP/StormByte) | Exceptions, Expected, `Size`, serialization, UUID, concepts | [/StormByte](https://dev.stormbyte.org/StormByte) |
| [Buffer](https://github.com/StormBytePP/StormByte-Buffer) | FIFO, SharedFIFO, Ring, Producer/Consumer and multi-stage pipelines | [/StormByte-Buffer](https://dev.stormbyte.org/StormByte-Buffer) |
| [Config](https://github.com/StormBytePP/StormByte-Config) | Human-readable text and versioned binary documents (groups, lists, raw bytes) | [/StormByte-Config](https://dev.stormbyte.org/StormByte-Config) |
| [Crypto](https://github.com/StormBytePP/StormByte-Crypto) | Hash, compress, encrypt, sign and key agreement — Crypto++ never leaves the private tree | [/StormByte-Crypto](https://dev.stormbyte.org/StormByte-Crypto) |
| [Database](https://github.com/StormBytePP/StormByte-Database) | One API over SQLite, PostgreSQL and MariaDB | [/StormByte-Database](https://dev.stormbyte.org/StormByte-Database) |
| [Logger](https://github.com/StormBytePP/StormByte-Logger) | Stream logger with levels, headers, hierarchical components and `Scope` | [/StormByte-Logger](https://dev.stormbyte.org/StormByte-Logger) |
| [Multimedia](https://github.com/StormBytePP/StormByte-Multimedia) | Decode, encode and containers without raw FFmpeg types; codecs enabled only if present | [/StormByte-Multimedia](https://dev.stormbyte.org/StormByte-Multimedia) |
| [Network](https://github.com/StormBytePP/StormByte-Network) | Framed packets, Client/Server, IPv4/IPv6 TCP and Buffer pipelines (compress/encrypt) | [/StormByte-Network](https://dev.stormbyte.org/StormByte-Network) |
| [String](https://github.com/StormBytePP/StormByte-String) | Owned UTF-8 / wide text that can cross a DLL boundary (`String`, `WString`, `CString`, `WCString`) | [/StormByte-String](https://dev.stormbyte.org/StormByte-String) |
| **System** | This repository | [/StormByte-System](https://dev.stormbyte.org/StormByte-System) |

## Public API

| Type | Role |
|------|------|
| `Process` | Spawn and talk to a child. Args are `std::vector<StormByte::String::String>`. |
| `Variable` | Expand environment strings; returns `StormByte::String::String`. |
| `Device` | Classify the medium behind a path. Path is `StormByte::String::String`. |
| `Kind` | `HDD`, `SSD`, `NVMeGen3` / `Gen4` / `Gen5`, `USBHDD`, `USBStick`, `Network`. |
| `Access` / `AccessFlag` | `Readable` / `Writable` bitmask for this process on this path. |
| `Throughput` | Nominal sequential read/write bytes per second. |
| `Window` | Suggested transfer sizes (`StormByte::Size`, 16 KiB–1 MiB). |
| `Error` / `DeviceError` | `std::error_code` domains; held as `StormByte::Error::Fault`. |
| `Exception` / `FileIOError` / `ExecutableNotFound` / `ProcessCreationError` | Thrown process errors |
| `System::EoF` | Close process stdin |

`Pipe` is private.

`Process << std::string_view` / `String` / `CString` writes stdin. `Process >> std::string&` or `Process >> String&` reads stdout in the caller’s object. The same pair exists for `Stderr`.

`Device` constructors take `String`, `std::string_view`, `std::wstring_view` and `std::filesystem::path`. `operator bool` is a successful probe, not permission. `Kind`, `Access`, `Throughput` and `Window` are valid only when the Device is `true`. Check `Fault()` for `BrokenSymlink`, `DeviceNotFound`, `NotADevice`, `ProbeFailed` or `Error::Permission`.

## Examples

### Run a process

```cpp
#include <StormByte/string/string.hxx>
#include <StormByte/system/process.hxx>

StormByte::System::Process echo("/bin/echo", {StormByte::String::String("hello")});
std::string out;
echo >> out;
echo.Wait();
```

### Pipe two processes

```cpp
StormByte::System::Process producer("/bin/echo", {StormByte::String::String("hello")});
StormByte::System::Process consumer("/usr/bin/tr", {StormByte::String::String("a-z"), StormByte::String::String("A-Z")});
producer >> consumer;
producer << StormByte::System::EoF;
std::string out;
consumer >> out;
```

On Windows use `C:\\Windows\\System32\\cmd.exe` (or the real binary path) instead of `/bin/echo`.

### Expand variables

```cpp
#include <StormByte/system/variable.hxx>

auto home = StormByte::System::Variable::Expand("~");
#ifdef WINDOWS
auto tmp = StormByte::System::Variable::Expand("%TEMP%");
#endif
```

`home` / `tmp` are `StormByte::String::String`. Compare or print in the caller (`std::string_view(home)`, `std::string(home)`).

### Classify a device

```cpp
#include <StormByte/system/device.hxx>

StormByte::System::Device disk("/var/tmp/out.bin");
if (!disk) {
	// disk.Fault() — BrokenSymlink, DeviceNotFound, Permission, …
	return;
}

const auto kind = disk.Kind();
const auto access = disk.Access();
const auto rate = disk.Throughput();
const auto window = disk.Window();

if (access.Has(StormByte::System::AccessFlag::Writable)) {
	// window.write is a StormByte::Size, 16 KiB–1 MiB
}
```

A missing leaf on an existing writable volume can still convert to `true` (not readable, possibly writable). A special device node is never writable. Throughput is a nominal preset, not a benchmark.

## Design notes

- Construction starts the child immediately.
- `Wait()` has an untimed overload and a timed `Wait(std::chrono::milliseconds)` overload.
- On UNIX, System ignores `SIGPIPE` once process-wide so closed pipe peers report write failure instead of terminating the host process.
- Windows `Suspend()` / `Resume()` operate on a snapshot of the child threads; a thread created during enumeration may not be affected.
- Windows process stdio handles are made non-inheritable immediately after process creation; a small inheritance window exists during `CreateProcessW`.
- Destructor waits if the process is still owned.
- Move invalidates the source (PID / handles cleared).
- `Process` is inheritable. Its only data member is a private PIMPL. `std::filesystem::path` is accepted by constructor and copied into that PIMPL; it is not a public field.
- `Device` is copyable. It stores only the caller accessor as `String`. Classification, access, rates and windows are computed on each call. Symlinks are followed (`stat`). Process exceptions and `Fault` are separate hierarchies.

## Testing

Enable tests in CMake (`ENABLE_TEST`) and run CTest from the build tree.

## Contributing

Issues on GitHub. No wiki, no discussions.

## License

Dual license: GNU Lesser General Public License v3.0 or later, or a commercial license from the copyright holder. See [LICENSE](LICENSE), [COPYING.LGPLv3](COPYING.LGPLv3) and <https://www.gnu.org/licenses/lgpl-3.0.html>. Third-party trees under `thirdparty/` keep their own licenses.
