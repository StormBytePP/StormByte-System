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

#include <StormByte/system/exception.hxx>
#include <StormByte/system/process/implementation.hxx>
#include <StormByte/system/pipe.hxx>
#include <StormByte/system/process.hxx>
#ifdef UNIX
#include <fcntl.h>
#include <sys/wait.h>
#include <cerrno>
#include <cstring>
#include <signal.h>
#include <cstdlib>
#else
#include <tlhelp32.h>
#include <sstream>
#include <iterator>
#endif
using namespace StormByte::System;
Process::Process(const std::filesystem::path& prog, const std::vector<std::string>& args):
	m_implementation(std::make_unique<ProcessImplementation>()) {
	m_implementation->m_status = Status::RUNNING;
	m_implementation->m_pid = -1;
	m_implementation->m_pstdout = std::make_shared<Pipe>();
	m_implementation->m_pstdin = std::make_shared<Pipe>();
	m_implementation->m_pstderr = std::make_shared<Pipe>();
	m_implementation->m_program = prog;
	m_implementation->m_arguments = args;
#ifdef WINDOWS
	ZeroMemory(&m_implementation->m_siStartInfo, sizeof(STARTUPINFOW));
	ZeroMemory(&m_implementation->m_piProcInfo, sizeof(PROCESS_INFORMATION));
#endif
	Run();
}
Process::Process(std::filesystem::path&& prog, std::vector<std::string>&& args):
	m_implementation(std::make_unique<ProcessImplementation>()) {
	m_implementation->m_status = Status::RUNNING;
	m_implementation->m_pid = -1;
	m_implementation->m_pstdout = std::make_shared<Pipe>();
	m_implementation->m_pstdin = std::make_shared<Pipe>();
	m_implementation->m_pstderr = std::make_shared<Pipe>();
	m_implementation->m_program = std::move(prog);
	m_implementation->m_arguments = std::move(args);
#ifdef WINDOWS
	ZeroMemory(&m_implementation->m_siStartInfo, sizeof(STARTUPINFOW));
	ZeroMemory(&m_implementation->m_piProcInfo, sizeof(PROCESS_INFORMATION));
#endif
	Run();
}
void Process::ReleaseOwnership() noexcept {
	if (!m_implementation)
		return;
#ifdef UNIX
	m_implementation->m_pid = -1;
#else
	ZeroMemory(&m_implementation->m_piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&m_implementation->m_siStartInfo, sizeof(STARTUPINFOW));
#endif
	m_implementation->m_status = Status::TERMINATED;
	m_implementation->m_pstdout.reset();
	m_implementation->m_pstdin.reset();
	m_implementation->m_pstderr.reset();
	m_implementation->m_forwarder.reset();
}
void Process::JoinForwarder() noexcept {
	if (!m_implementation || !m_implementation->m_forwarder)
		return;
	try {
		if (m_implementation->m_forwarder->joinable())
			m_implementation->m_forwarder->join();
	} catch (...) {
		try {
			if (m_implementation->m_forwarder->joinable())
				m_implementation->m_forwarder->detach();
		} catch (...) {}
	}
	m_implementation->m_forwarder.reset();
}
Process::Process(Process&& proc) noexcept:
	m_implementation(std::move(proc.m_implementation)) {}
