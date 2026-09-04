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
