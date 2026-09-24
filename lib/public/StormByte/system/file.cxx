/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-System.
 *
 * StormByte-System original source is dual-licensed:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You may redistribute and/or modify this file under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this file may be used under the terms of a commercial
 *    license agreement with the copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *
 * Both licenses apply only to original StormByte-System source in this
 * repository. They do not cover other StormByte modules or any third-party
 * material shipped with this repository (including everything under
 * thirdparty/, and in particular the bundled StormByte Base tree), which
 * remains under its own license.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte-System is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte-System. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#include <StormByte/system/file.hxx>

#include <StormByte/error.txx>
#include <StormByte/string/wstring.hxx>
#include <StormByte/system/directory.hxx>

#include <filesystem>
#include <string>
#include <vector>

#ifdef WINDOWS
#include <windows.h>
#elifdef MACOS
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <mach-o/dyld.h>
#else
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#endif

using namespace StormByte::System;

namespace {
	thread_local StormByte::Error::Fault g_last{make_error_code(File::Error::Success)};

	bool Store(const enum File::Error code) {
		g_last = StormByte::Error::Fault{make_error_code(code)};
		return code == File::Error::Success;
	}

	StormByte::String::String FromNative(const std::filesystem::path& path) {
#ifdef WINDOWS
		return StormByte::String::String(StormByte::String::WString(path.wstring()));
#else
		return StormByte::String::String(path.string());
#endif
	}

	std::filesystem::path NativePath(const StormByte::String::String& text) {
#ifdef WINDOWS
		return std::filesystem::path(static_cast<std::wstring_view>(StormByte::String::WString(text)));
#else
		return std::filesystem::path(static_cast<std::string_view>(text));
#endif
	}

	std::filesystem::path ExecutableFile() {
#ifdef WINDOWS
		std::vector<wchar_t> buf(MAX_PATH);
		for (;;) {
			const DWORD n = GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
			if (n == 0)
				return {};
			if (n < buf.size())
				return std::filesystem::path(std::wstring(buf.data(), n));
			buf.resize(buf.size() * 2);
		}
#elifdef MACOS
		uint32_t size = 0;
		_NSGetExecutablePath(nullptr, &size);
		if (size == 0)
			return {};
		std::vector<char> buf(size);
		if (_NSGetExecutablePath(buf.data(), &size) != 0)
			return {};
		return std::filesystem::path(buf.data());
#else
		std::vector<char> buf(256);
		for (;;) {
			const ssize_t count = readlink("/proc/self/exe", buf.data(), buf.size());
			if (count < 0)
				return {};
			if (static_cast<size_t>(count) < buf.size())
				return std::filesystem::path(std::string(buf.data(), static_cast<size_t>(count)));
			buf.resize(buf.size() * 2);
		}
#endif
	}
}

StormByte::Error::Fault File::LastError() noexcept {
	return g_last;
}

bool File::Temporary(StormByte::String::String& path, std::string_view prefix, std::string_view suffix) {
	StormByte::String::String directory;
	if (!Directory::Temporary(directory))
		return Store(File::Error::Failed);

#ifdef WINDOWS
	std::wstring prefix_w(static_cast<std::wstring_view>(StormByte::String::WString(StormByte::String::String(prefix))));
	if (prefix_w.size() > 3)
		prefix_w.resize(3);
	if (prefix_w.empty())
		prefix_w = L"TMP";

	wchar_t generated[MAX_PATH];
	const StormByte::String::WString dir_w(directory);
	if (GetTempFileNameW(static_cast<const wchar_t*>(dir_w), prefix_w.c_str(), 0, generated) == 0)
		return Store(File::Error::Failed);

	std::filesystem::path result(generated);
	if (!suffix.empty()) {
		const std::filesystem::path dest = std::filesystem::path(std::wstring(generated) +
			std::wstring(static_cast<std::wstring_view>(StormByte::String::WString(StormByte::String::String(suffix)))));
		if (!MoveFileW(result.c_str(), dest.c_str())) {
			DeleteFileW(result.c_str());
			return Store(File::Error::Failed);
		}
		result = dest;
	}
	path = FromNative(result);
	return Store(File::Error::Success);
#else
	std::string tmpl = NativePath(directory).string();
	if (tmpl.empty() || tmpl.back() != '/')
		tmpl.push_back('/');
	tmpl.append(prefix.empty() ? "TMP" : std::string(prefix));
	tmpl.append("XXXXXX");
	tmpl.append(suffix);

	const int fd = suffix.empty()
		? mkstemp(tmpl.data())
		: mkstemps(tmpl.data(), static_cast<int>(suffix.size()));
	if (fd == -1)
		return Store((errno == EACCES || errno == EPERM) ? File::Error::Permission : File::Error::Failed);
	::close(fd);
	path = FromNative(std::filesystem::path(tmpl));
	return Store(File::Error::Success);
#endif
}

bool File::CurrentExecutable(StormByte::String::String& path) {
	const std::filesystem::path file = ExecutableFile();
	if (file.empty())
		return Store(File::Error::Failed);
	path = FromNative(file);
	return Store(File::Error::Success);
}

namespace StormByte::System {
	const StormByte::Error::Category<enum File::Error>& file_category() noexcept {
		static StormByte::Error::Category<enum File::Error> instance;
		return instance;
	}

	std::error_code make_error_code(const enum File::Error e) noexcept {
		return std::error_code(static_cast<int>(e), file_category());
	}
}