Process& Process::operator=(Process&& proc) noexcept {
	if (this != &proc) {
		Wait();
		m_implementation = std::move(proc.m_implementation);
	}
	return *this;
}
Process::~Process() noexcept {
	Wait();
	if (!m_implementation)
		return;
	m_implementation->m_pstdout.reset();
	m_implementation->m_pstdin.reset();
	m_implementation->m_pstderr.reset();
#ifdef WINDOWS
	ZeroMemory(&m_implementation->m_siStartInfo, sizeof(STARTUPINFOW));
	ZeroMemory(&m_implementation->m_piProcInfo, sizeof(PROCESS_INFORMATION));
#endif
}
Process& Process::operator>>(Process& exe) {
	if (!m_implementation || !exe.m_implementation)
		return exe;
	if (m_implementation->m_forwarder && m_implementation->m_forwarder->joinable()) {
		m_implementation->m_forwarder_cancel->store(true);
		m_implementation->m_pstdout->CloseRead();
		JoinForwarder();
	}
	m_implementation->m_forwarder_cancel = std::make_shared<std::atomic_bool>(false);
	#ifdef UNIX
	const pid_t source_pid = m_implementation->m_pid;
	m_implementation->m_forwarder = std::make_unique<std::thread>(Pipe::Connect(m_implementation->m_pstdout, exe.m_implementation->m_pstdin, m_implementation->m_forwarder_cancel, [source_pid] {
		if (source_pid > 0)
			kill(source_pid, SIGTERM);
	}));
	#else
	const HANDLE source_process = m_implementation->m_piProcInfo.hProcess;
	m_implementation->m_forwarder = std::make_unique<std::thread>(Pipe::Connect(m_implementation->m_pstdout, exe.m_implementation->m_pstdin, m_implementation->m_forwarder_cancel, [source_process] {
		if (source_process != nullptr)
			TerminateProcess(source_process, 0);
	}));
	#endif
	return exe;
}
std::string& Process::operator>>(std::string& data) const {
	if (m_implementation && m_implementation->m_pstdout)
		*m_implementation->m_pstdout >> data;
	return data;
}
std::string& Process::Stderr(std::string& str) const {
	if (m_implementation && m_implementation->m_pstderr)
		*m_implementation->m_pstderr >> str;
	return str;
}
std::ostream& StormByte::System::operator<<(std::ostream& os, const Process& exe) {
	std::string data;
	if (exe.m_implementation && exe.m_implementation->m_pstdout)
		*exe.m_implementation->m_pstdout >> data;
	return os << data;
}
Process& Process::operator<<(const std::string& data) {
	if (m_implementation && m_implementation->m_pstdin)
		*m_implementation->m_pstdin << data;
	return *this;
}
void Process::operator<<(const System::_EoF&) {
	if (m_implementation && m_implementation->m_pstdin)
		m_implementation->m_pstdin->CloseWrite();
}
void Process::Run() {
#ifdef UNIX
	int exec_status[2] = { -1, -1 };
#ifdef LINUX
	if (pipe2(exec_status, O_CLOEXEC) == -1)
		throw ProcessCreationError(std::strerror(errno));
#else
	if (pipe(exec_status) == -1 || fcntl(exec_status[0], F_SETFD, FD_CLOEXEC) == -1 || fcntl(exec_status[1], F_SETFD, FD_CLOEXEC) == -1) {
		const int error = errno;
		if (exec_status[0] != -1)
			close(exec_status[0]);
		if (exec_status[1] != -1)
			close(exec_status[1]);
		throw ProcessCreationError(std::strerror(error));
	}
#endif
	m_implementation->m_pid = fork();
	if (m_implementation->m_pid == 0) {
		close(exec_status[0]);
		auto report_exec_error = [&exec_status] {
			const int error = errno;
			const char* data = reinterpret_cast<const char*>(&error);
			size_t remaining = sizeof(error);
			while (remaining > 0) {
				const ssize_t written = write(exec_status[1], data, remaining);
				if (written > 0) {
					data += written;
					remaining -= static_cast<size_t>(written);
				} else if (written < 0 && errno == EINTR) {
					continue;
				} else {
					break;
				}
			}
		};
		m_implementation->m_pstdin->CloseWrite();
		if (!m_implementation->m_pstdin->BindRead(STDIN_FILENO)) {
			report_exec_error();
			_exit(127);
		}
		m_implementation->m_pstdout->CloseRead();
		if (!m_implementation->m_pstdout->BindWrite(STDOUT_FILENO)) {
			report_exec_error();
			_exit(127);
		}
		m_implementation->m_pstderr->CloseRead();
		if (!m_implementation->m_pstderr->BindWrite(STDERR_FILENO)) {
			report_exec_error();
			_exit(127);
		}
		std::vector<char*> argv;
		argv.reserve(m_implementation->m_arguments.size() + 2);
		argv.push_back(const_cast<char*>(m_implementation->m_program.c_str()));
		for (size_t i = 0; i < m_implementation->m_arguments.size(); i++)
			argv.push_back(m_implementation->m_arguments[i].data());
		argv.push_back(nullptr);
		execvp(m_implementation->m_program.c_str(), argv.data());
		report_exec_error();
		_exit(127);
	} else if (m_implementation->m_pid > 0) {
		close(exec_status[1]);
		m_implementation->m_pstdin->CloseRead();
		m_implementation->m_pstdout->CloseWrite();
		m_implementation->m_pstderr->CloseWrite();
		int child_error = 0;
		char* data = reinterpret_cast<char*>(&child_error);
		size_t remaining = sizeof(child_error);
		while (remaining > 0) {
			const ssize_t bytes_read = read(exec_status[0], data, remaining);
			if (bytes_read > 0) {
				data += bytes_read;
				remaining -= static_cast<size_t>(bytes_read);
			} else if (bytes_read < 0 && errno == EINTR) {
				continue;
			} else {
				break;
			}
		}
		close(exec_status[0]);
		if (remaining == sizeof(child_error))
			return;
		if (remaining == 0) {
			waitpid(m_implementation->m_pid, nullptr, 0);
			m_implementation->m_status = Status::TERMINATED;
			m_implementation->m_pid = -1;
			if (child_error == ENOENT)
				throw ExecutableNotFound(m_implementation->m_program);
			throw ProcessCreationError(std::strerror(child_error));
		}
		waitpid(m_implementation->m_pid, nullptr, 0);
		m_implementation->m_status = Status::TERMINATED;
		m_implementation->m_pid = -1;
		throw ProcessCreationError("Incomplete exec failure status");
	} else {
		close(exec_status[0]);
		close(exec_status[1]);
		m_implementation->m_status = Status::TERMINATED;
		const int error = errno;
		throw ProcessCreationError(std::strerror(error));
	}
#else
	ZeroMemory(&m_implementation->m_piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&m_implementation->m_siStartInfo, sizeof(STARTUPINFOW));
	m_implementation->m_siStartInfo.cb = sizeof(STARTUPINFOW);
	m_implementation->m_siStartInfo.hStdError = m_implementation->m_pstderr->WriteHandle();
	m_implementation->m_siStartInfo.hStdOutput = m_implementation->m_pstdout->WriteHandle();
	m_implementation->m_siStartInfo.hStdInput = m_implementation->m_pstdin->ReadHandle();
	m_implementation->m_siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	m_implementation->m_pstdout->ReadHandleInformation(HANDLE_FLAG_INHERIT, FALSE);
	m_implementation->m_pstderr->ReadHandleInformation(HANDLE_FLAG_INHERIT, FALSE);
	m_implementation->m_pstdin->WriteHandleInformation(HANDLE_FLAG_INHERIT, FALSE);
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
			&m_implementation->m_siStartInfo,
			&m_implementation->m_piProcInfo)) {
		m_implementation->m_pstdout->WriteHandleInformation(HANDLE_FLAG_INHERIT, 0);
		m_implementation->m_pstderr->WriteHandleInformation(HANDLE_FLAG_INHERIT, 0);
		m_implementation->m_pstdin->ReadHandleInformation(HANDLE_FLAG_INHERIT, 0);
		m_implementation->m_pstdout->CloseWrite();
		m_implementation->m_pstderr->CloseWrite();
		m_implementation->m_pstdin->CloseRead();
	} else {
		m_implementation->m_status = Status::TERMINATED;
		const DWORD error = GetLastError();
		if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
			throw ExecutableNotFound(m_implementation->m_program);
		throw ProcessCreationError("CreateProcessW failed with error " + std::to_string(error));
	}
