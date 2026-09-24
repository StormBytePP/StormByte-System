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

#include <StormByte/system/device.hxx>
#include <StormByte/test_handlers.h>

#include <filesystem>
#include <iostream>
#include <string>

using StormByte::System::Device;

namespace {
	const char* KindName(const enum Device::Kind kind) {
		switch (kind) {
			case Device::Kind::HDD:			return "HDD";
			case Device::Kind::SSD:			return "SSD";
			case Device::Kind::NVMeGen3:	return "NVMeGen3";
			case Device::Kind::NVMeGen4:	return "NVMeGen4";
			case Device::Kind::NVMeGen5:	return "NVMeGen5";
			case Device::Kind::USBHDD:		return "USBHDD";
			case Device::Kind::USBStick:	return "USBStick";
			case Device::Kind::Network:		return "Network";
		}
		return "Unknown";
	}

	void Dump(const char* label, const Device& device) {
		std::cout << label << " path=" << std::string(device.Path())
			<< " ok=" << (static_cast<bool>(device) ? "true" : "false")
			<< " fault=" << device.Fault().what() << '\n';
		if (!device)
			return;
		const auto access = device.Access();
		const auto rate = device.Throughput();
		const auto window = device.Window();
		std::cout << label << " kind=" << KindName(device.Kind())
			<< " readable=" << (access.Has(Device::AccessFlag::Readable) ? "true" : "false")
			<< " writable=" << (access.Has(Device::AccessFlag::Writable) ? "true" : "false")
			<< " read_bps=" << rate.read_bps
			<< " write_bps=" << rate.write_bps
			<< " window_read=" << static_cast<std::size_t>(window.read)
			<< " window_write=" << static_cast<std::size_t>(window.write)
			<< '\n';
	}
}

// -------------------
// Root
// -------------------
int test_device_root() {
	const std::string fn = "test_device_root";
#ifdef WINDOWS
	Device device(std::filesystem::path("C:\\"));
#else
	Device device(std::filesystem::path("/"));
#endif
	Dump("root", device);
	Device copied(device);
	Dump("root.copy", copied);
	RETURN_TEST(fn, 0);
}

// -------------------
// Temp
// -------------------
int test_device_temp() {
	const std::string fn = "test_device_temp";
	Device device(std::filesystem::temp_directory_path());
	Dump("temp", device);
	RETURN_TEST(fn, 0);
}

int main() {
	int result = 0;

	// -------------------
	// Root
	// -------------------
	result += test_device_root();

	// -------------------
	// Temp
	// -------------------
	result += test_device_temp();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
