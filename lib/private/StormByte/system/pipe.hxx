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

#include <memory>
#include <optional>
#include <string>
#include <vector>

#ifdef WINDOWS
#include <windows.h>
#else
#include <poll.h>
#endif

/**
 * @namespace System
 * @brief System utilities: processes, pipes, environment variables.
 */
namespace StormByte::System {
	/**
	 * @class Pipe
	 * @brief Cross-platform anonymous pipe for process IPC.
	 *
	 * UNIX: pipe(2) / pipe2; Windows: CreatePipe. Move-only.
	 */
	class STORMBYTE_SYSTEM_PRIVATE Pipe {
		public:
			/**
			 * Maximum bytes per read operation (4 MiB).
			 */
			static constexpr const size_t MAX_READ_BYTES = 4 * 1024 * 1024;

			/**
			 * Creates a new pipe pair.
			 */
			Pipe();

			/**
			 * Copy constructor (deleted).
			 */
			Pipe(const Pipe&) = delete;

			/**
			 * Move constructor.
			 */
			Pipe(Pipe&&) = default;

			/**
			 * Copy assignment (deleted).
			 */
			Pipe& operator=(const Pipe&) = delete;

			/**
			 * Move assignment.
			 */
			Pipe& operator=(Pipe&&) = default;

			/**
			 * Closes both ends.
			 */
			~Pipe() noexcept;

			#ifdef UNIX
			/**
			 * Dup2 read end onto @p fd.
			 * @param fd Destination file descriptor.
			 */
			void BindRead(int fd) noexcept;

			/**
			 * Binds read end to another pipe's read side (if implemented in .cxx).
			 * @param fd Other pipe.
			 */
			void BindRead(Pipe& fd) noexcept;

			/**
			 * Dup2 write end onto @p fd.
			 * @param fd Destination file descriptor.
			 */
			void BindWrite(int fd) noexcept;

			/**
			 * Binds write end to another pipe.
			 * @param fd Other pipe.
			 */
			void BindWrite(Pipe& fd) noexcept;

			/**
			 * Writes a string to the write end.
			 * @param str Data.
			 * @return Bytes written.
			 */
			ssize_t Write(const std::string& str);

			/**
			 * @return true if the write end is no longer writable (HUP/ERR).
			 */
			bool WriteEOF() const;

			/**
			 * Reads into @p buffer up to @p size bytes.
			 * @param buffer Destination.
			 * @param size Max bytes.
			 * @return Bytes read.
			 */
			ssize_t Read(std::vector<char>& buffer, ssize_t size) const;

			/**
			 * @return true if the read end reports HUP/ERR.
			 */
			bool ReadEOF() const;
			#else
			/**
			 * Sets handle information on the read end.
			 * @param mask Mask.
			 * @param flags Flags.
			 */
			void ReadHandleInformation(DWORD mask, DWORD flags);

			/**
			 * Sets handle information on the write end.
			 * @param mask Mask.
			 * @param flags Flags.
			 */
			void WriteHandleInformation(DWORD mask, DWORD flags);

			/**
			 * @return Read HANDLE.
			 */
			HANDLE ReadHandle() const;

			/**
			 * @return Write HANDLE.
			 */
			HANDLE WriteHandle() const;

			/**
			 * Writes a string to the write end.
			 * @param str Data.
			 * @return Bytes written.
			 */
			DWORD Write(const std::string& str);

			/**
			 * Reads into @p buffer up to @p size bytes.
			 * @param buffer Destination.
			 * @param size Max bytes.
			 * @return Bytes read.
			 */
			DWORD Read(std::vector<CHAR>& buffer, DWORD size) const;
			#endif

			/**
			 * Writes @p str in chunks until complete or peer closes.
			 * @param str Data (moved).
			 * @return true if all data was written.
			 */
			bool WriteAtomic(std::string&& str);

			/**
			 * Closes the read end.
			 */
			void CloseRead() noexcept;

			/**
			 * Closes the write end.
			 */
			void CloseWrite() noexcept;

			/**
			 * Writes @p str via Write().
			 * @param str Data.
			 * @return *this.
			 */
			Pipe& operator<<(const std::string& str);

			/**
			 * Reads until EOF into @p str.
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
			 * Dup2 and close source.
			 * @param src Source fd (set to closed).
			 * @param dst Destination fd.
			 */
			void Bind(int& src, int dst) noexcept;

			/**
			 * Closes @p fd if open.
			 * @param fd File descriptor.
			 */
			void Close(int& fd) noexcept;
			#else
			/**
			 * Closes @p handle if valid.
			 * @param handle Handle.
			 */
			void Close(HANDLE& handle) noexcept;

			/**
			 * SetHandleInformation wrapper.
			 * @param handle Handle.
			 * @param mask Mask.
			 * @param flags Flags.
			 */
			void HandleInformation(HANDLE handle, DWORD mask, DWORD flags);
			#endif
	};
}
