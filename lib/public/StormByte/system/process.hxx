/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-System.
 *
 * StormByte-System is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * or later, as published by the Free Software Foundation.
 *
 * StormByte-System is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte-System. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 */

#pragma once

#include <StormByte/system/visibility.h>

#include <atomic>
#include <chrono>
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
 * @brief System module of the StormByte suite.
 */
namespace StormByte::System {
	class Pipe;	///< Forward declaration

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
	 * Wait() blocks until exit (no timeout).
	 */
	class STORMBYTE_SYSTEM_PUBLIC Process {
		public:
			/**
			 * @brief Construct and start.
			 * @param prog Executable path or name.
			 * @param args Argument list (not including argv[0]).
			 */
			Process(const std::filesystem::path& prog, const std::vector<std::string>& args = std::vector<std::string>());

			/**
			 * @brief Construct and start (moved).
			 * @param prog Executable path or name (moved).
			 * @param args Argument list (moved).
			 */
			Process(std::filesystem::path&& prog, std::vector<std::string>&& args = std::vector<std::string>());

			/**
			 * @brief Copy constructor (deleted).
			 */
			Process(const Process& proc) = delete;

			/**
			 * @brief Move constructor (invalidates the source).
			 */
			Process(Process&& proc) noexcept;

			/**
			 * @brief Copy assignment (deleted).
			 */
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
			 * @note If output is forwarded, this does not wait for downstream consumers to drain.
			 * @return Exit code, or -1 on failure / already reaped.
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
			 * @note If output is forwarded, this does not wait for downstream consumers to drain.
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
			 * @return Info (zeroed if moved-from).
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
			 * @brief Read remaining stdout into @p str.
			 * @param str Destination string.
			 * @return Reference to @p str.
			 */
			std::string& operator>>(std::string& str) const;

			/**
			 * @brief Read remaining stderr into @p str.
			 * @param str Destination string.
			 * @return Reference to @p str.
			 */
			std::string& Stderr(std::string& str) const;

			/**
			 * @brief Stream process stdout to an ostream.
			 */
			friend STORMBYTE_SYSTEM_PUBLIC std::ostream& operator<<(std::ostream& ostream, const Process& proc);

			/**
			 * @brief Write @p str to process stdin.
			 * @param str Data.
			 * @return *this.
			 */
			Process& operator<<(const std::string& str);

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

		protected:
			Status m_status;									///< Current status
			#ifdef UNIX
			pid_t m_pid;										///< Child PID (-1 if none)
			#else
			STARTUPINFOW m_siStartInfo;							///< Startup info
			PROCESS_INFORMATION m_piProcInfo;					///< Process info
			#endif
			std::shared_ptr<Pipe> m_pstdout;					///< stdout pipe
			std::shared_ptr<Pipe> m_pstdin;					///< stdin pipe
			std::shared_ptr<Pipe> m_pstderr;					///< stderr pipe
			std::filesystem::path m_program;					///< Program path
			std::vector<std::string> m_arguments;				///< Arguments
			std::unique_ptr<std::thread> m_forwarder;			///< Forwarder thread
			std::shared_ptr<std::atomic_bool> m_forwarder_cancel;			///< Forwarder cancellation state

		private:
			/**
			 * @brief Write to stdin.
			 * @param str Data.
			 */
			void Send(const std::string& str);

			/**
			 * @brief Spawn the child process.
			 */
			void Run();

			/**
			 * @brief Clear ownership so Wait/destructor are no-ops.
			 */
			void ReleaseOwnership() noexcept;

			#ifdef WINDOWS
			/**
			 * @brief Quote one argument for the Windows command-line parser.
			 * @param argument Argument text.
			 * @return Quoted command-line argument.
			 */
			static std::string QuoteWindowsArgument(const std::string& argument);

			/**
			 * @brief Full command line as wide string.
			 * @return Command line.
			 */
			std::wstring FullCommand() const;
			#endif
	};

	/**
	 * @brief Stream process stdout to an ostream.
	 * @param ostream Destination.
	 * @param proc Process.
	 * @return ostream.
	 */
	STORMBYTE_SYSTEM_PUBLIC std::ostream& operator<<(std::ostream& ostream, const Process& proc);
}
