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
#include <StormByte/string.hxx>
#ifdef WINDOWS
#include <windows.h>
#include <tchar.h>
#define INFO_BUFFER_SIZE 32767
#else
#include <pwd.h>
#include <regex>
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
	return std::regex_replace(var, std::regex("~"), HomePath().string());
	#endif
}
#ifdef WINDOWS
std::string Variable::ExpandEnvironmentVariable(const std::wstring& var) {
	wchar_t infoBuf[INFO_BUFFER_SIZE] = { L'\0' };
	::ExpandEnvironmentStringsW(var.c_str(), infoBuf, INFO_BUFFER_SIZE);
	return String::UTF8Encode(std::wstring(infoBuf));
}
#else
std::filesystem::path Variable::HomePath() {
	const struct passwd *pw = getpwuid(getuid());
	return pw->pw_dir;
}
#endif
