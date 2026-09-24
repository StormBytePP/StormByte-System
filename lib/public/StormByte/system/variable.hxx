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

#include <StormByte/system/visibility.h>

#include <filesystem>
#include <string>

/**
 * @brief System module of the StormByte suite.
 */
namespace StormByte::System {
	/**
	 * @class Variable
	 * @brief Environment variable expansion helpers.
	 *
	 * Windows: ExpandEnvironmentStrings. UNIX: expand `~` to the home path.
	 */
	class STORMBYTE_SYSTEM_PUBLIC Variable {
		public:
			/**
			 * @brief Expand environment variables in @p str.
			 * @param str Input string.
			 * @return Expanded string.
			 */
			static std::string Expand(const std::string& str);

			#ifdef WINDOWS
			/**
			 * @brief Expand environment variables in a wide string.
			 * @param str Input wide string.
			 * @return Expanded UTF-8 string.
			 */
			static std::string Expand(const std::wstring& str);
			#endif

		private:
			/**
			 * @brief Platform implementation for UTF-8 / narrow strings.
			 * @param str Input.
			 * @return Expanded string.
			 */
			static std::string ExpandEnvironmentVariable(const std::string& str);

			#ifdef WINDOWS
			/**
			 * @brief Platform implementation for wide strings.
			 * @param str Input.
			 * @return Expanded UTF-8 string.
			 */
			static std::string ExpandEnvironmentVariable(const std::wstring& str);
			#else
			/**
			 * @brief Current user home directory.
			 * @return Home path.
			 */
			static std::filesystem::path HomePath();
			#endif
	};
}
