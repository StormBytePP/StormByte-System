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

#include <StormByte/system/pipe.hxx>
#include <StormByte/system/exception.hxx>
#include <cerrno>
#include <cstring>
#ifndef WINDOWS
#include <system_error>
#else
#include <string>
#endif
using namespace StormByte::System;
#ifdef UNIX
#include <fcntl.h>
#include <limits.h>
#include <mutex>
#include <signal.h>
#include <unistd.h>
#else
SECURITY_ATTRIBUTES Pipe::m_sAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
#endif
#include <vector>
Pipe::Pipe():
#ifdef WINDOWS
	m_fd{ INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE } {
#else
	m_fd{ -1, -1 } {
#endif
	#ifdef UNIX
	static std::once_flag sigpipe_once;
	std::call_once(sigpipe_once, [] {
		signal(SIGPIPE, SIG_IGN);
	});
	int result;
	#ifdef LINUX
	result = pipe2(m_fd, O_CLOEXEC);
	#else
	result = pipe(m_fd);
	if (result == 0 && (fcntl(m_fd[0], F_SETFD, FD_CLOEXEC) == -1 || fcntl(m_fd[1], F_SETFD, FD_CLOEXEC) == -1))
		result = -1;
	#endif
	if (result == -1) {
		const int error = errno;
		CloseRead();
		CloseWrite();
		throw ProcessCreationError(std::strerror(error));
	}
	#else
	if (!CreatePipe(&m_fd[0], &m_fd[1], &m_sAttr, 0))
		throw ProcessCreationError("CreatePipe failed with error " + std::to_string(GetLastError()));
	#endif
}
Pipe::Pipe(Pipe&& pipe) noexcept:
#ifdef WINDOWS
	m_fd{ pipe.m_fd[0], pipe.m_fd[1] } {
	pipe.m_fd[0] = INVALID_HANDLE_VALUE;
	pipe.m_fd[1] = INVALID_HANDLE_VALUE;
#else
	m_fd{ pipe.m_fd[0], pipe.m_fd[1] } {
	pipe.m_fd[0] = -1;
	pipe.m_fd[1] = -1;
#endif
}
Pipe& Pipe::operator=(Pipe&& pipe) noexcept {
	if (this == &pipe)
		return *this;
	CloseRead();
	CloseWrite();
	m_fd[0] = pipe.m_fd[0];
	m_fd[1] = pipe.m_fd[1];
#ifdef WINDOWS
	pipe.m_fd[0] = INVALID_HANDLE_VALUE;
	pipe.m_fd[1] = INVALID_HANDLE_VALUE;
#else
	pipe.m_fd[0] = -1;
	pipe.m_fd[1] = -1;
#endif
	return *this;
}
Pipe::~Pipe() noexcept {
	CloseRead();
	CloseWrite();
}
#ifdef UNIX
void Pipe::BindRead(int dest) noexcept {
	Bind(m_fd[0], dest);
}
void Pipe::BindWrite(int dest) noexcept {
	Bind(m_fd[1], dest);
}
ssize_t Pipe::Write(const std::string& data) {
	return write(m_fd[1], data.c_str(), sizeof(char) * data.length());
}
bool Pipe::WriteEOF() const {
	pollfd poll_data;
	poll_data.fd = m_fd[1];
	poll_data.events = POLLOUT;
	poll(&poll_data, 1, -1);
	return !((poll_data.revents & POLLOUT) == POLLOUT) || ((poll_data.revents & POLLERR) == POLLERR);
}
ssize_t Pipe::Read(std::vector<char>& buffer, ssize_t bytes) const {
	return read(m_fd[0], buffer.data(), static_cast<size_t>(bytes));
}
bool Pipe::ReadEOF() const {
	pollfd poll_data;
	poll_data.fd = m_fd[0];
	poll_data.events = POLLIN;
	poll(&poll_data, 1, -1);
	return ((poll_data.revents & POLLHUP) == POLLHUP) || ((poll_data.revents & POLLERR) == POLLERR);
}
#else
void Pipe::ReadHandleInformation(DWORD mask, DWORD flags) {
	HandleInformation(m_fd[0], mask, flags);
}
void Pipe::WriteHandleInformation(DWORD mask, DWORD flags) {
	HandleInformation(m_fd[1], mask, flags);
}
HANDLE Pipe::ReadHandle() const {
	return m_fd[0];
}
HANDLE Pipe::WriteHandle() const {
	return m_fd[1];
}
DWORD Pipe::Write(const std::string& data) {
	DWORD dwWritten = 0;
	WriteFile(m_fd[1], data.c_str(), static_cast<DWORD>(sizeof(char) * data.length()), &dwWritten, NULL);
	return dwWritten;
}
DWORD Pipe::Read(std::vector<CHAR>& buffer, DWORD size) const {
	DWORD dwRead = 0;
	ReadFile(m_fd[0], buffer.data(), size, &dwRead, NULL);
	return dwRead;
}
#endif
#ifdef UNIX
bool Pipe::WriteAtomic(std::string&& data) {
	if (data.empty())
		return true;
	std::string out = std::move(data);
	bool can_continue = true;
	do {
		const size_t chunk_size = (out.length() > static_cast<size_t>(PIPE_BUF)) ? static_cast<size_t>(PIPE_BUF) : out.length();
		const ssize_t bytes_written = ::write(m_fd[1], out.c_str(), chunk_size);
		if (bytes_written < 0 || static_cast<size_t>(bytes_written) != chunk_size) {
			can_continue = false;
			break;
		}
		out.erase(0, chunk_size);
		can_continue = !WriteEOF();
	} while (!out.empty() && can_continue);
	return out.empty();
}
#else
bool Pipe::WriteAtomic(std::string&& data) {
	if (data.empty())
		return true;
	std::string out = std::move(data);
	bool can_continue = true;
	do {
		const size_t chunk_size = (out.length() > 4096) ? 4096 : out.length();
		DWORD dwWritten = 0;
		if (!WriteFile(m_fd[1], out.c_str(), static_cast<DWORD>(chunk_size), &dwWritten, NULL) ||
			dwWritten != static_cast<DWORD>(chunk_size)) {
			can_continue = false;
			break;
		}
		out.erase(0, chunk_size);
	} while (!out.empty() && can_continue);
	return out.empty();
}
#endif
void Pipe::CloseRead() noexcept {
	Close(m_fd[0]);
}
void Pipe::CloseWrite() noexcept {
	Close(m_fd[1]);
}
Pipe& Pipe::operator<<(const std::string& data) {
	Write(data);
	return *this;
}
std::thread Pipe::Connect(Pipe& destination, std::function<void()> on_failure) {
	return std::thread([this, &destination, on_failure = std::move(on_failure)] {
#ifdef UNIX
		std::vector<char> buffer(MAX_READ_BYTES);
		ssize_t bytes_read;
		bool forwarding = true;
		while (forwarding && (bytes_read = Read(buffer, MAX_READ_BYTES)) > 0)
			forwarding = destination.WriteAtomic(std::string(buffer.data(), static_cast<size_t>(bytes_read)));
		if (!forwarding && on_failure)
			on_failure();
#else
		std::vector<CHAR> buffer(MAX_READ_BYTES);
		DWORD bytes_read;
		bool forwarding = true;
		while (forwarding && (bytes_read = Read(buffer, static_cast<DWORD>(MAX_READ_BYTES))) > 0)
			forwarding = destination.WriteAtomic(std::string(buffer.data(), bytes_read));
		if (!forwarding && on_failure)
			on_failure();
#endif
		destination.CloseWrite();
	});
}
std::string& Pipe::operator>>(std::string& out) const {
	#ifdef UNIX
	ssize_t bytes;
	#else
	DWORD bytes;
	#endif
	std::vector<char> buffer(MAX_READ_BYTES);
	while ((bytes = Read(buffer, static_cast<
#ifdef UNIX
		ssize_t
#else
		DWORD
#endif
	>(MAX_READ_BYTES)))) {
		if (bytes > 0)
			out.append(buffer.data(), static_cast<size_t>(bytes));
	}
	return out;
}
#ifdef UNIX
void Pipe::Bind(int& src, int dest) noexcept {
	dup2(src, dest);
	close(src);
	src = -1;
}
void Pipe::Close(int& fd) noexcept {
	if (fd == -1)
		return;
	close(fd);
	fd = -1;
}
#else
void Pipe::Close(HANDLE& fd) noexcept {
	if (fd == INVALID_HANDLE_VALUE)
		return;
	CloseHandle(fd);
	fd = INVALID_HANDLE_VALUE;
}
void Pipe::HandleInformation(HANDLE handle, DWORD mask, DWORD flags) {
	SetHandleInformation(handle, mask, flags);
}
#endif
