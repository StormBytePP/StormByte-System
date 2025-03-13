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
			Pipe(const Pipe&)									= delete;

			/**
			 * Move constructor
			 */
			Pipe(Pipe&&)										= default;

			/**
			 * Assignment operator
			 */
			Pipe& operator=(const Pipe&)						= delete;

			/**
			 * Move operator
			 */
			Pipe& operator=(Pipe&&)								= default;

			/**
			 * Destructor
			 */
			~Pipe() noexcept;

			#ifdef LINUX
			/**
			 * Binds a file descriptor to the pipe
			 * @param fd file descriptor
			 */
			void 												BindRead(int fd) noexcept;

			/**
			 * Binds a file descriptor to the pipe
			 * @param fd pipe descriptor
			 */
			void 												BindRead(Pipe& fd) noexcept;

			/**
			 * Binds a file descriptor to the pipe
			 * @param fd file descriptor
			 */
			void 												BindWrite(int fd) noexcept;

			/**
			 * Binds a file descriptor to the pipe
			 * @param fd pipe descriptor
			 */
			void 												BindWrite(Pipe& fd) noexcept;

			/**
			 * Writes to the pipe
			 * @param str string
			 * @return bytes written
			 */
			ssize_t 											Write(const std::string& str);

			/**
			 * Writes EOF to the pipe
			 */
			bool 												WriteEOF() const;

			/**
			 * Reads from the pipe
			 * @param buffer buffer
			 * @param size size
			 * @return bytes read
			 */
			ssize_t 											Read(std::vector<char>& buffer, ssize_t size) const;

			/**
			 * Reads EOF from the pipe
			 */
			bool 												ReadEOF() const;
			#else
			/**
			 * Sets the read handle information
			 * @param mask mask
			 * @param flags flags
			 */
			void 												ReadHandleInformation(DWORD mask, DWORD flags);

			/**
			 * Sets the write handle information
			 * @param mask mask
			 * @param flags flags
			 */
			void 												WriteHandleInformation(DWORD mask, DWORD flags);

			/**
			 * Gets the read handle
			 * @return handle
			 */
			HANDLE 												ReadHandle() const;

			/**
			 * Gets the write handle
			 * @return handle
			 */
			HANDLE 												WriteHandle() const;

			/**
			 * Writes to the pipe
			 * @param str string
			 * @return bytes written
			 */
			DWORD 												Write(const std::string& str);

			/**
			 * Reads from the pipe
			 * @param buffer buffer
			 * @param size size
			 * @return bytes read
			 */
			DWORD 												Read(std::vector<CHAR>& buffer, DWORD size) const;
			#endif

			/**
			 * Writes to the pipe
			 * @param str string
			 * @return boolean indicating if it was written
			 */
			bool 												WriteAtomic(std::string&& str);

			/**
			 * Close read end
			 */
			void 												CloseRead() noexcept;

			/**
			 * Close write end
			 */
			void 												CloseWrite() noexcept;

			/**
			 * Writes to the pipe
			 * @param str string
			 * @return Pipe reference
			 */
			Pipe& 												operator<<(const std::string& str);

			/**
			 * Reads from the pipe
			 * @param str string
			 * @return Pipe reference
			 */
			std::string& 										operator>>(std::string& str) const;

		private:
			#ifdef WINDOWS
			HANDLE m_fd[2];										///< File descriptors
			static SECURITY_ATTRIBUTES m_sAttr;					///< Security attributes
			#else
			int m_fd[2];										///< File descriptors
			#endif
			#ifdef LINUX
			/**
			 * Binds a file descriptor to the pipe
			 * @param src file descriptor
			 * @param dst destination
			 */
			void 												Bind(int& src, int dst) noexcept;

			/**
			 * Closes a file descriptor
			 * @param fd file descriptor
			 */
			void 												Close(int& fd) noexcept;
			#else
			/**
			 * Closes a handle
			 * @param handle handle
			 */
			void 												Close(HANDLE& handle) noexcept;

			/**
			 * Sets the handle information
			 * @param handle handle
			 * @param str string
			 * @return bytes written
			 */
			void 												HandleInformation(HANDLE handle, DWORD, DWORD);
			#endif
	};
}