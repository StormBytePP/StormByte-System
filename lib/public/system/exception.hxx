#pragma once

#include <exception.hxx>
#include <system/visibility.h>

#include <filesystem>

/**
 * @namespace System
 * @brief All the classes for handling system exceptions
 */
namespace StormByte::System {
	/**
	 * @class Exception
	 * @brief Exception base class for System
	 * 
	 * The purpose of this class is to handle a generic exception for all the StormByte library
	 * and also to cover the Windows/Linux differences in exception handling, specially in passing
	 * std::string across library boundaries handling the const char* memory management.
	 */
	class STORMBYTE_SYSTEM_PUBLIC Exception: public StormByte::Exception {
		public:
			/**
			 * Constructor
			 * @param message message
			 */
			Exception(const std::string& message);

			/**
			 * Copy constructor
			 */
			Exception(const Exception&)					= default;

			/**
			 * Move constructor
			 */
			Exception(Exception&&) noexcept				= default;

			/**
			 * Assignment operator
			 */
			Exception& operator=(const Exception&)		= default;

			/**
			 * Move operator
			 */
			Exception& operator=(Exception&&) noexcept	= default;

			/**
			 * Destructor
			 */
			virtual ~Exception() noexcept override		= default;
	};

	/**
	 * @class FileIOError
	 * @brief Exception thrown when a file can not be opened or written
	 */
	class STORMBYTE_SYSTEM_PUBLIC FileIOError final: public Exception {
		public:
			/**
			 * @enum Operation
			 * @brief File operation
			 */
			enum class Operation { Read = 0, Write }; 

			/**
			 * Converts Operation to string
			 * @param op operation
			 * @return string
			 */
			constexpr static const char* operation_to_string(const Operation& op) noexcept {
				switch(op) {
					case Operation::Read: 	return "read";
					case Operation::Write:	return "write";
					default:				return "unknown";
				}
			}

			/**
			 * Constructor
			 * @param file file path
			 * @param operation operation
			 */
			FileIOError(const std::filesystem::path& file, const Operation& operation);

			/**
			 * Copy constructor
			 */
			FileIOError(const FileIOError&)				= default;

			/**
			 * Move constructor
			 */
			FileIOError(FileIOError&&) noexcept			= default;

			/**
			 * Assignment operator
			 */
			FileIOError& operator=(const FileIOError&)	= default;

			/**
			 * Move assignment operator
			 */
			FileIOError& operator=(FileIOError&&)		= default;

			/**
			 * Destructor
			 */
			~FileIOError() noexcept override			= default;
	};
}