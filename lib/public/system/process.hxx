#pragma once

#include <system/pipe.hxx>

#include <filesystem>
#include <iostream>
#include <thread>
#ifdef LINUX
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <vector>

/**
 * @namespace System
 * @brief All the classes for handling system exceptions
 */
namespace StormByte::System {
	/**
	 * @struct _EoF
	 * @brief End of file struct
	 */
	struct {} typedef _EoF;

	/**
	 * End of file constant
	 */
	static constexpr const _EoF EoF = {};

	/**
	 * @class Process
	 * @brief Process class for running external programs
	 * They will run immediately after creation
	 */
	class STORMBYTE_SYSTEM_PUBLIC Process {
		public:
			/**
			 * Constructor
			 * @param prog program
			 * @param args arguments
			 */
			Process(const std::filesystem::path& prog, const std::vector<std::string>& args = std::vector<std::string>());

			/**
			 * Constructor
			 * @param prog program
			 * @param args arguments
			 */
			Process(std::filesystem::path&& prog, std::vector<std::string>&& args = std::vector<std::string>());

			/**
			 * Copy constructor
			 */
			Process(const Process&)				= delete;

			/**
			 * Move constructor
			 */
			Process(Process&&)					= default;

			/**
			 * Assignment operator
			 */
			Process& operator=(const Process&)	= delete;

			/**
			 * Move assignment operator
			 */
			Process& operator=(Process&&)		= default;

			/**
			 * Destructor
			 */
			virtual ~Process() noexcept;
			#ifdef LINUX
			/**
			 * Waits for the process to finish
			 * @return exit code
			 */
			int wait() noexcept;

			/**
			 * Gets the process id
			 * @return process id
			 */
			pid_t get_pid() noexcept;
			#else
			/**
			 * Waits for the process to finish
			 * @return exit code
			 */
			DWORD wait() noexcept;

			/**
			 * Gets the process id
			 * @return process id
			 */
			PROCESS_INFORMATION get_pid();
			#endif
			/**
			 * Suspends the process
			 */
			void suspend();

			/**
			 * Resumes the process
			 */
			void resume();

			/**
			 * Binds current process stdout to a process stdin
			 * @param proc target Process
			 * @return Process reference
			 */
			Process& operator>>(Process& proc);

			/**
			 * Outputs the process stdout to a string
			 * @param str string
			 * @return string reference
			 */
			std::string& operator>>(std::string& str) const;

			/**
			 * Outputs the process stdout to an ostream
			 * @param ostream ostream
			 * @param proc Process
			 * @return ostream reference
			 */
			friend STORMBYTE_SYSTEM_PUBLIC std::ostream& operator<<(std::ostream& ostream, const Process& proc);

			/**
			 * Writes to the process stdin
			 * @param str string
			 * @return Process reference
			 */
			Process& operator<<(const std::string& str);

			/**
			 * Writes EOF to the process stdin
			 * @param eof EoF
			 */
			void operator<<(const System::_EoF& eof);

			/**
			 * Process status
			 * @enum Status
			 * @brief Process status
			 */
			enum class Status:unsigned short {
				RUNNING,	///< Process is running
				SUSPENDED,	///< Process is suspended
				TERMINATED	///< Process is terminated
			};

		protected:
			/**
			 * Process status
			 */
			Status m_status;								///< Process status
			#ifdef LINUX
			/**
			 * Process id
			 */
			pid_t m_pid;									///< Process id
			#else
			/**
			 * Process handles
			 */
			STARTUPINFO m_siStartInfo;						///< Startup information

			/**
			 * Process information
			 */
			PROCESS_INFORMATION m_piProcInfo;				///< Process information
			#endif
			/**
			 * Process pipes
			 */
			Pipe m_pstdout;									///< Standard output pipe
			Pipe m_pstdin;									///< Standard input pipe
			Pipe m_pstderr;									///< Standard error pipe
			
		private:
			/**
			 * Sends a string to the process stdin
			 * @param str string
			 */
			void send(const std::string& str);

			/**
			 * Runs the process
			 */
			void run();

			/**
			 * Consumes current process stdout and forwards it to another process stdin until it finishes
			 * @param exec Process
			 */
			void consume_and_forward(Process&);
			#ifdef WINDOWS
			/**
			 * Gets the full command
			 * @return full command
			 */
			std::wstring full_command() const;
			#endif

			/**
			 * Program path
			 */
			std::filesystem::path m_program;

			/**
			 * Program arguments
			 */
			std::vector<std::string> m_arguments;

			/**
			 * stdin to stdout forwarder thread
			 */
			std::unique_ptr<std::thread> m_forwarder;
	};
	/**
	 * Outputs the process to an ostream
	 * @param ostream ostream
	 * @param proc Process
	 * @return ostream reference
	 */
	STORMBYTE_SYSTEM_PUBLIC std::ostream& operator<<(std::ostream&, const Process&);
}