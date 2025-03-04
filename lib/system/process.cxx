#include <system/process.hxx>

#ifdef LINUX
#include <sys/wait.h>
#else
#include <tlhelp32.h>
#endif

using namespace StormByte::System;

Process::Process(const std::filesystem::path& prog, const std::vector<std::string>& args):m_status(Status::RUNNING), m_program(prog), m_arguments(args) {
	run();
}

Process::Process(std::filesystem::path&& prog, std::vector<std::string>&& args):m_status(Status::RUNNING), m_program(std::move(prog)), m_arguments(std::move(args)) {
	run();
}

Process::~Process() noexcept {
	wait();
}

Process& Process::operator>>(Process& exe) {
	consume_and_forward(exe);
	return exe;
}

std::string& Process::operator>>(std::string& data) const {
	m_pstdout >> data;
	return data;
}

std::ostream& StormByte::System::operator<<(std::ostream& os, const Process& exe) {
	std::string data;
	exe.m_pstdout >> data;
	return os << data;
}

Process& Process::operator<<(const std::string& data) {
	m_pstdin << data;
	return *this;
}

void Process::operator<<(const System::_EoF&) {
	m_pstdin.close_write();
}

void Process::run() {
	#ifdef LINUX
	m_pid = fork();

	if (m_pid == 0) {
		/* STDIN: Child reads from STDIN but does not write to */
		m_pstdin.close_write();
		m_pstdin.bind_read(STDIN_FILENO);

		/* STDOUT: Child writes to STDOUT but does not read from */
		m_pstdout.close_read();
		m_pstdout.bind_write(STDOUT_FILENO);

		/* STDERR: Child writes to STDERR but does not read from */
		m_pstderr.close_read();
		m_pstderr.bind_write(STDERR_FILENO);

		std::vector<char*> argv;
		argv.reserve(m_arguments.size() + 2);
		argv.push_back(const_cast<char*>(m_program.c_str()));
		for (size_t i = 0; i < m_arguments.size(); i++)
			argv.push_back(m_arguments[i].data());
		argv.push_back(NULL);
		
		execvp(m_program.c_str(), argv.data());
		// If we reach here then we failed to execute the program
		exit(0);
	}
	else {
		/* STDIN: Parent writes to STDIN but does not read from */
		m_pstdin.close_read();

		/* STDOUT: Parent reads from to STDOUT but does not write to */
		m_pstdout.close_write();

		/* STDERR: Parent reads from to STDERR but does not write to */
		m_pstderr.close_write();
	}
	#else
	ZeroMemory(&m_piProcInfo,	sizeof(PROCESS_INFORMATION));
	ZeroMemory(&m_siStartInfo,	sizeof(STARTUPINFO));
	m_siStartInfo.cb = sizeof(STARTUPINFO);
	m_siStartInfo.hStdError = m_pstderr.get_write_handle();
	m_siStartInfo.hStdOutput = m_pstdout.get_write_handle();
	m_siStartInfo.hStdInput = m_pstdin.get_read_handle();
	m_siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	m_pstdout.set_read_handle_information(HANDLE_FLAG_INHERIT, FALSE);
	m_pstderr.set_read_handle_information(HANDLE_FLAG_INHERIT, FALSE);
	m_pstdin.set_write_handle_information(HANDLE_FLAG_INHERIT, FALSE);

	std::wstring command = full_command();
	TCHAR* szCmdline = const_cast<TCHAR*>(command.c_str());

	if (CreateProcess(	NULL,
						szCmdline,			// command line 
						NULL,				// process security attributes 
						NULL,				// primary thread security attributes 
						TRUE,				// handles are inherited 
						CREATE_NO_WINDOW,	// creation flags 
						NULL,				// use parent's environment 
						NULL,				// use parent's current directory 
						&m_siStartInfo,		// STARTUPINFO pointer 
						&m_piProcInfo)) {
		// Set the rest of handles not inheritable by other execs
		m_pstdout.set_write_handle_information(HANDLE_FLAG_INHERIT, 0);
		m_pstderr.set_write_handle_information(HANDLE_FLAG_INHERIT, 0);
		m_pstdin.set_read_handle_information(HANDLE_FLAG_INHERIT, 0);
	
		// Close handles to the stdin and stdout pipes no longer needed by the child process.
		// If they are not explicitly closed, there is no way to recognize that the child process has ended.
		m_pstdout.close_write();
		m_pstderr.close_write();
		m_pstdin.close_read();
	}
	#endif
}

void Process::send(const std::string& str) {
	m_pstdin << str;
}

#ifdef LINUX
int Process::wait() noexcept {
	int status;
	if (m_forwarder) {
		m_forwarder->join();
		m_forwarder.reset();
	}
	waitpid(m_pid, &status, 0);
	m_status = Status::TERMINATED;
	return WEXITSTATUS(status);
}

