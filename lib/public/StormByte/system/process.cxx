#include <StormByte/system/exception.hxx>
#include <StormByte/system/pipe.hxx>
#include <StormByte/system/process.hxx>

#ifdef UNIX
#include <sys/wait.h>
#include <signal.h>
#include <cstdlib>
#else
#include <tlhelp32.h>
#include <sstream>
#include <iterator>
#endif

using namespace StormByte::System;

Process::Process(const std::filesystem::path& prog, const std::vector<std::string>& args):
	m_status(Status::RUNNING),
#ifdef UNIX
	m_pid(-1),
#endif
	m_pstdout(std::make_unique<Pipe>()),
	m_pstdin(std::make_unique<Pipe>()),
	m_pstderr(std::make_unique<Pipe>()),
	m_program(prog),
	m_arguments(args) {
#ifdef WINDOWS
	ZeroMemory(&m_siStartInfo, sizeof(STARTUPINFOW));
	ZeroMemory(&m_piProcInfo, sizeof(PROCESS_INFORMATION));
#endif
	Run();
}

Process::Process(std::filesystem::path&& prog, std::vector<std::string>&& args):
	m_status(Status::RUNNING),
#ifdef UNIX
	m_pid(-1),
#endif
	m_pstdout(std::make_unique<Pipe>()),
	m_pstdin(std::make_unique<Pipe>()),
	m_pstderr(std::make_unique<Pipe>()),
	m_program(std::move(prog)),
	m_arguments(std::move(args)) {
#ifdef WINDOWS
	ZeroMemory(&m_siStartInfo, sizeof(STARTUPINFOW));
	ZeroMemory(&m_piProcInfo, sizeof(PROCESS_INFORMATION));
#endif
	Run();
}

void Process::ReleaseOwnership() noexcept {
#ifdef UNIX
	m_pid = -1;
#else
	ZeroMemory(&m_piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&m_siStartInfo, sizeof(STARTUPINFOW));
#endif
	m_status = Status::TERMINATED;
	m_pstdout.reset();
	m_pstdin.reset();
	m_pstderr.reset();
	m_forwarder.reset();
}

Process::Process(Process&& proc) noexcept:
	m_status(proc.m_status),
#ifdef UNIX
	m_pid(proc.m_pid),
#else
	m_siStartInfo(proc.m_siStartInfo),
	m_piProcInfo(proc.m_piProcInfo),
#endif
	m_pstdout(std::move(proc.m_pstdout)),
	m_pstdin(std::move(proc.m_pstdin)),
	m_pstderr(std::move(proc.m_pstderr)),
	m_program(std::move(proc.m_program)),
	m_arguments(std::move(proc.m_arguments)),
	m_forwarder(std::move(proc.m_forwarder)) {
	proc.ReleaseOwnership();
}

Process& Process::operator=(Process&& proc) noexcept {
	if (this != &proc) {
		Wait();
		m_status = proc.m_status;
#ifdef UNIX
		m_pid = proc.m_pid;
#else
		m_siStartInfo = proc.m_siStartInfo;
		m_piProcInfo = proc.m_piProcInfo;
#endif
		m_pstdout = std::move(proc.m_pstdout);
		m_pstdin = std::move(proc.m_pstdin);
		m_pstderr = std::move(proc.m_pstderr);
		m_program = std::move(proc.m_program);
		m_arguments = std::move(proc.m_arguments);
		m_forwarder = std::move(proc.m_forwarder);
		proc.ReleaseOwnership();
	}
	return *this;
}

Process::~Process() noexcept {
	Wait();
	m_pstdout.reset();
	m_pstdin.reset();
	m_pstderr.reset();
#ifdef WINDOWS
	ZeroMemory(&m_siStartInfo, sizeof(STARTUPINFOW));
	ZeroMemory(&m_piProcInfo, sizeof(PROCESS_INFORMATION));
#endif
}

Process& Process::operator>>(Process& exe) {
	ConsumeAndForward(exe);
	return exe;
}

std::string& Process::operator>>(std::string& data) const {
	if (m_pstdout)
		*m_pstdout >> data;
	return data;
}

