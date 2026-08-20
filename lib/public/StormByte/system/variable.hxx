#pragma once

#include <StormByte/system/visibility.h>

#include <filesystem>
#include <string>

/**
 * @namespace System
 * @brief System utilities: processes, pipes, environment variables.
 */
namespace StormByte::System {
	/**
	 * @class Variable
	 * @brief Environment variable expansion helpers.
	 *
	 * On Windows uses ExpandEnvironmentStrings; on UNIX expands `~` to the home path.
	 */
	class STORMBYTE_SYSTEM_PUBLIC Variable {
		public:
			/**
			 * Expands environment variables in @p str.
			 * @param str Input string.
			 * @return Expanded string.
			 */
			static std::string Expand(const std::string& str);

			#ifdef WINDOWS
			/**
			 * Expands environment variables in a wide string.
			 * @param str Input wide string.
			 * @return Expanded UTF-8 string.
			 */
			static std::string Expand(const std::wstring& str);
			#endif

		private:
			/**
			 * Platform implementation for UTF-8 / narrow strings.
			 * @param str Input.
			 * @return Expanded string.
			 */
			static std::string ExpandEnvironmentVariable(const std::string& str);

			#ifdef WINDOWS
			/**
			 * Platform implementation for wide strings.
			 * @param str Input.
			 * @return Expanded UTF-8 string.
			 */
			static std::string ExpandEnvironmentVariable(const std::wstring& str);
			#else
			/**
			 * @return Current user home directory.
			 */
			static std::filesystem::path HomePath();
			#endif
	};
}
