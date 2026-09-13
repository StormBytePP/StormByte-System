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

#include <StormByte/system/variable.hxx>
#include <StormByte/system/exception.hxx>
#include <StormByte/string.hxx>
#include <vector>
#ifdef WINDOWS
#include <windows.h>
#include <tchar.h>
#else
#include <pwd.h>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>
#endif
using namespace StormByte::System;
std::string Variable::Expand(const std::string& var) {
	return ExpandEnvironmentVariable(var);
} 
#ifdef WINDOWS
std::string Variable::Expand(const std::wstring& var) {
	return ExpandEnvironmentVariable(var);
} 
#endif
std::string Variable::ExpandEnvironmentVariable(const std::string& var) {
	#ifdef WINDOWS
	return ExpandEnvironmentVariable(String::UTF8Decode(var));
	#else
	if (var != "~" && (var.size() < 2 || var[0] != '~' || var[1] != '/'))
		return var;
	const std::filesystem::path home = HomePath();
	if (home.empty())
		return var;
	return home.string() + (var.size() == 1 ? std::string() : var.substr(1));
	#endif
}
#ifdef WINDOWS
std::string Variable::ExpandEnvironmentVariable(const std::wstring& var) {
	DWORD size = ::ExpandEnvironmentStringsW(var.c_str(), nullptr, 0);
	if (size == 0)
		throw ProcessCreationError("ExpandEnvironmentStringsW failed with error " + std::to_string(GetLastError()));
	std::vector<wchar_t> buffer(size);
	while (true) {
		const DWORD result = ::ExpandEnvironmentStringsW(var.c_str(), buffer.data(), static_cast<DWORD>(buffer.size()));
		if (result == 0)
			throw ProcessCreationError("ExpandEnvironmentStringsW failed with error " + std::to_string(GetLastError()));
		if (result <= buffer.size())
			return String::UTF8Encode(std::wstring(buffer.data(), result - 1));
		buffer.resize(result);
	}
}
#else
std::filesystem::path Variable::HomePath() {
	if (const char* home = std::getenv("HOME"); home != nullptr && *home != '\0')
		return home;
	const struct passwd *pw = getpwuid(getuid());
	return pw == nullptr || pw->pw_dir == nullptr ? std::filesystem::path() : std::filesystem::path(pw->pw_dir);
}
#endif
