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

#include <StormByte/system/host.hxx>
#include <StormByte/test_handlers.h>

#include <iostream>
#include <string>

using StormByte::String::String;

namespace {
	void DumpText(const char* label, const String& value) {
		std::cout << label
			<< " value=" << std::string(value)
			<< " fault=" << StormByte::System::Host::LastError().what()
			<< '\n';
	}
}

// -------------------
// Architecture
// -------------------
int test_host_architecture() {
	const std::string fn = "test_host_architecture";
	DumpText("architecture", StormByte::System::Host::Architecture());
	RETURN_TEST(fn, 0);
}

// -------------------
// Bitness
// -------------------
int test_host_bitness() {
	const std::string fn = "test_host_bitness";
	std::cout << "bitness=" << StormByte::System::Host::Bitness() << '\n';
	RETURN_TEST(fn, 0);
}

// -------------------
// CPU
// -------------------
int test_host_cpu() {
	const std::string fn = "test_host_cpu";
	DumpText("cpu", StormByte::System::Host::CPU());
	RETURN_TEST(fn, 0);
}

// -------------------
// Kernel
// -------------------
int test_host_kernel() {
	const std::string fn = "test_host_kernel";
	DumpText("kernel", StormByte::System::Host::Kernel());
	RETURN_TEST(fn, 0);
}

// -------------------
// Memory
// -------------------
int test_host_memory() {
	const std::string fn = "test_host_memory";
	const auto page = StormByte::System::Host::PageSize();
	std::cout << "page_size=" << static_cast<std::uint64_t>(page)
		<< " fault=" << StormByte::System::Host::LastError().what() << '\n';
	const auto physical = StormByte::System::Host::PhysicalMemory();
	std::cout << "physical_memory=" << static_cast<std::uint64_t>(physical)
		<< " fault=" << StormByte::System::Host::LastError().what() << '\n';
	const auto available = StormByte::System::Host::AvailableMemory();
	std::cout << "available_memory=" << static_cast<std::uint64_t>(available)
		<< " fault=" << StormByte::System::Host::LastError().what() << '\n';
	RETURN_TEST(fn, 0);
}

// -------------------
// Name
// -------------------
int test_host_name() {
	const std::string fn = "test_host_name";
	String name;
	const bool ok = StormByte::System::Host::Name(name);
	std::cout << "name ok=" << (ok ? "true" : "false")
		<< " value=" << std::string(name)
		<< " fault=" << StormByte::System::Host::LastError().what() << '\n';
	RETURN_TEST(fn, 0);
}

// -------------------
// OS
// -------------------
int test_host_os() {
	const std::string fn = "test_host_os";
	DumpText("os", StormByte::System::Host::OS());
	RETURN_TEST(fn, 0);
}

// -------------------
// Processors
// -------------------
int test_host_processors() {
	const std::string fn = "test_host_processors";
	std::cout << "logical_processors=" << StormByte::System::Host::LogicalProcessors() << '\n';
	RETURN_TEST(fn, 0);
}

int main() {
	int result = 0;

	// -------------------
	// Architecture
	// -------------------
	result += test_host_architecture();

	// -------------------
	// Bitness
	// -------------------
	result += test_host_bitness();

	// -------------------
	// CPU
	// -------------------
	result += test_host_cpu();

	// -------------------
	// Kernel
	// -------------------
	result += test_host_kernel();

	// -------------------
	// Memory
	// -------------------
	result += test_host_memory();

	// -------------------
	// Name
	// -------------------
	result += test_host_name();

	// -------------------
	// OS
	// -------------------
	result += test_host_os();

	// -------------------
	// Processors
	// -------------------
	result += test_host_processors();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
