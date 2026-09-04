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

#include <StormByte/exception.hxx>
#include <StormByte/system/visibility.h>

#include <filesystem>

/**
 * @brief System module of the StormByte suite.
 */
namespace StormByte::System {
	/**
	 * @class Exception
	 * @brief Base exception for the System module.
	 */
	class STORMBYTE_SYSTEM_PUBLIC Exception: public StormByte::Exception {
		public:
			/**
			 * @brief Construct from a message.
			 * @param message Error message.
			 */
			Exception(const std::string& message);

			/**
			 * @brief Copy constructor.
			 */
			Exception(const Exception&) = default;

			/**
			 * @brief Move constructor.
			 */
			Exception(Exception&&) noexcept = default;

			/**
			 * @brief Copy assignment.
			 */
			Exception& operator=(const Exception&) = default;

			/**
			 * @brief Move assignment.
			 */
			Exception& operator=(Exception&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			virtual ~Exception() noexcept override = default;
	};

	/**
	 * @class FileIOError
	 * @brief A file could not be opened for read or write.
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
			 * @brief Operation as text.
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
			 * @brief Construct from a path and an operation.
			 * @param file File path.
			 * @param operation Failed operation.
			 */
			FileIOError(const std::filesystem::path& file, const Operation& operation);

			/**
			 * @brief Copy constructor.
			 */
			FileIOError(const FileIOError&) = default;

			/**
			 * @brief Move constructor.
			 */
			FileIOError(FileIOError&&) noexcept = default;

			/**
			 * @brief Copy assignment.
			 */
			FileIOError& operator=(const FileIOError&) = default;

			/**
			 * @brief Move assignment.
			 */
			FileIOError& operator=(FileIOError&&) = default;

			/**
			 * @brief Destructor.
			 */
			~FileIOError() noexcept override = default;
	};

	/**
	 * @class ExecutableNotFound
	 * @brief A program could not be executed / was not found.
	 */
	class STORMBYTE_SYSTEM_PUBLIC ExecutableNotFound: public Exception {
		public:
			/**
			 * @brief Construct from an executable path or name.
			 * @param exec Path or name of the missing executable.
			 */
			ExecutableNotFound(const std::filesystem::path& exec);

			/**
			 * @brief Copy constructor.
			 */
			ExecutableNotFound(const ExecutableNotFound&) = default;

			/**
			 * @brief Move constructor.
			 */
			ExecutableNotFound(ExecutableNotFound&&) noexcept = default;

			/**
			 * @brief Copy assignment.
			 */
			ExecutableNotFound& operator=(const ExecutableNotFound&) = default;

			/**
			 * @brief Move assignment.
			 */
			ExecutableNotFound& operator=(ExecutableNotFound&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~ExecutableNotFound() noexcept override = default;
	};
}
