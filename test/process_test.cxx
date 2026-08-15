//==============================================================================
// FILE: test/process_test.cxx
//==============================================================================

#include <StormByte/system/process.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string Trim(std::string s) {
	auto not_space = [](unsigned char c) { return !std::isspace(c); };
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
	s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
	return s;
}

} // namespace

#ifdef UNIX
int test_basic_execution() {
	std::vector<std::string> args = { "Hello, World!" };
	StormByte::System::Process proc("/bin/echo", args);

	std::string output;
	proc >> output;

	ASSERT_EQUAL("test_basic_execution", "Hello, World!\n", output);

	int exit_code = proc.Wait();
	ASSERT_EQUAL("test_basic_execution", 0, exit_code);

	RETURN_TEST("test_basic_execution", exit_code);
}

int test_pipeline_execution() {
	// "Hello\n" → 6 bytes (portable: printf, not echo -e)
	std::vector<std::string> args1 = { "%s", "Hello\n" };
	std::vector<std::string> args2 = { "-c" };

	StormByte::System::Process proc1("/usr/bin/printf", args1);
	StormByte::System::Process proc2("/usr/bin/wc", args2);

	proc1 >> proc2;

	std::string output;
	proc2 >> output;

	// BSD wc pads with spaces; compare trimmed
	ASSERT_EQUAL("test_pipeline_execution", "6", Trim(output));

	proc1.Wait();
	proc2.Wait();

	RETURN_TEST("test_pipeline_execution", 0);
}

int test_pipeline_sort() {
	std::vector<std::string> args1 = { "%s", "banana\napple\ncherry\n" };

	StormByte::System::Process proc1("/usr/bin/printf", args1);
	StormByte::System::Process proc2("/usr/bin/sort");

	proc1 >> proc2;

	std::string output;
	proc2 >> output;

	ASSERT_EQUAL("test_pipeline_sort", "apple\nbanana\ncherry\n", output);

	proc1.Wait();
	proc2.Wait();

	RETURN_TEST("test_pipeline_sort", 0);
}

int test_pipeline_find_sort_wc() {
	std::vector<std::string> args1 = { "%s", "apple\nbanana\ncherry\napple\nbanana\ncherry\n" };
	std::vector<std::string> args2 = { "apple" };
	std::vector<std::string> args4 = { "-l" };

	StormByte::System::Process proc1("/usr/bin/printf", args1);
	StormByte::System::Process proc2("/usr/bin/grep", args2);  // portable (Linux + macOS)
	StormByte::System::Process proc3("/usr/bin/sort");
	StormByte::System::Process proc4("/usr/bin/wc", args4);

	proc1 >> proc2 >> proc3 >> proc4;

	std::string output;
	proc4 >> output;

	ASSERT_EQUAL("test_pipeline_find_sort_wc", "2", Trim(output));

	proc1.Wait();
	proc2.Wait();
	proc3.Wait();
	proc4.Wait();

	RETURN_TEST("test_pipeline_find_sort_wc", 0);
}

int test_pipeline_echo_sort_wc() {
	std::vector<std::string> args1 = { "%s", "orange\nbanana\napple\ncherry\nbanana\napple\n" };
	std::vector<std::string> args4 = { "-l" };

	StormByte::System::Process proc1("/usr/bin/printf", args1);
	StormByte::System::Process proc2("/usr/bin/sort");
	StormByte::System::Process proc3("/usr/bin/uniq");
	StormByte::System::Process proc4("/usr/bin/wc", args4);

	proc1 >> proc2 >> proc3 >> proc4;

	std::string output;
	proc4 >> output;

	ASSERT_EQUAL("test_pipeline_echo_sort_wc", "4", Trim(output));

	proc1.Wait();
	proc2.Wait();
	proc3.Wait();
	proc4.Wait();

	RETURN_TEST("test_pipeline_echo_sort_wc", 0);
}

int process_to_ostream() {
	std::vector<std::string> args = { "Hello, World!" };
	StormByte::System::Process proc("/bin/echo", args);

	std::ostringstream oss;
	oss << proc;

	std::string output = oss.str();
	ASSERT_EQUAL("process_to_ostream", "Hello, World!\n", output);

	RETURN_TEST("process_to_ostream", 0);
}

#elifdef WINDOWS
int test_basic_execution_windows() {
	std::vector<std::string> args = { "Hello, World!" };
	StormByte::System::Process proc(L"cmd.exe /c echo", args);

	std::string output;
	proc >> output;

	ASSERT_EQUAL("test_basic_execution_windows", "Hello, World! \r\n", output);

	DWORD exit_code = proc.Wait();
	ASSERT_EQUAL("test_basic_execution_windows", 0, exit_code);

	RETURN_TEST("test_basic_execution_windows", exit_code);
}

int test_complex_command_windows() {
	std::vector<std::string> args = { "/c", "dir", "/b", "/a-d" };
	StormByte::System::Process proc(L"cmd.exe", args);

	std::string output;
	proc >> output;

	ASSERT_FALSE("test_complex_command_windows", output.empty());

	DWORD exit_code = proc.Wait();
	ASSERT_EQUAL("test_complex_command_windows", 0, exit_code);

	RETURN_TEST("test_complex_command_windows", exit_code);
}
#endif

int main() {
	int result = 0;
	#ifdef UNIX
		result += test_basic_execution();
		result += test_pipeline_execution();
		result += test_pipeline_sort();
		result += test_pipeline_find_sort_wc();
		result += test_pipeline_echo_sort_wc();
		result += process_to_ostream();
	#elifdef WINDOWS
		/*
		result += test_basic_execution_windows();
		result += test_complex_command_windows();
		*/
	#endif
	if (result == 0) {
		std::cout << "All tests passed!" << std::endl;
	} else {
		std::cout << result << " tests failed." << std::endl;
	}
	return result;
}