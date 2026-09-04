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

#include <string>
#include <vector>

#ifdef WINDOWS
#include <windows.h>
#else
#include <poll.h>
#endif

/**
 * @brief System module of the StormByte suite.
 */
namespace StormByte::System {
	/**
	 * @class Pipe
	 * @brief Cross-platform anonymous pipe for process IPC.
	 *
	 * UNIX: pipe(2)/pipe2. Windows: CreatePipe. Move-only.
	 * @note On UNIX, SIGPIPE is ignored process-wide once (first Pipe construction).
	 */
	class STORMBYTE_SYSTEM_PRIVATE Pipe {
		public:
			/**
			 * @brief Maximum bytes per read (4 MiB).
			 */
			static constexpr const size_t MAX_READ_BYTES = 4 * 1024 * 1024;

			/**
			 * @brief Create a new pipe pair.
			 */
			Pipe();

			/**
			 * @brief Copy constructor (deleted).
			 */
			Pipe(const Pipe&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Pipe(Pipe&&) = default;

			/**
			 * @brief Copy assignment (deleted).
			 */
			Pipe& operator=(const Pipe&) = delete;

			/**
			 * @brief Move assignment.
			 */
			Pipe& operator=(Pipe&&) = default;

			/**
			 * @brief Close both ends.
			 */
			~Pipe() noexcept;

			#ifdef UNIX
			/**
			 * @brief Dup2 read end onto @p fd.
			 * @param fd Destination file descriptor.
			 */
			void BindRead(int fd) noexcept;

			/**
			 * @brief Dup2 write end onto @p fd.
			 * @param fd Destination file descriptor.
			 */
			void BindWrite(int fd) noexcept;

			/**
			 * @brief Write a string to the write end.
			 * @param str Data.
			 * @return Bytes written.
			 */
			ssize_t Write(const std::string& str);

			/**
			 * @brief Whether the write end is no longer writable (HUP/ERR).
			 * @return true if so.
			 */
			bool WriteEOF() const;

			/**
			 * @brief Read into @p buffer up to @p size bytes.
			 * @param buffer Destination.
			 * @param size Max bytes.
			 * @return Bytes read.
			 */
			ssize_t Read(std::vector<char>& buffer, ssize_t size) const;

			/**
			 * @brief Whether the read end reports HUP/ERR.
			 * @return true if so.
			 */
			bool ReadEOF() const;
			#else
			/**
			 * @brief Set handle information on the read end.
			 * @param mask Mask.
			 * @param flags Flags.
			 */
			void ReadHandleInformation(DWORD mask, DWORD flags);

			/**
			 * @brief Set handle information on the write end.
			 * @param mask Mask.
			 * @param flags Flags.
			 */
			void WriteHandleInformation(DWORD mask, DWORD flags);

			/**
			 * @brief Read HANDLE.
			 * @return Handle.
			 */
			HANDLE ReadHandle() const;

			/**
			 * @brief Write HANDLE.
			 * @return Handle.
			 */
			HANDLE WriteHandle() const;

			/**
			 * @brief Write a string to the write end.
			 * @param str Data.
			 * @return Bytes written.
			 */
			DWORD Write(const std::string& str);

			/**
			 * @brief Read into @p buffer up to @p size bytes.
			 * @param buffer Destination.
			 * @param size Max bytes.
			 * @return Bytes read.
			 */
			DWORD Read(std::vector<CHAR>& buffer, DWORD size) const;
			#endif

			/**
			 * @brief Write @p str in chunks until complete or peer closes.
			 * @param str Data (moved). Empty string succeeds immediately.
			 * @return true if all data was written.
			 */
			bool WriteAtomic(std::string&& str);

			/**
			 * @brief Close the read end.
			 */
			void CloseRead() noexcept;

			/**
			 * @brief Close the write end.
			 */
			void CloseWrite() noexcept;

			/**
			 * @brief Write @p str via Write().
			 * @param str Data.
			 * @return *this.
			 */
			Pipe& operator<<(const std::string& str);

			/**
			 * @brief Read until EOF into @p str.
			 * @param str Destination.
			 * @return Reference to @p str.
			 */
			std::string& operator>>(std::string& str) const;

		private:
			#ifdef WINDOWS
			HANDLE m_fd[2];						///< Read / write handles
			static SECURITY_ATTRIBUTES m_sAttr;	///< Inherit attributes
			#else
			int m_fd[2];						///< Read / write fds
			#endif

			#ifdef UNIX
			/**
			 * @brief Dup2 and close source.
			 * @param src Source fd (set to -1).
			 * @param dst Destination fd.
			 */
			void Bind(int& src, int dst) noexcept;

			/**
			 * @brief Close @p fd if open.
			 * @param fd File descriptor.
			 */
			void Close(int& fd) noexcept;
			#else
			/**
			 * @brief Close @p handle if valid.
			 * @param handle Handle.
			 */
			void Close(HANDLE& handle) noexcept;

			/**
			 * @brief SetHandleInformation wrapper.
			 * @param handle Handle.
			 * @param mask Mask.
			 * @param flags Flags.
			 */
			void HandleInformation(HANDLE handle, DWORD mask, DWORD flags);
			#endif
	};
}
