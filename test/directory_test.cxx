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

#include <StormByte/system/directory.hxx>
#include <StormByte/test_handlers.h>

#include <iostream>
#include <string>

using StormByte::String::String;

namespace {
	void Dump(const char* label, const bool ok, const String& path) {
		std::cout << label
			<< " ok=" << (ok ? "true" : "false")
			<< " value=" << std::string(path)
			<< " fault=" << StormByte::System::Directory::LastError().what()
			<< '\n';
	}
}

// -------------------
// Current
// -------------------
int test_directory_current() {
	const std::string fn = "test_directory_current";
	String path;
	Dump("current", StormByte::System::Directory::Current(path), path);
	RETURN_TEST(fn, 0);
}

// -------------------
// CurrentExecutable
// -------------------
int test_directory_current_executable() {
	const std::string fn = "test_directory_current_executable";
	String path;
	Dump("current_executable", StormByte::System::Directory::CurrentExecutable(path), path);
	RETURN_TEST(fn, 0);
}

// -------------------
// Home
// -------------------
int test_directory_home() {
	const std::string fn = "test_directory_home";
	String path;
	Dump("home", StormByte::System::Directory::Home(path), path);
	RETURN_TEST(fn, 0);
}

// -------------------
// Temporary
// -------------------
int test_directory_temporary() {
	const std::string fn = "test_directory_temporary";
	String path;
	Dump("temporary", StormByte::System::Directory::Temporary(path), path);
	RETURN_TEST(fn, 0);
}

int main() {
	int result = 0;

	// -------------------
	// Current
	// -------------------
	result += test_directory_current();

	// -------------------
	// CurrentExecutable
	// -------------------
	result += test_directory_current_executable();

	// -------------------
	// Home
	// -------------------
	result += test_directory_home();

	// -------------------
	// Temporary
	// -------------------
	result += test_directory_temporary();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