#endif
}
void Process::Send(const std::string& str) {
	if (m_implementation->m_pstdin)
		*m_implementation->m_pstdin << str;
}
#ifdef UNIX
int Process::Wait() noexcept {
	if (!m_implementation || m_implementation->m_status == Status::TERMINATED || m_implementation->m_pid <= 0)
		return -1;
	if (m_implementation->m_forwarder) {
		m_implementation->m_forwarder_cancel->store(true);
		m_implementation->m_pstdout->CloseRead();
	}
	int status = 0;
	pid_t result;
	do {
		result = waitpid(m_implementation->m_pid, &status, 0);
	} while (result == -1 && errno == EINTR);
	if (result == -1) {
		m_implementation->m_status = Status::TERMINATED;
		m_implementation->m_pid = -1;
		if (m_implementation->m_forwarder) {
			JoinForwarder();
		}
		return -1;
	}
	m_implementation->m_status = Status::TERMINATED;
	m_implementation->m_pid = -1;
	if (m_implementation->m_forwarder) {
		JoinForwarder();
	}
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	return -1;
}
int Process::Wait(std::chrono::milliseconds timeout) noexcept {
	if (!m_implementation || m_implementation->m_status == Status::TERMINATED || m_implementation->m_pid <= 0)
		return -1;
	const auto deadline = std::chrono::steady_clock::now() + timeout;
	int status = 0;
	while (true) {
		const pid_t result = waitpid(m_implementation->m_pid, &status, WNOHANG);
		if (result == m_implementation->m_pid) {
			m_implementation->m_status = Status::TERMINATED;
			m_implementation->m_pid = -1;
			if (m_implementation->m_forwarder) {
				m_implementation->m_forwarder_cancel->store(true);
				m_implementation->m_pstdout->CloseRead();
				JoinForwarder();
			}
			return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
		}
		if (result == -1 || std::chrono::steady_clock::now() >= deadline) {
			if (m_implementation->m_forwarder) {
				m_implementation->m_forwarder_cancel->store(true);
				m_implementation->m_pstdout->CloseRead();
			}
			return -1;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
pid_t Process::Pid() noexcept {
	if (!m_implementation)
		return -1;
	return m_implementation->m_pid;
}
#else
DWORD Process::Wait() noexcept {
	if (!m_implementation || m_implementation->m_status == Status::TERMINATED || m_implementation->m_piProcInfo.hProcess == nullptr)
		return static_cast<DWORD>(-1);
	if (m_implementation->m_forwarder) {
		m_implementation->m_forwarder_cancel->store(true);
		m_implementation->m_pstdout->CloseRead();
	}
	DWORD exitCode = 0;
	if (WaitForSingleObject(m_implementation->m_piProcInfo.hProcess, INFINITE) == WAIT_FAILED) {
		CloseHandle(m_implementation->m_piProcInfo.hProcess);
		CloseHandle(m_implementation->m_piProcInfo.hThread);
		ZeroMemory(&m_implementation->m_piProcInfo, sizeof(PROCESS_INFORMATION));
		m_implementation->m_status = Status::TERMINATED;
		return static_cast<DWORD>(-1);
	}
	if (!GetExitCodeProcess(m_implementation->m_piProcInfo.hProcess, &exitCode)) {
		CloseHandle(m_implementation->m_piProcInfo.hProcess);
		CloseHandle(m_implementation->m_piProcInfo.hThread);
		ZeroMemory(&m_implementation->m_piProcInfo, sizeof(PROCESS_INFORMATION));
		m_implementation->m_status = Status::TERMINATED;
		return static_cast<DWORD>(-1);
	}
	CloseHandle(m_implementation->m_piProcInfo.hProcess);
	CloseHandle(m_implementation->m_piProcInfo.hThread);
	ZeroMemory(&m_implementation->m_piProcInfo, sizeof(PROCESS_INFORMATION));
	m_implementation->m_status = Status::TERMINATED;
	if (m_implementation->m_forwarder) {
		JoinForwarder();
	}
	return exitCode;
}
DWORD Process::Wait(std::chrono::milliseconds timeout) noexcept {
	if (!m_implementation || m_implementation->m_status == Status::TERMINATED || m_implementation->m_piProcInfo.hProcess == nullptr)
		return static_cast<DWORD>(-1);
	const auto count = timeout.count() < 0 ? 0 : timeout.count();
	const DWORD wait_result = WaitForSingleObject(m_implementation->m_piProcInfo.hProcess, static_cast<DWORD>(count));
	if (wait_result != WAIT_OBJECT_0) {
		if (m_implementation->m_forwarder) {
			m_implementation->m_forwarder_cancel->store(true);
			m_implementation->m_pstdout->CloseRead();
		}
		return static_cast<DWORD>(-1);
	}
	if (m_implementation->m_forwarder) {
		m_implementation->m_forwarder_cancel->store(true);
		m_implementation->m_pstdout->CloseRead();
		JoinForwarder();
	}
	DWORD exitCode = 0;
	if (!GetExitCodeProcess(m_implementation->m_piProcInfo.hProcess, &exitCode)) {
		CloseHandle(m_implementation->m_piProcInfo.hProcess);
		CloseHandle(m_implementation->m_piProcInfo.hThread);
		ZeroMemory(&m_implementation->m_piProcInfo, sizeof(PROCESS_INFORMATION));
		m_implementation->m_status = Status::TERMINATED;
		return static_cast<DWORD>(-1);
	}
	CloseHandle(m_implementation->m_piProcInfo.hProcess);
	CloseHandle(m_implementation->m_piProcInfo.hThread);
	ZeroMemory(&m_implementation->m_piProcInfo, sizeof(PROCESS_INFORMATION));
	m_implementation->m_status = Status::TERMINATED;
	return exitCode;
}
PROCESS_INFORMATION Process::Pid() {
	if (!m_implementation)
		return PROCESS_INFORMATION{};
	return m_implementation->m_piProcInfo;
}
#endif
void Process::Suspend() {
	if (!m_implementation)
		return;
#ifdef UNIX
	if (m_implementation->m_pid > 0)
		::kill(m_implementation->m_pid, SIGSTOP);
#else
	if (m_implementation->m_piProcInfo.dwProcessId == 0)
		return;
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return;
	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);
	if (Thread32First(hThreadSnap, &te32)) {
		do {
			if (te32.th32OwnerProcessID == m_implementation->m_piProcInfo.dwProcessId) {
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
	m_implementation->m_status = Status::SUSPENDED;
}
void Process::Resume() {
	if (!m_implementation)
		return;
#ifdef UNIX
	if (m_implementation->m_pid > 0)
		::kill(m_implementation->m_pid, SIGCONT);
#else
	if (m_implementation->m_piProcInfo.dwProcessId == 0)
		return;
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return;
	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);
	if (Thread32First(hThreadSnap, &te32)) {
		do {
			if (te32.th32OwnerProcessID == m_implementation->m_piProcInfo.dwProcessId) {
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
	m_implementation->m_status = Status::RUNNING;
}
#ifdef WINDOWS
std::string Process::QuoteWindowsArgument(const std::string& argument) {
	std::string quoted = "\"";
	size_t backslashes = 0;
	for (const char character: argument) {
		if (character == '\\') {
			++backslashes;
			continue;
		}
		if (character == '"')
			quoted.append(backslashes * 2 + 1, '\\');
		else
			quoted.append(backslashes, '\\');
		quoted += character;
		backslashes = 0;
	}
	quoted.append(backslashes * 2, '\\');
	quoted += '"';
	return quoted;
}
std::wstring Process::FullCommand() const {
	std::stringstream ss;
	std::vector<std::string> full = { m_implementation->m_program.string() };
	full.insert(full.end(), m_implementation->m_arguments.begin(), m_implementation->m_arguments.end());
	for (size_t i = 0; i < full.size(); ++i) {
		if (i)
			ss << ' ';
		ss << QuoteWindowsArgument(full[i]);
	}
	const std::string narrow = ss.str();
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, NULL, 0);
	std::unique_ptr<wchar_t[]> wstr_buff = std::make_unique<wchar_t[]>(static_cast<size_t>(wchars_num));
	MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, wstr_buff.get(), wchars_num);
	return std::wstring(wstr_buff.get());
}
#endif
