#pragma once

#include <system/visibility.h>

#include <filesystem>
#include <string>

/**
 * @namespace System
 * @brief All the classes for handling system exceptions
 */
namespace StormByte::System {
	/**
	 * @class Variable
	 * @brief Handling system variables
	 */
	class STORMBYTE_SYSTEM_PUBLIC Variable {
		public:
			/**
			 * Expands environment variables in a string
			 * @param str string
			 * @return expanded string
			 */
			static std::string 				Expand(const std::string& str);
			#ifdef WINDOWS
			/**
			 * Expands environment variables in a string
			 * @param str string
			 * @return expanded string
			 */
			static std::string 				Expand(const std::wstring&);
			#endif

		private:
			/**
			 * Expands environment variables in a string
			 * @param str string
			 * @return expanded string
			 */
			static std::string				ExpandEnvironmentVariable(const std::string& str);
			#ifdef WINDOWS
			/**
			 * Expands environment variables in a string
			 * @param str string
			 * @return expanded string
			 */
			static std::string				ExpandEnvironmentVariable(const std::wstring&);
			#else
			/**
			 * Gets the home path
			 * @return home path
			 */
			static std::filesystem::path	HomePath();
			#endif
	};
}