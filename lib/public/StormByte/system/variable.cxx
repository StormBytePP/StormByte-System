#include <StormByte/system/variable.hxx>
#include <StormByte/string.hxx>

#ifdef LINUX
#include <pwd.h>
#include <regex>
#include <sys/types.h>
#include <unistd.h>
#else
#include <windows.h>
#include <tchar.h>
#define INFO_BUFFER_SIZE 32767
#endif

using namespace StormByte::System;

std::string Variable::Expand(const std::string& var) {
	return ExpandEnvironmentVariable(var);
} 

#ifdef WINDOWS
std::string Variable::Expand(const std::wstring& var) {
	return ExpandEnvironmentVariable(var);
} 
#endif

std::string Variable::ExpandEnvironmentVariable(const std::string& var) {
	#ifdef WINDOWS
	return ExpandEnvironmentVariable(String::UTF8Decode(var));
	#else
	return std::regex_replace(var, std::regex("~"), HomePath().string());
	#endif
}
#ifdef WINDOWS
std::string Variable::ExpandEnvironmentVariable(const std::wstring& var) {
	wchar_t infoBuf[INFO_BUFFER_SIZE] = { L'\0' };
	::ExpandEnvironmentStringsW(var.c_str(), infoBuf, INFO_BUFFER_SIZE);

	return String::UTF8Encode(std::wstring(infoBuf));
}
#else
std::filesystem::path Variable::HomePath() {
	const struct passwd *pw = getpwuid(getuid());
	return pw->pw_dir;
}
#endif