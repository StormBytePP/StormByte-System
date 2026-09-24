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

#pragma once

#include <StormByte/error.hxx>
#include <StormByte/system/visibility.h>

#include <string>
#include <system_error>

/**
 * @brief System module of the StormByte suite.
 */
namespace StormByte::System {
	/**
	 * @enum Error
	 * @brief Module-wide `std::error_code` enumerators that are not Device or Process.
	 *
	 * Device uses @ref Device::Error. Process uses @ref Process::Error.
	 * Zero is success (`std::error_code` converts to false).
	 */
	enum class Error {
		Success = 0,	///< No error
		Unknown,		///< Unclassified System error
		Permission,		///< The caller may not perform this System operation
		NotFound,		///< A System resource other than a Device or child process was missing
		Invalid			///< The request is not applicable to this System object
	};
}

/**
 * @brief Domain for @ref StormByte::System::Error.
 */
template<>
struct StormByte::Error::Domain<StormByte::System::Error> {
	static constexpr const char* Name = "StormByte.System";	///< Stable category tag

	/**
	 * @brief Text for one System enumerator.
	 * @param e Enumerator.
	 * @return Human-readable message.
	 */
	static std::string Message(StormByte::System::Error e) {
		switch (e) {
			case StormByte::System::Error::Success:
				return "Success";
			case StormByte::System::Error::Unknown:
				return "Unknown System error";
			case StormByte::System::Error::Permission:
				return "Permission denied";
			case StormByte::System::Error::NotFound:
				return "Resource not found";
			case StormByte::System::Error::Invalid:
				return "Invalid request";
		}
		return "Unknown System error";
	}
};

namespace StormByte::System {
	/**
	 * @brief Category singleton for @ref Error.
	 * @return Process-wide category.
	 */
	STORMBYTE_SYSTEM_PUBLIC const StormByte::Error::Category<Error>& category() noexcept;

	/**
	 * @brief Builds an `std::error_code` from @ref Error.
	 * @param e Enumerator.
	 * @return Code in @ref category().
	 */
	STORMBYTE_SYSTEM_PUBLIC std::error_code make_error_code(Error e) noexcept;
}

namespace std {
	/**
	 * @brief Marks @ref StormByte::System::Error as an `std::error_code` enum.
	 */
	template<>
	struct is_error_code_enum<StormByte::System::Error>: true_type {};
}
