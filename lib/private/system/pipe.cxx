#include <system/pipe.hxx>

using namespace StormByte::System;

#ifdef LINUX
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#else
SECURITY_ATTRIBUTES Pipe::m_sAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
#endif
#include <vector>

Pipe::Pipe() {
	#ifdef LINUX
	signal(SIGPIPE, SIG_IGN);
	(void)pipe2(m_fd, O_CLOEXEC);
	#else
	CreatePipe(&m_fd[0], &m_fd[1], &m_sAttr, 0);
	#endif
}

Pipe::~Pipe() noexcept {
	close_read();
	close_write();
}

#ifdef LINUX
void Pipe::bind_read(int dest) noexcept {
	bind(m_fd[0], dest);
}

void Pipe::bind_write(int dest) noexcept {
	bind(m_fd[1], dest);
}

ssize_t Pipe::write(const std::string& data) {
	return ::write(m_fd[1], data.c_str(), sizeof(char) * data.length());
}

bool Pipe::write_eof() const {
	pollfd poll_data;
	poll_data.fd = m_fd[1];
	poll_data.events = POLLOUT;
	poll(&poll_data, 1, -1);

	return !((poll_data.revents & POLLOUT) == POLLOUT) || ((poll_data.revents & POLLERR) == POLLERR);
}

ssize_t Pipe::read(std::vector<char>& buffer, ssize_t bytes) const {
	return ::read(m_fd[0], &buffer[0], bytes);
}

bool Pipe::read_eof() const {
	pollfd poll_data;
	poll_data.fd = m_fd[0];
	poll_data.events = POLLIN;
	poll(&poll_data, 1, -1);

	return ((poll_data.revents & POLLHUP) == POLLHUP) || ((poll_data.revents & POLLERR) == POLLERR);
}
#else
void Pipe::set_read_handle_information(DWORD mask, DWORD flags) {
	set_handle_information(m_fd[0], mask, flags);
}

void Pipe::set_write_handle_information(DWORD mask, DWORD flags) {
	set_handle_information(m_fd[1], mask, flags);
}

HANDLE Pipe::get_read_handle() const {
	return m_fd[0];
}

HANDLE Pipe::get_write_handle() const {
	return m_fd[1];
}

DWORD Pipe::write(const std::string& data) {
	DWORD dwWritten;
	WriteFile(m_fd[1], data.c_str(), static_cast<DWORD>(sizeof(char) * data.length()), &dwWritten, NULL);
	return dwWritten;
}

DWORD Pipe::read(std::vector<CHAR>& buffer, DWORD size) const {
	DWORD dwRead;
	ReadFile(m_fd[0], buffer.data(), size, &dwRead, NULL);
	return dwRead;
}
#endif

/** This function will write chunks until write HUPs taking ownership    **/
/** of the provided data to write. Empty parameter is Undefined Behavior **/
#ifdef LINUX
bool Pipe::write_atomic(std::string&& data) {
	std::string out = std::move(data);
	bool can_continue;

	do {
		size_t chunk_size = (out.length() > PIPE_BUF) ? PIPE_BUF : out.length(), bytes_written = 0;
		bytes_written = ::write(m_fd[1], out.c_str(), chunk_size);
		out.erase(0, chunk_size);
		can_continue = (chunk_size == bytes_written) && !write_eof();
	} while (!out.empty() && can_continue);
	return out.empty();
}
#else
bool Pipe::write_atomic(std::string&& data) {
	std::string out = std::move(data);
	bool can_continue;

	do {
		size_t chunk_size = (out.length() > 4096) ? 4096 : out.length(), bytes_written = 0;
		DWORD dwWritten;
		WriteFile(m_fd[1], out.c_str(), static_cast<DWORD>(chunk_size), &dwWritten, NULL);
		out.erase(0, chunk_size);
		can_continue = chunk_size == dwWritten;
	} while (!out.empty() && can_continue);
	return out.empty();
}
#endif

void Pipe::close_read() noexcept {
	close(m_fd[0]);
}

void Pipe::close_write() noexcept {
	close(m_fd[1]);
}

Pipe& Pipe::operator<<(const std::string& data) {
	write(data);
	return *this;
}

std::string& Pipe::operator>>(std::string& out) const {
	#ifdef LINUX
	ssize_t bytes;
	#else
	DWORD bytes;
	#endif
	std::vector<char> buffer(MAX_READ_BYTES);
	while((bytes = read(buffer, MAX_READ_BYTES))) {
		if (bytes > 0) {
			out += std::string(&buffer[0], bytes);
		}
	}
	return out;
}

#ifdef LINUX
void Pipe::bind(int& src, int dest) noexcept {
	dup2(src, dest);
	close(src);
}

void Pipe::close(int& fd) noexcept {
	::close(fd);
}
#else
void Pipe::close(HANDLE& fd) noexcept {
	CloseHandle(fd);
}

void Pipe::set_handle_information(HANDLE handle, DWORD mask, DWORD flags) {
	SetHandleInformation(handle, mask, flags);
}
#endif

