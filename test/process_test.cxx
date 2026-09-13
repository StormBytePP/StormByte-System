/*
	StormByte::System::Process proc("true");
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

#include <StormByte/system/exception.hxx>
#include <StormByte/system/process.hxx>
#include <StormByte/system/variable.hxx>
#include <StormByte/test_handlers.h>
#include <algorithm>
#ifdef UNIX
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#endif
#include <chrono>
#include <cctype>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
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
	StormByte::System::Process proc("echo", args);
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
	StormByte::System::Process proc1("printf", args1);
	StormByte::System::Process proc2("wc", args2);
	proc1 >> proc2;
	std::string output;
	proc2 >> output;
	ASSERT_EQUAL("test_pipeline_execution", "6", Trim(output));
	proc1.Wait();
	proc2.Wait();
	RETURN_TEST("test_pipeline_execution", 0);
}
int test_pipeline_accumulated_and_future_output() {
	StormByte::System::Process producer("cat");
	StormByte::System::Process consumer("cat");
	producer << "before\n";
	producer >> consumer;
	producer << "after\n";
	producer << StormByte::System::EoF;
	std::string output;
	consumer >> output;
	ASSERT_EQUAL("test_pipeline_accumulated_and_future_output", "before\nafter\n", output);
	producer.Wait();
	consumer.Wait();
	RETURN_TEST("test_pipeline_accumulated_and_future_output", 0);
}
int test_pipeline_destination_exits_first() {
	StormByte::System::Process producer("yes");
	{
		std::vector<std::string> args = { "-c", "1" };
		StormByte::System::Process consumer("head", args);
		producer >> consumer;
		std::string output;
		consumer >> output;
		ASSERT_EQUAL("test_pipeline_destination_exits_first", 1u, output.size());
	}
	producer.Wait();
	RETURN_TEST("test_pipeline_destination_exits_first", 0);
}
int test_pipeline_reconnect() {
	StormByte::System::Process producer("cat");
	StormByte::System::Process first_consumer("cat");
	StormByte::System::Process second_consumer("cat");
	producer >> first_consumer;
	producer << "before\n";
	producer >> second_consumer;
	producer << "after\n";
	producer << StormByte::System::EoF;
	std::string output;
	second_consumer >> output;
	ASSERT_TRUE("test_pipeline_reconnect", output.ends_with("after\n"));
	producer.Wait();
	first_consumer.Wait();
	second_consumer.Wait();
	RETURN_TEST("test_pipeline_reconnect", 0);
}
int test_pipeline_reconnect_long_lived() {
	StormByte::System::Process producer("sleep", { "1" });
	StormByte::System::Process first_consumer("cat");
	StormByte::System::Process second_consumer("cat");
	producer >> first_consumer;
	producer >> second_consumer;
	ASSERT_EQUAL("test_pipeline_reconnect_long_lived", 0, producer.Wait());
	ASSERT_EQUAL("test_pipeline_reconnect_long_lived", 0, first_consumer.Wait());
	ASSERT_EQUAL("test_pipeline_reconnect_long_lived", 0, second_consumer.Wait());
	RETURN_TEST("test_pipeline_reconnect_long_lived", 0);
}
int test_pipeline_sort() {
	std::vector<std::string> args1 = { "%s", "banana\napple\ncherry\n" };
	StormByte::System::Process proc1("printf", args1);
	StormByte::System::Process proc2("sort");
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
	StormByte::System::Process proc1("printf", args1);
	StormByte::System::Process proc2("grep", args2);
	StormByte::System::Process proc3("sort");
	StormByte::System::Process proc4("wc", args4);
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
	StormByte::System::Process proc1("printf", args1);
	StormByte::System::Process proc2("sort");
	StormByte::System::Process proc3("uniq");
	StormByte::System::Process proc4("wc", args4);
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
	StormByte::System::Process proc("echo", args);
	std::ostringstream oss;
	oss << proc;
	ASSERT_EQUAL("process_to_ostream", "Hello, World!\n", oss.str());
	proc.Wait();
	RETURN_TEST("process_to_ostream", 0);
}
int test_stdin_roundtrip() {
	// cat copies stdin → stdout
	StormByte::System::Process proc("cat");
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
	StormByte::System::Process proc("sh", args);
	std::string err;
	proc.Stderr(err);
	ASSERT_EQUAL("test_stderr_capture", "err-msg", err);
	int exit_code = proc.Wait();
	ASSERT_EQUAL("test_stderr_capture", 0, exit_code);
	RETURN_TEST("test_stderr_capture", 0);
}
int test_exit_code_false() {
	StormByte::System::Process proc("false");
	int exit_code = proc.Wait();
	ASSERT_TRUE("test_exit_code_false", exit_code != 0);
	RETURN_TEST("test_exit_code_false", 0);
}
int test_exit_code_true() {
	StormByte::System::Process proc("true");
	int exit_code = proc.Wait();
	ASSERT_EQUAL("test_exit_code_true", 0, exit_code);
	RETURN_TEST("test_exit_code_true", 0);
}
int test_missing_executable() {
	ASSERT_THROWS("test_missing_executable", StormByte::System::Process("/no/such/stormbyte-executable"), StormByte::System::ExecutableNotFound);
	RETURN_TEST("test_missing_executable", 0);
}
int test_variable_expansion() {
	ASSERT_EQUAL("test_variable_expansion", "foo~bar", StormByte::System::Variable::Expand("foo~bar"));
	const char* home = std::getenv("HOME");
	if (home != nullptr && *home != '\0') {
		ASSERT_EQUAL("test_variable_expansion", std::string(home), StormByte::System::Variable::Expand("~"));
		ASSERT_EQUAL("test_variable_expansion", std::string(home) + "/a", StormByte::System::Variable::Expand("~/a"));
	}
	RETURN_TEST("test_variable_expansion", 0);
}
int test_wait_timeout() {
	StormByte::System::Process proc("sleep", { "1" });
	const int timeout_result = proc.Wait(std::chrono::milliseconds(10));
	ASSERT_EQUAL("test_wait_timeout", -1, timeout_result);
	const int exit_code = proc.Wait();
	ASSERT_EQUAL("test_wait_timeout", 0, exit_code);
	RETURN_TEST("test_wait_timeout", 0);
}
int test_wait_with_undrained_pipeline() {
	std::vector<std::string> args = { "if=/dev/zero", "bs=1048576", "count=16" };
	StormByte::System::Process producer("dd", args);
	StormByte::System::Process consumer("cat");
	producer >> consumer;
	(void)producer.Wait();
	(void)consumer.Wait();
	RETURN_TEST("test_wait_with_undrained_pipeline", 0);
}
int test_signaled_process() {
	StormByte::System::Process proc("sh", { "-c", "kill -TERM $$" });
	ASSERT_EQUAL("test_signaled_process", -1, proc.Wait());
	RETURN_TEST("test_signaled_process", 0);
}
int test_write_after_consumer_exit() {
	StormByte::System::Process proc("/bin/true");
	ASSERT_EQUAL("test_write_after_consumer_exit", 0, proc.Wait());
	ASSERT_THROWS("test_write_after_consumer_exit", proc << std::string(4096, 'x'), StormByte::System::ProcessCreationError);
	RETURN_TEST("test_write_after_consumer_exit", 0);
}
volatile sig_atomic_t wait_interrupt_signal = 0;
void wait_interrupt_handler(int) {
	wait_interrupt_signal = 1;
}
int test_wait_interrupted_by_signal() {
	struct sigaction action{};
	action.sa_handler = wait_interrupt_handler;
	sigemptyset(&action.sa_mask);
	struct sigaction previous{};
	sigaction(SIGUSR1, &action, &previous);
	const pthread_t main_thread = pthread_self();
	StormByte::System::Process proc("sleep", { "1" });
	std::thread interrupter([main_thread] {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
		pthread_kill(main_thread, SIGUSR1);
	});
	const int exit_code = proc.Wait();
	interrupter.join();
	sigaction(SIGUSR1, &previous, nullptr);
	ASSERT_EQUAL("test_wait_interrupted_by_signal", 1, wait_interrupt_signal);
	ASSERT_EQUAL("test_wait_interrupted_by_signal", 0, exit_code);
	RETURN_TEST("test_wait_interrupted_by_signal", 0);
}
int test_standard_descriptor_reuse() {
	const int saved_stdin = dup(STDIN_FILENO);
	const int saved_stdout = dup(STDOUT_FILENO);
	const int saved_stderr = dup(STDERR_FILENO);
	if (saved_stdin == -1 || saved_stdout == -1 || saved_stderr == -1)
		return 1;
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	std::string output;
	int result = 0;
	try {
		StormByte::System::Process proc("echo", { "descriptor-safe" });
		proc >> output;
		result = proc.Wait();
	} catch (...) {
		result = -1;
	}
	dup2(saved_stdin, STDIN_FILENO);
	dup2(saved_stdout, STDOUT_FILENO);
	dup2(saved_stderr, STDERR_FILENO);
	close(saved_stdin);
	close(saved_stdout);
	close(saved_stderr);
	ASSERT_EQUAL("test_standard_descriptor_reuse", 0, result);
	ASSERT_EQUAL("test_standard_descriptor_reuse", "descriptor-safe\n", output);
	RETURN_TEST("test_standard_descriptor_reuse", 0);
}
int test_move_process() {
	std::vector<std::string> args = { "moved" };
	StormByte::System::Process original("echo", args);
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
int test_move_assignment() {
	StormByte::System::Process source("echo", { "assigned" });
	StormByte::System::Process destination("echo", { "discarded" });
	destination = std::move(source);
	(void)source.Wait();
	std::string output;
	destination >> output;
	ASSERT_EQUAL("test_move_assignment", "assigned\n", output);
	ASSERT_EQUAL("test_move_assignment", 0, destination.Wait());
	RETURN_TEST("test_move_assignment", 0);
}
int test_tr_pipeline() {
	std::vector<std::string> args1 = { "%s", "abc" };
	std::vector<std::string> args2 = { "a-z", "A-Z" };
	StormByte::System::Process proc1("printf", args1);
	StormByte::System::Process proc2("tr", args2);
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
	std::vector<std::string> args = { "/d", "/c", "echo Hello, World!" };
	StormByte::System::Process proc("cmd.exe", args);
	std::string output;
	proc >> output;
	// cmd echo typically ends with \r\n; trim for robustness
	ASSERT_EQUAL("test_basic_execution_windows", "Hello, World!", Trim(output));
	DWORD exit_code = proc.Wait();
	ASSERT_EQUAL("test_basic_execution_windows", 0u, exit_code);
	RETURN_TEST("test_basic_execution_windows", 0);
}
int test_windows_argument_with_space() {
	std::vector<std::string> args = { "/d", "/c", "echo hello world" };
	StormByte::System::Process proc("cmd.exe", args);
	std::string output;
	proc >> output;
	ASSERT_EQUAL("test_windows_argument_with_space", "hello world", Trim(output));
	ASSERT_EQUAL("test_windows_argument_with_space", 0u, proc.Wait());
	RETURN_TEST("test_windows_argument_with_space", 0);
}
int test_windows_argument_with_quotes() {
	std::vector<std::string> args = { "/d", "/c", "echo hello \"world\"" };
	StormByte::System::Process proc("cmd.exe", args);
	std::string output;
	proc >> output;
	ASSERT_EQUAL("test_windows_argument_with_quotes", "hello \"world\"", Trim(output));
	ASSERT_EQUAL("test_windows_argument_with_quotes", 0u, proc.Wait());
	RETURN_TEST("test_windows_argument_with_quotes", 0);
}
int test_windows_long_environment_expansion() {
	const std::string value = "0123456789";
	_putenv_s("STORMBYTE_LONG_ENV", value.c_str());
	std::wstring input;
	for (size_t i = 0; i < 4000; ++i)
		input += L"%STORMBYTE_LONG_ENV%";
	const std::string expanded = StormByte::System::Variable::Expand(input);
	_putenv_s("STORMBYTE_LONG_ENV", "");
	ASSERT_EQUAL("test_windows_long_environment_expansion", 40000u, expanded.size());
	RETURN_TEST("test_windows_long_environment_expansion", 0);
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
	std::vector<std::string> args = { "/d", "/c", "exit /b 7" };
	StormByte::System::Process proc("cmd.exe", args);
	DWORD exit_code = proc.Wait();
	ASSERT_EQUAL("test_exit_code_windows", 7u, exit_code);
	RETURN_TEST("test_exit_code_windows", 0);
}
int test_move_process_windows() {
	std::vector<std::string> args = { "/d", "/c", "echo moved" };
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
	std::vector<std::string> args = { "/d", "/c", "dir /b" };
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
	#define RUN_TEST_CASE(test_case) do { \
		std::cerr << "[ RUN      ] " << #test_case << std::endl; \
		try { result += test_case(); } \
		catch (const std::exception& exception) { \
			std::cerr << "[ EXCEPTION ] " << #test_case << ": " << exception.what() << std::endl; \
			++result; \
		} \
		catch (...) { \
			std::cerr << "[ EXCEPTION ] " << #test_case << ": unknown exception" << std::endl; \
			++result; \
		} \
	} while (false)
#ifdef UNIX
RUN_TEST_CASE(test_basic_execution);
RUN_TEST_CASE(test_pipeline_execution);
RUN_TEST_CASE(test_pipeline_accumulated_and_future_output);
RUN_TEST_CASE(test_pipeline_destination_exits_first);
RUN_TEST_CASE(test_pipeline_reconnect);
RUN_TEST_CASE(test_pipeline_reconnect_long_lived);
RUN_TEST_CASE(test_pipeline_sort);
RUN_TEST_CASE(test_pipeline_find_sort_wc);
RUN_TEST_CASE(test_pipeline_echo_sort_wc);
RUN_TEST_CASE(process_to_ostream);
RUN_TEST_CASE(test_stdin_roundtrip);
RUN_TEST_CASE(test_stderr_capture);
RUN_TEST_CASE(test_exit_code_false);
RUN_TEST_CASE(test_exit_code_true);
RUN_TEST_CASE(test_missing_executable);
RUN_TEST_CASE(test_variable_expansion);
RUN_TEST_CASE(test_wait_timeout);
RUN_TEST_CASE(test_wait_with_undrained_pipeline);
RUN_TEST_CASE(test_signaled_process);
RUN_TEST_CASE(test_wait_interrupted_by_signal);
RUN_TEST_CASE(test_standard_descriptor_reuse);
RUN_TEST_CASE(test_move_process);
RUN_TEST_CASE(test_move_assignment);
RUN_TEST_CASE(test_tr_pipeline);
RUN_TEST_CASE(test_write_after_consumer_exit);
#elif defined(WINDOWS)
RUN_TEST_CASE(test_basic_execution_windows);
RUN_TEST_CASE(test_windows_argument_with_space);
RUN_TEST_CASE(test_windows_argument_with_quotes);
RUN_TEST_CASE(test_stdin_roundtrip_windows);
RUN_TEST_CASE(test_exit_code_windows);
RUN_TEST_CASE(test_move_process_windows);
RUN_TEST_CASE(test_dir_lists_something);
RUN_TEST_CASE(test_windows_long_environment_expansion);
#endif
	#undef RUN_TEST_CASE
	if (result == 0) {
		std::cout << "All tests passed!" << std::endl;
	} else {
		std::cout << result << " tests failed." << std::endl;
	}
	return result;
}
