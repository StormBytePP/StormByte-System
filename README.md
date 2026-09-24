# StormByte-System

![Multiplatform](https://img.shields.io/badge/Linux%20%7C%20Windows%20%7C%20macOS-Supported-1793D1)
![C++26](https://img.shields.io/badge/C%2B%2B-26-00599C?logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.28+-064F8C?logo=cmake&logoColor=white)
![License: LGPL v3 or commercial](https://img.shields.io/badge/License-LGPL_v3_or_commercial-blue.svg)
[![CI](https://github.com/StormBytePP/StormByte-System/actions/workflows/ci.yml/badge.svg)](https://github.com/StormBytePP/StormByte-System/actions/workflows/ci.yml)
[![Sponsor](https://img.shields.io/badge/Sponsor-StormBytePP-ea4aaa?logo=githubsponsors)](https://github.com/sponsors/StormBytePP)

StormByte-System is the C++26 system module of the [StormByte](https://dev.stormbyte.org/StormByte) suite.

Spawn processes with piped stdin/stdout/stderr, chain them, suspend/resume, and expand environment variables. Classify the storage or network medium behind a path. POSIX and Windows stay behind one API. Process and Device report failures as `StormByte::Error::Fault` in their own domains; they do not throw.

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
- [Support](#support)

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
| **No Process exceptions** | Spawn, wait and stdin failures are `Process::Error` held in `Fault()`. |
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
- `StormByte::System::Process::Error` + `Fault()` (domain tag `StormByte.System.Process`)
- Private `Pipe` (pipe2 / CreatePipe); construction and I/O do not throw
- `Device`: medium kind, access bitmask, throughput, windows
- `StormByte::System::Device::Error` + `Fault()` (domain tag `StormByte.System.Device`)

## Dependencies

| Dependency | Role |
|------------|------|
| [StormByte-String 1.0.0](https://github.com/StormBytePP/StormByte-String/releases/tag/1.0.0) | Owned UTF-8 / wide text across a DLL boundary |
| [StormByte (base) 2.0.0](https://github.com/StormBytePP/StormByte/releases/tag/2.0.0) | Visibility, `CString` / `WCString`, `Size`, `Bitmask`, `Error::Fault` (vendored by String) |

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
| `Process` | Spawn and talk to a child. Args are `std::vector<StormByte::String::String>`. Does not throw. |
| `Process::Error` | Domain `StormByte.System.Process`. Compare with `make_error_code(Process::Error::…)`. |
| `Variable` | Expand environment strings; returns `StormByte::String::String`. |
| `Device` | Classify the medium behind a path. Path is `StormByte::String::String`. |
| `Device::Kind` | `HDD`, `SSD`, `NVMeGen3` / `Gen4` / `Gen5`, `USBHDD`, `USBStick`, `Network`. |
| `Device::Access` / `Device::AccessFlag` | `Readable` / `Writable` bitmask for this process on this path. |
| `Device::Throughput` | Nominal sequential read/write bytes per second. |
| `Device::Window` | Suggested transfer sizes (`StormByte::Size`, 16 KiB–1 MiB). |
| `Device::Error` | Domain `StormByte.System.Device`. Held as `StormByte::Error::Fault`. |
| `System::EoF` | Close process stdin |

`Pipe` is private.

`Process << std::string_view` / `String` / `CString` writes stdin. A failed write sets `Process::Error::BrokenPipe`. `Process >> std::string&` or `Process >> String&` reads stdout into the caller’s object. The same pair exists for `Stderr`.

`Process::operator bool` is true only while a child is live (`RUNNING` or `SUSPENDED`). After a failed spawn, a finished wait, or a move-from, it is false. `Fault()` is the reason when you need one; a clean exit is not an error.

`Device` constructors take `String`, `std::string_view`, `std::wstring_view` and `std::filesystem::path`. `operator bool` is a successful probe, not permission. `Kind`, `Access`, `Throughput` and `Window` are valid only when the Device is `true`. Check `Fault()` for `BrokenSymlink`, `DeviceNotFound`, `NotADevice`, `ProbeFailed` or `Permission`.

## Examples

### Run a process

```cpp
#include <StormByte/string/string.hxx>
#include <StormByte/system/process.hxx>

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

using StormByte::String::String;
using StormByte::System::Process;

int run_or_report(Process& proc) {
	if (!proc) {
		const std::error_code code = proc.Fault().code();
		if (code == make_error_code(Process::Error::ExecutableNotFound))
			std::cerr << "executable not found\n";
		else if (code == make_error_code(Process::Error::Permission))
			std::cerr << "permission denied\n";
		else
			std::cerr << proc.Fault().what() << '\n';
		return 127;
	}

	std::string out;
	proc >> out;
	const int status = proc.Wait();
	if (!proc && proc.Fault().code() == make_error_code(Process::Error::AlreadyExited)) {
		// Second Wait() after a completed child.
	}
	std::cout << out;
	return status;
}

int main() {
	Process missing("/no/such/stormbyte-tool");
	(void)run_or_report(missing);

	Process echo("echo", std::vector<String>{String{"hello"}});
	(void)run_or_report(echo);

	Process sleeper("sleep", std::vector<String>{String{"2"}});
	if (sleeper) {
		const int timed = sleeper.Wait(std::chrono::milliseconds(10));
		if (timed < 0 && sleeper.Fault().code() == make_error_code(Process::Error::TimedOut))
			(void)sleeper.Wait();
	}

	Process done("true");
	(void)done.Wait();
	done << std::string_view("late");
	if (done.Fault().code() == make_error_code(Process::Error::BrokenPipe))
		std::cerr << "stdin closed after exit\n";
}
```

### Pipe two processes

```cpp
#include <StormByte/string/string.hxx>
#include <StormByte/system/process.hxx>

#include <iostream>
#include <string>
#include <vector>

using StormByte::String::String;
using StormByte::System::Process;

int main() {
	Process producer("printf", std::vector<String>{String{"%s"}, String{"banana\napple\n"}});
	Process consumer("sort");
	if (!producer || !consumer) {
		std::cerr << (producer ? consumer.Fault().what() : producer.Fault().what()) << '\n';
		return 1;
	}

	producer >> consumer;
	std::string sorted;
	consumer >> sorted;
	producer.Wait();
	consumer.Wait();
	std::cout << sorted;
}
```

### Expand variables

```cpp
#include <StormByte/system/variable.hxx>

#ifdef _WIN32
auto tmp = StormByte::System::Variable::Expand("%TEMP%");
#else
auto home = StormByte::System::Variable::Expand("~");
#endif
```

`home` / `tmp` are `StormByte::String::String`. Compare or print in the caller (`std::string_view(home)`, `std::string(home)`).

### Classify a device

```cpp
#include <StormByte/system/device.hxx>

#include <iostream>

using StormByte::System::Device;

int main() {
	Device dangling("/tmp/this-symlink-is-broken");
	if (!dangling) {
		if (dangling.Fault().code() == make_error_code(Device::Error::BrokenSymlink))
			std::cerr << "dangling symlink\n";
		else if (dangling.Fault().code() == make_error_code(Device::Error::DeviceNotFound))
			std::cerr << "path not found\n";
		else if (dangling.Fault().code() == make_error_code(Device::Error::Permission))
			std::cerr << "cannot probe path\n";
		else
			std::cerr << dangling.Fault().what() << '\n';
		return 1;
	}

	Device disk("/var/tmp/out.bin");
	if (!disk) {
		std::cerr << disk.Fault().what() << '\n';
		return 1;
	}

	const auto access = disk.Access();
	if (access.Has(StormByte::System::Device::AccessFlag::Writable)) {
		const auto window = disk.Window();
		(void)window.write; // StormByte::Size, 16 KiB–1 MiB
	}

	if (!access.Has(StormByte::System::Device::AccessFlag::Readable)) {
		// Missing leaf on an existing writable volume: probe can still succeed.
	}
}
```

A missing leaf on an existing writable volume can still convert to `true` (not readable, possibly writable). A special device node is never writable. Throughput is a nominal preset, not a benchmark.

## Design notes

- Construction starts the child immediately and does not throw.
- `Wait()` has an untimed overload and a timed `Wait(std::chrono::milliseconds)` overload. Timeout sets `Process::Error::TimedOut` and leaves the child running. A second wait after a finished child sets `AlreadyExited`.
- On UNIX, System ignores `SIGPIPE` once process-wide so closed pipe peers report write failure instead of terminating the host process.
- Windows `Suspend()` / `Resume()` operate on a snapshot of the child threads; a thread created during enumeration may not be affected.
- Windows process stdio handles are made non-inheritable immediately after process creation; a small inheritance window exists during `CreateProcessW`.
- Destructor waits if the process is still owned.
- Move invalidates the source (PID / handles cleared).
- `Process` is inheritable. Its only data member is a private PIMPL. `std::filesystem::path` is accepted by constructor and copied into that PIMPL; it is not a public field.
- `Device` is copyable. It stores only the caller accessor as `String`. Classification, access, rates and windows are computed on each call. Symlinks are followed (`stat`). Process and Device errors live in separate domains; neither throws.

## Testing

Enable tests in CMake (`ENABLE_TEST`) and run CTest from the build tree.

## Contributing

Issues on GitHub. No wiki, no discussions.

## License

Dual license: GNU Lesser General Public License v3.0 or later, or a commercial license from the copyright holder. See [LICENSE](LICENSE), [COPYING.LGPLv3](COPYING.LGPLv3) and <https://www.gnu.org/licenses/lgpl-3.0.html>. Third-party trees under `thirdparty/` keep their own licenses.

## Support

StormByte is developed in spare time. Sponsorship is optional and does not buy features, priority or support.

- [GitHub Sponsors](https://github.com/sponsors/StormBytePP)
- [PayPal](https://paypal.me/StormBytePP)
