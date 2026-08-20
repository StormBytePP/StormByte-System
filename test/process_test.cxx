#include <StormByte/system/process.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
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

	RETURN_TEST("test_basic_execution", 0);
}

int test_pipeline_execution() {
	std::vector<std::string> args1 = { "%s", "Hello\n" };
	std::vector<std::string> args2 = { "-c" };

	StormByte::System::Process proc1("/usr/bin/printf", args1);
	StormByte::System::Process proc2("/usr/bin/wc", args2);

	proc1 >> proc2;

	std::string output;
	proc2 >> output;

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
	StormByte::System::Process proc2("/usr/bin/grep", args2);
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

	ASSERT_EQUAL("process_to_ostream", "Hello, World!\n", oss.str());

	proc.Wait();
	RETURN_TEST("process_to_ostream", 0);
}

int test_stdin_roundtrip() {
	// cat copies stdin → stdout
	StormByte::System::Process proc("/bin/cat");

	proc << "line-one\n";
	proc << "line-two\n";
	proc << StormByte::System::EoF;

	std::string output;
	proc >> output;

	ASSERT_EQUAL("test_stdin_roundtrip", "line-one\nline-two\n", output);

	int exit_code = proc.Wait();
	ASSERT_EQUAL("test_stdin_roundtrip", 0, exit_code);

	RETURN_TEST("test_stdin_roundtrip", 0);
}

int test_stderr_capture() {
	// printf to stderr: format on argv, data on argv — use sh -c only if /bin/sh is acceptable.
	// Portable without shell: write to stdout via printf and rely on stderr from a known tool.
	// /usr/bin/printf does not write to stderr easily without shell.
	// Use: printf goes to stdout; for stderr use a second approach with /bin/sh -c which is on all UNIX.
	std::vector<std::string> args = { "-c", "printf '%s' 'err-msg' 1>&2" };
	StormByte::System::Process proc("/bin/sh", args);

	std::string err;
	proc.Stderr(err);

	ASSERT_EQUAL("test_stderr_capture", "err-msg", err);

	int exit_code = proc.Wait();
	ASSERT_EQUAL("test_stderr_capture", 0, exit_code);

	RETURN_TEST("test_stderr_capture", 0);
}

int test_exit_code_false() {
	StormByte::System::Process proc("/usr/bin/false");
	int exit_code = proc.Wait();
	ASSERT_TRUE("test_exit_code_false", exit_code != 0);
	RETURN_TEST("test_exit_code_false", 0);
}

int test_exit_code_true() {
	StormByte::System::Process proc("/usr/bin/true");
	int exit_code = proc.Wait();
	ASSERT_EQUAL("test_exit_code_true", 0, exit_code);
	RETURN_TEST("test_exit_code_true", 0);
}

int test_move_process() {
	std::vector<std::string> args = { "moved" };
	StormByte::System::Process original("/bin/echo", args);
	StormByte::System::Process moved(std::move(original));

	// Original should no longer own the child (Wait is safe no-op / -1)
	(void)original.Wait();

	std::string output;
	moved >> output;
	ASSERT_EQUAL("test_move_process", "moved\n", output);

	int exit_code = moved.Wait();
	ASSERT_EQUAL("test_move_process", 0, exit_code);

	RETURN_TEST("test_move_process", 0);
}

int test_tr_pipeline() {
	std::vector<std::string> args1 = { "%s", "abc" };
	std::vector<std::string> args2 = { "a-z", "A-Z" };

	StormByte::System::Process proc1("/usr/bin/printf", args1);
	StormByte::System::Process proc2("/usr/bin/tr", args2);

	proc1 >> proc2;

	std::string output;
	proc2 >> output;

	ASSERT_EQUAL("test_tr_pipeline", "ABC", output);

	proc1.Wait();
	proc2.Wait();

	RETURN_TEST("test_tr_pipeline", 0);
}

#elifdef WINDOWS

int test_basic_execution_windows() {
	std::vector<std::string> args = { "/c", "echo", "Hello, World!" };
	StormByte::System::Process proc("cmd.exe", args);

	std::string output;
	proc >> output;

	// cmd echo typically ends with \r\n; trim for robustness
	ASSERT_EQUAL("test_basic_execution_windows", "Hello, World!", Trim(output));

	DWORD exit_code = proc.Wait();
	ASSERT_EQUAL("test_basic_execution_windows", 0u, exit_code);

	RETURN_TEST("test_basic_execution_windows", 0);
}

int test_stdin_roundtrip_windows() {
	// sort.exe is in System32 on all supported Windows images
	StormByte::System::Process proc("sort.exe");

	proc << "b\r\n";
	proc << "a\r\n";
	proc << StormByte::System::EoF;

	std::string output;
	proc >> output;

	// Normalize CRLF → LF for comparison
	std::string normalized;
	normalized.reserve(output.size());
	for (size_t i = 0; i < output.size(); ++i) {
		if (output[i] == '\r')
			continue;
		normalized.push_back(output[i]);
	}

	ASSERT_EQUAL("test_stdin_roundtrip_windows", "a\nb\n", normalized);

	DWORD exit_code = proc.Wait();
	ASSERT_EQUAL("test_stdin_roundtrip_windows", 0u, exit_code);

	RETURN_TEST("test_stdin_roundtrip_windows", 0);
}

int test_exit_code_windows() {
	std::vector<std::string> args = { "/c", "exit", "/b", "7" };
	StormByte::System::Process proc("cmd.exe", args);

	DWORD exit_code = proc.Wait();
	ASSERT_EQUAL("test_exit_code_windows", 7u, exit_code);

	RETURN_TEST("test_exit_code_windows", 0);
}

int test_move_process_windows() {
	std::vector<std::string> args = { "/c", "echo", "moved" };
	StormByte::System::Process original("cmd.exe", args);
	StormByte::System::Process moved(std::move(original));

	(void)original.Wait();

	std::string output;
	moved >> output;
	ASSERT_EQUAL("test_move_process_windows", "moved", Trim(output));

	DWORD exit_code = moved.Wait();
	ASSERT_EQUAL("test_move_process_windows", 0u, exit_code);

	RETURN_TEST("test_move_process_windows", 0);
}

int test_dir_lists_something() {
	std::vector<std::string> args = { "/c", "dir", "/b" };
	StormByte::System::Process proc("cmd.exe", args);

	std::string output;
	proc >> output;

	ASSERT_FALSE("test_dir_lists_something", Trim(output).empty());

	DWORD exit_code = proc.Wait();
	ASSERT_EQUAL("test_dir_lists_something", 0u, exit_code);

	RETURN_TEST("test_dir_lists_something", 0);
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
	result += test_stdin_roundtrip();
	result += test_stderr_capture();
	result += test_exit_code_false();
	result += test_exit_code_true();
	result += test_move_process();
	result += test_tr_pipeline();
#elif defined(WINDOWS)
	result += test_basic_execution_windows();
	result += test_stdin_roundtrip_windows();
	result += test_exit_code_windows();
	result += test_move_process_windows();
	result += test_dir_lists_something();
#endif

	if (result == 0) {
		std::cout << "All tests passed!" << std::endl;
	} else {
		std::cout << result << " tests failed." << std::endl;
	}
	return result;
}