std::string& Process::Stderr(std::string& str) const {
	if (m_pstderr)
		*m_pstderr >> str;
	return str;
}

std::ostream& StormByte::System::operator<<(std::ostream& os, const Process& exe) {
	std::string data;
	if (exe.m_pstdout)
		*exe.m_pstdout >> data;
	return os << data;
}

Process& Process::operator<<(const std::string& data) {
	if (m_pstdin)
		*m_pstdin << data;
	return *this;
}

void Process::operator<<(const System::_EoF&) {
	if (m_pstdin)
		m_pstdin->CloseWrite();
}

void Process::Run() {
#ifdef UNIX
	m_pid = fork();

	if (m_pid == 0) {
		m_pstdin->CloseWrite();
		m_pstdin->BindRead(STDIN_FILENO);

		m_pstdout->CloseRead();
		m_pstdout->BindWrite(STDOUT_FILENO);

		m_pstderr->CloseRead();
		m_pstderr->BindWrite(STDERR_FILENO);

		std::vector<char*> argv;
		argv.reserve(m_arguments.size() + 2);
		argv.push_back(const_cast<char*>(m_program.c_str()));
		for (size_t i = 0; i < m_arguments.size(); i++)
			argv.push_back(m_arguments[i].data());
		argv.push_back(nullptr);

		execvp(m_program.c_str(), argv.data());
		// Child must not throw across fork boundary
		_exit(127);
	} else if (m_pid > 0) {
		m_pstdin->CloseRead();
		m_pstdout->CloseWrite();
		m_pstderr->CloseWrite();
	} else {
		m_status = Status::TERMINATED;
		throw ExecutableNotFound(m_program);
	}
#else
	ZeroMemory(&m_piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&m_siStartInfo, sizeof(STARTUPINFOW));
	m_siStartInfo.cb = sizeof(STARTUPINFOW);
	m_siStartInfo.hStdError = m_pstderr->WriteHandle();
	m_siStartInfo.hStdOutput = m_pstdout->WriteHandle();
	m_siStartInfo.hStdInput = m_pstdin->ReadHandle();
	m_siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	m_pstdout->ReadHandleInformation(HANDLE_FLAG_INHERIT, FALSE);
	m_pstderr->ReadHandleInformation(HANDLE_FLAG_INHERIT, FALSE);
	m_pstdin->WriteHandleInformation(HANDLE_FLAG_INHERIT, FALSE);

	std::wstring command = FullCommand();
	std::vector<wchar_t> cmdline(command.begin(), command.end());
	cmdline.push_back(L'\0');
	LPWSTR szCmdline = cmdline.data();

	if (CreateProcessW(NULL,
			szCmdline,
			NULL,
			NULL,
			TRUE,
			CREATE_NO_WINDOW,
			NULL,
			NULL,
			&m_siStartInfo,
			&m_piProcInfo)) {
		m_pstdout->WriteHandleInformation(HANDLE_FLAG_INHERIT, 0);
		m_pstderr->WriteHandleInformation(HANDLE_FLAG_INHERIT, 0);
		m_pstdin->ReadHandleInformation(HANDLE_FLAG_INHERIT, 0);

		m_pstdout->CloseWrite();
		m_pstderr->CloseWrite();
		m_pstdin->CloseRead();
	} else {
		m_status = Status::TERMINATED;
		throw ExecutableNotFound(m_program);
	}
#endif
}

void Process::Send(const std::string& str) {
	if (m_pstdin)
		*m_pstdin << str;
}

#ifdef UNIX
int Process::Wait() noexcept {
	if (m_status == Status::TERMINATED || m_pid <= 0)
		return -1;

	if (m_forwarder) {
		m_forwarder->join();
		m_forwarder.reset();
	}

	int status = 0;
	if (waitpid(m_pid, &status, 0) == -1) {
		m_status = Status::TERMINATED;
		m_pid = -1;
		return -1;
	}

	m_status = Status::TERMINATED;
	m_pid = -1;

	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	return -1;
}

