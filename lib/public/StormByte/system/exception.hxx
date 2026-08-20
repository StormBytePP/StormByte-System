/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/exception.hxx>
#include <StormByte/system/visibility.h>

#include <filesystem>

/**
 * @namespace System
 * @brief System utilities: processes, pipes, environment variables.
 */
namespace StormByte::System {
	/**
	 * @class Exception
	 * @brief Base exception for the System module.
	 */
	class STORMBYTE_SYSTEM_PUBLIC Exception: public StormByte::Exception {
		public:
			/**
			 * @param message Error message.
			 */
			Exception(const std::string& message);

			/**
			 * Copy constructor.
			 */
			Exception(const Exception&) = default;

			/**
			 * Move constructor.
			 */
			Exception(Exception&&) noexcept = default;

			/**
			 * Copy assignment.
			 */
			Exception& operator=(const Exception&) = default;

			/**
			 * Move assignment.
			 */
			Exception& operator=(Exception&&) noexcept = default;

			/**
			 * Destructor.
			 */
			virtual ~Exception() noexcept override = default;
	};

	/**
	 * @class FileIOError
	 * @brief Thrown when a file cannot be opened for read or write.
	 */
	class STORMBYTE_SYSTEM_PUBLIC FileIOError final: public Exception {
		public:
			/**
			 * @enum Operation
			 * @brief Failed file operation.
			 */
			enum class Operation {
				Read = 0,	///< Open for reading
				Write		///< Open for writing
			};

			/**
			 * @param op Operation.
			 * @return "read", "write", or "unknown".
			 */
			constexpr static const char* operation_to_string(const Operation& op) noexcept {
				switch (op) {
					case Operation::Read:	return "read";
					case Operation::Write:	return "write";
					default:				return "unknown";
				}
			}

			/**
			 * @param file File path.
			 * @param operation Failed operation.
			 */
			FileIOError(const std::filesystem::path& file, const Operation& operation);

			/**
			 * Copy constructor.
			 */
			FileIOError(const FileIOError&) = default;

			/**
			 * Move constructor.
			 */
			FileIOError(FileIOError&&) noexcept = default;

			/**
			 * Copy assignment.
			 */
			FileIOError& operator=(const FileIOError&) = default;

			/**
			 * Move assignment.
			 */
			FileIOError& operator=(FileIOError&&) = default;

			/**
			 * Destructor.
			 */
			~FileIOError() noexcept override = default;
	};

	/**
	 * @class ExecutableNotFound
	 * @brief Thrown when a program cannot be executed / was not found.
	 */
	class STORMBYTE_SYSTEM_PUBLIC ExecutableNotFound: public Exception {
		public:
			/**
			 * @param exec Path or name of the missing executable.
			 */
			ExecutableNotFound(const std::filesystem::path& exec);

			/**
			 * Copy constructor.
			 */
			ExecutableNotFound(const ExecutableNotFound&) = default;

			/**
			 * Move constructor.
			 */
			ExecutableNotFound(ExecutableNotFound&&) noexcept = default;

			/**
			 * Copy assignment.
			 */
			ExecutableNotFound& operator=(const ExecutableNotFound&) = default;

			/**
			 * Move assignment.
			 */
			ExecutableNotFound& operator=(ExecutableNotFound&&) noexcept = default;

			/**
			 * Destructor.
			 */
			~ExecutableNotFound() noexcept override = default;
	};
}
