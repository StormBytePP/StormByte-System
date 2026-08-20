/*
* Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
*
* This file is part of StormByte.
*
* StormByte is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* StormByte is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with StormByte. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <StormByte/system/visibility.h>

#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>
#ifdef WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <vector>

/**
 * @namespace System
 * @brief System utilities: processes, pipes, environment variables.
 */
namespace StormByte::System {
	class Pipe;	///< Forward declaration

	/**
	 * @struct _EoF
	 * @brief Tag type to close process stdin (write end).
	 */
	struct {} typedef _EoF;

	/**
	 * Sentinel used as `process << System::EoF` to close stdin.
	 */
	static constexpr const _EoF EoF = {};

	/**
	 * @class Process
	 * @brief Runs an external program with piped stdin/stdout/stderr.
	 *
	 * Starts immediately on construction. Move-only.
	 * Supports chaining (`p1 >> p2`), writing stdin, reading stdout/stderr,
	 * Suspend/Resume. @ref Wait() blocks until exit (no timeout).
	 */
	class STORMBYTE_SYSTEM_PUBLIC Process {
		public:
			/**
			 * @param prog Executable path or name.
			 * @param args Argument list (not including argv[0]).
			 */
			Process(const std::filesystem::path& prog, const std::vector<std::string>& args = std::vector<std::string>());

			/**
			 * @param prog Executable path or name (moved).
			 * @param args Argument list (moved).
			 */
			Process(std::filesystem::path&& prog, std::vector<std::string>&& args = std::vector<std::string>());

			/**
			 * Copy constructor (deleted).
			 */
			Process(const Process& proc) = delete;

			/**
			 * Move constructor (invalidates the source).
			 */
			Process(Process&& proc) noexcept;

			/**
			 * Copy assignment (deleted).
			 */
			Process& operator=(const Process& proc) = delete;

			/**
			 * Move assignment (invalidates the source).
			 */
			Process& operator=(Process&& proc) noexcept;

			/**
			 * Destructor (waits if still owning a child, then frees pipes).
			 */
			virtual ~Process() noexcept;

			#ifdef UNIX
			/**
			 * Blocks until the process exits (no timeout).
			 * @return Exit code, or -1 on failure / already reaped.
			 */
			int Wait() noexcept;

			/**
			 * @return Child PID, or -1 if not owning a process.
			 */
			pid_t Pid() noexcept;
			#else
			/**
			 * Blocks until the process exits (no timeout).
			 * @return Exit code, or (DWORD)-1 on failure.
			 */
			DWORD Wait() noexcept;

			/**
			 * @return Windows PROCESS_INFORMATION (zeroed if moved-from).
			 */
			PROCESS_INFORMATION Pid();
			#endif

			/**
			 * Suspends the child process.
			 */
			void Suspend();

			/**
			 * Resumes a suspended child process.
			 */
			void Resume();

			/**
			 * Forwards this process stdout to @p proc stdin (background thread).
			 * @param proc Target process.
			 * @return Reference to @p proc.
			 */
			Process& operator>>(Process& proc);

			/**
			 * Reads remaining stdout into @p str.
			 * @param str Destination string.
			 * @return Reference to @p str.
			 */
			std::string& operator>>(std::string& str) const;

			/**
			 * Reads remaining stderr into @p str.
			 * @param str Destination string.
			 * @return Reference to @p str.
			 */
			std::string& Stderr(std::string& str) const;

			/**
			 * Streams process stdout to an ostream.
			 */
			friend STORMBYTE_SYSTEM_PUBLIC std::ostream& operator<<(std::ostream& ostream, const Process& proc);

			/**
			 * Writes @p str to process stdin.
			 * @param str Data.
			 * @return *this.
			 */
			Process& operator<<(const std::string& str);

			/**
			 * Closes process stdin (write end).
			 * @param eof EoF sentinel.
			 */
			void operator<<(const System::_EoF& eof);

			/**
			 * @enum Status
			 * @brief Process lifecycle state.
			 */
			enum class Status: unsigned short {
				RUNNING,	///< Running
				SUSPENDED,	///< Suspended
				TERMINATED	///< Finished / cleaned up
			};

		protected:
			Status m_status;									///< Current status
			#ifdef UNIX
			pid_t m_pid;										///< Child PID (-1 if none)
			#else
			STARTUPINFOW m_siStartInfo;							///< Startup info
			PROCESS_INFORMATION m_piProcInfo;					///< Process info
			#endif
			std::unique_ptr<Pipe> m_pstdout;					///< stdout pipe
			std::unique_ptr<Pipe> m_pstdin;						///< stdin pipe
			std::unique_ptr<Pipe> m_pstderr;					///< stderr pipe
			std::filesystem::path m_program;					///< Program path
			std::vector<std::string> m_arguments;				///< Arguments
			std::unique_ptr<std::thread> m_forwarder;			///< Forwarder thread

		private:
			/**
			 * Writes to stdin.
			 * @param str Data.
			 */
			void Send(const std::string& str);

			/**
			 * Spawns the child process.
			 */
			void Run();

			/**
			 * Consumes stdout and forwards to another process stdin.
			 * @param exec Target process.
			 */
			void ConsumeAndForward(Process& exec);

			/**
			 * Clears ownership so Wait/destructor are no-ops.
			 */
			void ReleaseOwnership() noexcept;

			#ifdef WINDOWS
			/**
			 * @return Full command line as wide string.
			 */
			std::wstring FullCommand() const;
			#endif
	};

	/**
	 * Streams process stdout to an ostream.
	 */
	STORMBYTE_SYSTEM_PUBLIC std::ostream& operator<<(std::ostream& ostream, const Process& proc);
}