pid_t Process::Pid() noexcept {
	return m_pid;
}
#else
DWORD Process::Wait() noexcept {
	if (m_status == Status::TERMINATED || m_piProcInfo.hProcess == nullptr)
		return static_cast<DWORD>(-1);

	if (m_forwarder) {
		m_forwarder->join();
		m_forwarder.reset();
	}

	DWORD exitCode = 0;
	if (WaitForSingleObject(m_piProcInfo.hProcess, INFINITE) == WAIT_FAILED) {
		m_status = Status::TERMINATED;
		return static_cast<DWORD>(-1);
	}

	if (!GetExitCodeProcess(m_piProcInfo.hProcess, &exitCode)) {
		m_status = Status::TERMINATED;
		return static_cast<DWORD>(-1);
	}

	CloseHandle(m_piProcInfo.hProcess);
	CloseHandle(m_piProcInfo.hThread);
	ZeroMemory(&m_piProcInfo, sizeof(PROCESS_INFORMATION));

	m_status = Status::TERMINATED;
	return exitCode;
}

PROCESS_INFORMATION Process::Pid() {
	return m_piProcInfo;
}
#endif

void Process::Suspend() {
#ifdef UNIX
	if (m_pid > 0)
		::kill(m_pid, SIGSTOP);
#else
	if (m_piProcInfo.dwProcessId == 0)
		return;
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return;

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

void Process::Resume() {
#ifdef UNIX
	if (m_pid > 0)
		::kill(m_pid, SIGCONT);
#else
	if (m_piProcInfo.dwProcessId == 0)
		return;
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return;

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

void Process::ConsumeAndForward(Process& exec) {
	m_forwarder = std::make_unique<std::thread>(
		[this, &exec] {
#ifdef UNIX
			std::vector<char> buffer(Pipe::MAX_READ_BYTES);
			ssize_t bytes_read;
			bool chunks_written = true;
			do {
				bytes_read = m_pstdout->Read(buffer, Pipe::MAX_READ_BYTES);
				if (bytes_read > 0)
					chunks_written = exec.m_pstdin->WriteAtomic(std::string(buffer.data(), static_cast<size_t>(bytes_read)));
			} while (!m_pstdout->ReadEOF() && chunks_written);
			exec.m_pstdin->CloseWrite();

			if (!chunks_written) {
				if (m_pid > 0)
					kill(m_pid, SIGTERM);
				while (!m_pstdout->ReadEOF()) {
					std::vector<char> discard(Pipe::MAX_READ_BYTES);
					m_pstdout->Read(discard, Pipe::MAX_READ_BYTES);
				}
			}
#else
			DWORD status;
			std::vector<CHAR> buffer(Pipe::MAX_READ_BYTES);
			DWORD bytes_read;
			bool chunks_written = true;
			do {
				bytes_read = m_pstdout->Read(buffer, static_cast<DWORD>(Pipe::MAX_READ_BYTES));
				if (bytes_read > 0)
					chunks_written = exec.m_pstdin->WriteAtomic(std::string(buffer.data(), bytes_read));
				status = WaitForSingleObject(m_piProcInfo.hProcess, 0);
			} while (chunks_written && status == WAIT_TIMEOUT);

			if (!chunks_written)
				TerminateProcess(m_piProcInfo.hProcess, 0);
			exec.m_pstdin->CloseWrite();
#endif
		}
	);
}

#ifdef WINDOWS
std::wstring Process::FullCommand() const {
	std::stringstream ss;
	std::vector<std::string> full = { m_program.string() };
	full.insert(full.end(), m_arguments.begin(), m_arguments.end());
	for (size_t i = 0; i < full.size(); ++i) {
		if (i)
			ss << ' ';
		ss << full[i];
	}
	const std::string narrow = ss.str();
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, NULL, 0);
	std::unique_ptr<wchar_t[]> wstr_buff = std::make_unique<wchar_t[]>(static_cast<size_t>(wchars_num));
	MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, wstr_buff.get(), wchars_num);
	return std::wstring(wstr_buff.get());
}
#endif
