#pragma once

#include <system/visibility.h>


#include <memory>
#include <optional>
#include <string>
#include <vector>

#ifdef WINDOWS
#include <windows.h>
#else
#include <sys/poll.h>
#endif

/**
 * @namespace System
 * @brief All the classes for handling system exceptions
 */
namespace StormByte::System {
	/**
	 * @class Pipe
	 * @brief Pipe class for interprocess communication
	 */
	class STORMBYTE_SYSTEM_PRIVATE Pipe {
		public:
			/**
			 * Maximum bytes to read
			 */
			static constexpr const size_t MAX_READ_BYTES		= 4 * 1024 * 1024; // 4MiB

			/**
			 * Constructor
			 */
			Pipe();

			/**
			 * Copy constructor
			 */
			Pipe(const Pipe&)				= delete;

			/**
			 * Move constructor
			 */
			Pipe(Pipe&&)					= default;

			/**
			 * Assignment operator
			 */
			Pipe& operator=(const Pipe&)	= delete;

			/**
			 * Move operator
			 */
			Pipe& operator=(Pipe&&)			= default;

			/**
			 * Destructor
			 */
			~Pipe() noexcept;

			#ifdef LINUX
			/**
			 * Binds a file descriptor to the pipe
			 * @param fd file descriptor
			 */
			void bind_read(int fd) noexcept;

			/**
			 * Binds a file descriptor to the pipe
			 * @param fd file descriptor
			 */
			void bind_read(Pipe&) noexcept;

			/**
			 * Binds a file descriptor to the pipe
			 * @param fd file descriptor
			 */
			void bind_write(int fd) noexcept;

			/**
			 * Binds a file descriptor to the pipe
			 * @param fd file descriptor
			 */
			void bind_write(Pipe&) noexcept;

			/**
			 * Writes to the pipe
			 * @param str string
			 * @return bytes written
			 */
			ssize_t write(const std::string& str);

			/**
			 * Writes EOF to the pipe
			 */
			bool write_eof() const;

			/**
			 * Reads from the pipe
			 * @param buffer buffer
			 * @param size size
			 * @return bytes read
			 */
			ssize_t read(std::vector<char>&, ssize_t) const;

			/**
			 * Reads EOF from the pipe
			 */
			bool read_eof() const;
			#else
			/**
			 * Sets the read handle information
			 * @param mask mask
			 * @param flags flags
			 */
			void set_read_handle_information(DWORD mask, DWORD flags);

			/**
			 * Sets the write handle information
			 * @param mask mask
			 * @param flags flags
			 */
			void set_write_handle_information(DWORD mask, DWORD flags);

			/**
			 * Gets the read handle
			 * @return handle
			 */
			HANDLE get_read_handle() const;

			/**
			 * Gets the write handle
			 * @return handle
			 */
			HANDLE get_write_handle() const;

			/**
			 * Writes to the pipe
			 * @param str string
			 * @return bytes written
			 */
			DWORD write(const std::string& str);

			/**
			 * Reads from the pipe
			 * @param buffer buffer
			 * @param size size
			 * @return bytes read
			 */
			DWORD read(std::vector<CHAR>& buffer, DWORD size) const;
			#endif

			/**
			 * Writes to the pipe
			 * @param str string
			 * @return boolean indicating if it was written
			 */
			bool write_atomic(std::string&& str);

			/**
			 * Close read end
			 */
			void close_read() noexcept;

			/**
			 * Close write end
			 */
			void close_write() noexcept;

			/**
			 * Writes to the pipe
			 * @param str string
			 * @return Pipe reference
			 */
			Pipe& operator<<(const std::string& str);

			/**
			 * Reads from the pipe
			 * @param str string
			 * @return Pipe reference
			 */
			std::string& operator>>(std::string& str) const;

		private:
			#ifdef LINUX
			/**
			 * Binds a file descriptor to the pipe
			 * @param fd file descriptor
			 */
			void bind(int&, int) noexcept;

			/**
			 * Closes a file descriptor
			 * @param fd file descriptor
			 */
			void close(int& fd) noexcept;
			#else
			/**
			 * Closes a handle
			 * @param handle handle
			 */
			void close(HANDLE&) noexcept;

			/**
			 * Sets the handle information
			 * @param handle handle
			 * @param str string
			 * @return bytes written
			 */
			void set_handle_information(HANDLE, DWORD, DWORD);
			#endif

			#ifdef WINDOWS
			/**
			 * Handles
			 */
			HANDLE m_fd[2];

			/**
			 * Security attributes
			 */
			static SECURITY_ATTRIBUTES m_sAttr;
			#else
			/**
			 * File descriptors
			 */
			int m_fd[2];
			#endif
			
	};
}