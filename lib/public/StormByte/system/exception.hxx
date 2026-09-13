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
#include <format>
#include <utility>

/**
 * @brief System module of the StormByte suite.
 */
namespace StormByte::System {
	/**
	 * @class Exception
	 * @brief Base exception for the System module.
	 *
	 * Tags every message with the `System` component.
	 */
	class STORMBYTE_SYSTEM_PUBLIC Exception: public StormByte::Exception {
		public:
			/**
			 * @brief Plain-message constructor tagged with the `System` component.
			 * @param message Exception message.
			 */
			inline Exception(const std::string& message):
			StormByte::Exception(StormByte::Component("System"), "{}", message) {}

			/**
			 * @brief Format-string constructor tagged with the `System` component.
			 * @tparam Args Format argument types.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			inline Exception(std::format_string<Args...> fmt, Args&&... args):
			StormByte::Exception(StormByte::Component("System"), fmt, std::forward<Args>(args)...) {}

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
			inline FileIOError(const std::filesystem::path& file, const Operation& operation):
			Exception("File {} can not be opened for {}", file.string(), operation_to_string(operation)) {}

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
			inline ExecutableNotFound(const std::filesystem::path& exec):
			Exception("Executable {} not found", exec.string()) {}

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

	/**
	 * @class ProcessCreationError
	 * @brief A process could not be created.
	 */
	class STORMBYTE_SYSTEM_PUBLIC ProcessCreationError final: public Exception {
		public:
			/**
			 * @brief Construct from a creation failure reason.
			 * @param reason Failure reason.
			 */
			inline ProcessCreationError(const std::string& reason):
			Exception("Process creation failed: {}", reason) {}

			/**
			 * @brief Copy constructor.
			 */
			ProcessCreationError(const ProcessCreationError&) = default;

			/**
			 * @brief Move constructor.
			 */
			ProcessCreationError(ProcessCreationError&&) noexcept = default;

			/**
			 * @brief Copy assignment.
			 */
			ProcessCreationError& operator=(const ProcessCreationError&) = default;

			/**
			 * @brief Move assignment.
			 */
			ProcessCreationError& operator=(ProcessCreationError&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~ProcessCreationError() noexcept override = default;
	};
}
