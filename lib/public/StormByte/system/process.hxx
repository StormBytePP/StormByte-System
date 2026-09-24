/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-System.
 *
 * StormByte-System original source is dual-licensed:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You may redistribute and/or modify this file under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this file may be used under the terms of a commercial
 *    license agreement with the copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *
 * Both licenses apply only to original StormByte-System source in this
 * repository. They do not cover other StormByte modules or any third-party
 * material shipped with this repository (including everything under
 * thirdparty/, and in particular the bundled StormByte Base tree), which
 * remains under its own license.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte-System is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte-System. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#pragma once

#include <StormByte/cstring.hxx>
#include <StormByte/string/string.hxx>
#include <StormByte/system/visibility.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>
#ifdef WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace StormByte::System {
	class Pipe;
	class ProcessImplementation;

	/**
	 * @struct _EoF
	 * @brief Tag type to close process stdin (write end).
	 */
	struct {} typedef _EoF;

	/**
	 * @brief Sentinel used as `process << System::EoF` to close stdin.
	 */
	static constexpr const _EoF EoF = {};

	/**
	 * @class Process
	 * @brief Runs an external program with piped stdin/stdout/stderr.
	 *
	 * Starts immediately on construction. Move-only.
	 * Supports chaining (`p1 >> p2`), writing stdin, reading stdout/stderr, Suspend/Resume.
	 */
	class STORMBYTE_SYSTEM_PUBLIC Process {
		public:
			/**
			 * @brief Construct and start.
			 * @param prog Executable path or name.
			 * @param args Argument list (not including argv[0]).
			 */
			Process(const std::filesystem::path& prog, const std::vector<StormByte::String::String>& args = {});

			/**
			 * @brief Construct and start (moved).
			 * @param prog Executable path or name (moved).
			 * @param args Argument list (moved).
			 */
			Process(std::filesystem::path&& prog, std::vector<StormByte::String::String>&& args = {});

			Process(const Process& proc) = delete;

			/**
			 * @brief Move constructor (invalidates the source).
			 */
			Process(Process&& proc) noexcept;

			Process& operator=(const Process& proc) = delete;

			/**
			 * @brief Move assignment (invalidates the source).
			 */
			Process& operator=(Process&& proc) noexcept;

			/**
			 * @brief Destructor (waits if still owning a child, then frees pipes).
			 */
			virtual ~Process() noexcept;

			#ifdef UNIX
			/**
			 * @brief Block until the process exits (no timeout).
			 * @return Exit code, or -1 on failure, signal termination, or already reaped.
			 */
			int Wait() noexcept;

			/**
			 * @brief Wait for the process to exit up to @p timeout.
			 * @param timeout Maximum wait duration.
			 * @return Exit code, or -1 on timeout, failure, or already reaped.
			 */
			int Wait(std::chrono::milliseconds timeout) noexcept;

			/**
			 * @brief Child PID.
			 * @return PID, or -1 if not owning a process.
			 */
			pid_t Pid() noexcept;
			#else
			/**
			 * @brief Block until the process exits (no timeout).
			 * @return Exit code, or (DWORD)-1 on failure.
			 */
			DWORD Wait() noexcept;

			/**
			 * @brief Wait for the process to exit up to @p timeout.
			 * @param timeout Maximum wait duration.
			 * @return Exit code, or (DWORD)-1 on timeout, failure, or already reaped.
			 */
			DWORD Wait(std::chrono::milliseconds timeout) noexcept;

			/**
			 * @brief Windows PROCESS_INFORMATION.
			 * @return Info (zeroed if moved-from). The returned handles remain owned by Process.
			 */
			PROCESS_INFORMATION Pid();
			#endif

			/**
			 * @brief Suspend the child process.
			 */
			void Suspend();

			/**
			 * @brief Resume a suspended child process.
			 */
			void Resume();

			/**
			 * @brief Forward this process stdout to @p proc stdin (background thread).
			 * @param proc Target process.
			 * @return Reference to @p proc.
			 */
			Process& operator>>(Process& proc);

			/**
			 * @brief Read remaining stdout into a caller-owned string.
			 * @param str Destination.
			 * @return Reference to @p str.
			 */
			std::string& operator>>(std::string& str) const;

			/**
			 * @brief Read remaining stdout into owned text.
			 * @param str Destination.
			 * @return Reference to @p str.
			 */
			StormByte::String::String& operator>>(StormByte::String::String& str) const;

			/**
			 * @brief Read remaining stderr into a caller-owned string.
			 * @param str Destination.
			 * @return Reference to @p str.
			 */
			std::string& Stderr(std::string& str) const;

			/**
			 * @brief Read remaining stderr into owned text.
			 * @param str Destination.
			 * @return Reference to @p str.
			 */
			StormByte::String::String& Stderr(StormByte::String::String& str) const;

			/**
			 * @brief Stream process stdout to an ostream.
			 */
			friend STORMBYTE_SYSTEM_PUBLIC std::ostream& operator<<(std::ostream& ostream, const Process& proc);

			/**
			 * @brief Write UTF-8 text to process stdin.
			 * @param str Data.
			 * @return *this.
			 */
			Process& operator<<(std::string_view str);

			/**
			 * @brief Write owned UTF-8 text to process stdin.
			 * @param str Data.
			 * @return *this.
			 */
			Process& operator<<(const StormByte::String::String& str);

			/**
			 * @brief Write a CString to process stdin.
			 * @param str Data.
			 * @return *this.
			 */
			Process& operator<<(const StormByte::CString& str);

			/**
			 * @brief Close process stdin (write end).
			 * @param eof EoF sentinel.
			 */
			void operator<<(const System::_EoF& eof);

			/**
			 * @enum Status
			 * @brief Process lifecycle.
			 */
			enum class Status: unsigned short {
				RUNNING,	///< Running
				SUSPENDED,	///< Suspended
				TERMINATED	///< Finished / cleaned up
			};

		private:
			#ifdef WINDOWS
			/**
			 * @brief Quote one argument for the Windows command-line parser.
			 * @param argument Argument text.
			 * @return Quoted command-line argument.
			 */
			static std::string QuoteWindowsArgument(std::string_view argument);

			/**
			 * @brief Full command line as wide string.
			 * @return Command line.
			 */
			std::wstring FullCommand() const;
			#endif

			/**
			 * @brief Write to stdin.
			 * @param str Data.
			 */
			void Send(std::string_view str);

			/**
			 * @brief Spawn the child process.
			 */
			void Run();

			/**
			 * @brief Clear ownership so Wait/destructor are no-ops.
			 */
			void ReleaseOwnership() noexcept;

			/**
			 * @brief Join the forwarder without allowing exceptions to escape lifecycle methods.
			 */
			void JoinForwarder() noexcept;

			/**
			 * @brief Stop forwarding and optionally close the producer output read end.
			 * @param close_source_read Whether to close the local stdout read end.
			 */
			void StopForwarder(bool close_source_read) noexcept;

			std::unique_ptr<ProcessImplementation> m_implementation;
	};

	/**
	 * @brief Stream process stdout to an ostream.
	 * @param ostream Destination.
	 * @param proc Process.
	 * @return ostream.
	 */
	STORMBYTE_SYSTEM_PUBLIC std::ostream& operator<<(std::ostream& ostream, const Process& proc);
}