pid_t Process::get_pid() noexcept {
	return m_pid;
}
#else
DWORD Process::wait() noexcept {
	DWORD status;
	if (m_forwarder) {
		m_forwarder->join();
		m_forwarder.reset();
	}
	WaitForSingleObject(m_piProcInfo.hProcess, INFINITE);
	GetExitCodeThread(m_piProcInfo.hThread, &status);

	CloseHandle(m_piProcInfo.hProcess);
	CloseHandle(m_piProcInfo.hThread);
	m_status = Status::TERMINATED;
	return status;
}

PROCESS_INFORMATION Process::get_pid() {
	return m_piProcInfo;
}
#endif

void Process::suspend() {
	#ifdef LINUX
	::kill(m_pid, SIGSTOP);
	#else
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE) {
		return;
	}

	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	if (Thread32First(hThreadSnap, &te32)) {
		do {
			if (te32.th32OwnerProcessID == m_piProcInfo.dwProcessId) {
				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
				if (hThread != NULL) {
					SuspendThread(hThread);
					CloseHandle(hThread);
				}
			}
		} while (Thread32Next(hThreadSnap, &te32));
	}
	CloseHandle(hThreadSnap);
	#endif
	m_status = Status::SUSPENDED;
}

void Process::resume() {
	#ifdef LINUX
	::kill(m_pid, SIGCONT);
	#else
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE) {
		return;
	}

	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	if (Thread32First(hThreadSnap, &te32)) {
		do {
			if (te32.th32OwnerProcessID == m_piProcInfo.dwProcessId) {
				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
				if (hThread != NULL) {
					ResumeThread(hThread);
					CloseHandle(hThread);
				}
			}
		} while (Thread32Next(hThreadSnap, &te32));
	}
	CloseHandle(hThreadSnap);
	#endif
	m_status = Status::RUNNING;
}

void Process::consume_and_forward(Process& exec) {
	m_forwarder = std::make_unique<std::thread>(
		[&]{
			#ifdef LINUX
			std::vector<char> buffer(Pipe::MAX_READ_BYTES);
			ssize_t bytes_read;
			bool chunks_written = true;
			do {
				bytes_read = m_pstdout.read(buffer, Pipe::MAX_READ_BYTES);
				if (bytes_read > 0) {
					chunks_written = exec.m_pstdin.write_atomic(std::string(buffer.data(), bytes_read));
				}
			} while (!m_pstdout.read_eof() && chunks_written);
			exec.m_pstdin.close_write();

			/* If chunks_written is false then it means that target executable */
			/* already processed our input and closed connection, so we assume */
			/* that this process is no longer needed, we send SIGTERM because  */
			/* since we did not directly connected the IPC pipes in-out, this  */
			/* process can't know the other one closed its stdin.              */
			/* Furthermore, output is consumed (and ignored) to unlock current */
			/* process in case it is locked in a pipe write operation          */
			if (!chunks_written) {
				kill(m_pid, SIGTERM);
				while(!m_pstdout.read_eof()) {
					std::vector<char> buffer;
					buffer.reserve(Pipe::MAX_READ_BYTES);
					m_pstdout.read(buffer, Pipe::MAX_READ_BYTES);
				}
			}
			#else
			DWORD status;
			std::vector<CHAR> buffer(Pipe::MAX_READ_BYTES);
			SSIZE_T bytes_read;
			bool chunks_written = true;
			do {
				bytes_read = m_pstdout.read(buffer, Pipe::MAX_READ_BYTES);
				if (bytes_read > 0) {
					chunks_written = exec.m_pstdin.write_atomic(std::string(buffer.data(), bytes_read));
				}
				status = WaitForSingleObject(m_piProcInfo.hProcess, 0);
			} while (chunks_written && status == WAIT_TIMEOUT);
			/* See Linux version comment above, except that in Windows we don't need */
			/* to consume exceeding output before program can exit gracefully        */
			if (!chunks_written) {
				TerminateProcess(m_piProcInfo.hProcess, 0);
			}
			exec.m_pstdin.close_write();
			#endif
		}
	);
}

#ifdef WINDOWS
std::wstring Process::full_command() const {
	std::stringstream ss;

	std::vector<std::string> full = { m_program.string() };
	full.insert(full.end(), m_arguments.begin(), m_arguments.end());
	std::copy(full.begin(), full.end(), std::ostream_iterator<std::string>(ss, " "));
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, ss.str().c_str(), -1, NULL, 0);
	std::unique_ptr<wchar_t[]> wstr_buff = std::make_unique<wchar_t[]>(wchars_num);
	MultiByteToWideChar(CP_UTF8, 0, ss.str().c_str(), -1, wstr_buff.get(), wchars_num);
	return std::wstring(wstr_buff.get(), wchars_num);
}
#endif