#pragma once

#include <StormByte/system/visibility.h>

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
	class Pipe;												///< Forward declaration of Pipe class
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
			 * Copy constructor (deleted)
			 * @param proc Process
			 */
			Process(const Process& proc)						= delete;

			/**
			 * Move constructor
			 * @param proc Process
			 */
			Process(Process&& proc) noexcept;

			/**
			 * Assignment operator (deleted)
			 * @param proc Process
			 */
			Process& operator=(const Process& proc)				= delete;

			/**
			 * Move assignment operator
			 * @param proc Process
			 */
			Process& operator=(Process&& proc) noexcept;

			/**
			 * Destructor
			 */
			virtual ~Process() noexcept;
			#ifdef LINUX
			/**
			 * Waits for the process to finish
			 * @return exit code
			 */
			int 												Wait() noexcept;

			/**
			 * Gets the process id
			 * @return process id
			 */
			pid_t 												Pid() noexcept;
			#else
			/**
			 * Waits for the process to finish
			 * @return exit code
			 */
			DWORD 												Wait() noexcept;

			/**
			 * Gets the process id
			 * @return process id
			 */
			PROCESS_INFORMATION 								Pid();
			#endif
			/**
			 * Suspends the process
			 */
			void 												Suspend();

			/**
			 * Resumes the process
			 */
			void 												Resume();

			/**
			 * Binds current process stdout to a process stdin
			 * @param proc target Process
			 * @return Process reference
			 */
			Process& 											operator>>(Process& proc);

			/**
			 * Outputs the process stdout to a string
			 * @param str string
			 * @return string reference
			 */
			std::string& 										operator>>(std::string& str) const;

			/**
			 * Outputs the process stdout to an ostream
			 * @param ostream ostream
			 * @param proc Process
			 * @return ostream reference
			 */
			friend STORMBYTE_SYSTEM_PUBLIC std::ostream& 		operator<<(std::ostream& ostream, const Process& proc);

			/**
			 * Writes to the process stdin
			 * @param str string
			 * @return Process reference
			 */
			Process& 											operator<<(const std::string& str);

			/**
			 * Writes EOF to the process stdin
			 * @param eof EoF
			 */
			void 												operator<<(const System::_EoF& eof);

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
			Status m_status;									///< Process status
			#ifdef LINUX
			pid_t m_pid;										///< Process id
			#else
			STARTUPINFOW m_siStartInfo;							///< Startup information (wide)
			PROCESS_INFORMATION m_piProcInfo;					///< Process information
			#endif
			Pipe* m_pstdout;									///< Standard output pipe
			Pipe* m_pstdin;										///< Standard input pipe
			Pipe* m_pstderr;									///< Standard error pipe
			std::filesystem::path m_program;					///< Program path
			std::vector<std::string> m_arguments;				///< Program arguments
			std::unique_ptr<std::thread> m_forwarder;			///< Forwarder thread
			
		private:
			/**
			 * Sends a string to the process stdin
			 * @param str string
			 */
			void 												Send(const std::string& str);

			/**
			 * Runs the process
			 */
			void 												Run();

			/**
			 * Consumes current process stdout and forwards it to another process stdin until it finishes
			 * @param exec Process
			 */
			void 												ConsumeAndForward(Process&);
			#ifdef WINDOWS
			/**
			 * Gets the full command
			 * @return full command
			 */
			std::wstring 										FullCommand() const;
			#endif
	};
	/**
	 * Outputs the process to an ostream
	 * @param ostream ostream
	 * @param proc Process
	 * @return ostream reference
	 */
	STORMBYTE_SYSTEM_PUBLIC std::ostream& 						operator<<(std::ostream&, const Process&);
}