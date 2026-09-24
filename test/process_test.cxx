/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-System.
 *
 * StormByte-System original source is dual-licensed:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You may redistribute and/or modify this file under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this file may be used under the terms of a commercial
 *    license agreement with the copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *
 * Both licenses apply only to original StormByte-System source in this
 * repository. They do not cover other StormByte modules or any third-party
 * material shipped with this repository (including everything under
 * thirdparty/, and in particular the bundled StormByte Base tree), which
 * remains under its own license.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte-System is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte-System. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#include <StormByte/string/string.hxx>
#include <StormByte/system/exception.hxx>
#include <StormByte/system/process.hxx>
#include <StormByte/system/variable.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#ifdef UNIX
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#endif

using StormByte::String::String;
using StormByte::System::Process;

namespace {
	std::string Trim(std::string s) {
		auto not_space = [](unsigned char c) { return !std::isspace(c); };
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
		s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
		return s;
	}

	std::vector<String> Args(std::initializer_list<const char*> items) {
		std::vector<String> out;
		out.reserve(items.size());
		for (const char* item : items)
			out.emplace_back(item);
		return out;
	}

}

#ifdef UNIX
// -------------------
// Basic
// -------------------
int test_basic_execution() {
	Process proc("echo", Args({"Hello, World!"}));
	std::string output;
	proc >> output;
	ASSERT_EQUAL("test_basic_execution", "Hello, World!\n", output);
	int exit_code = proc.Wait();
	ASSERT_EQUAL("test_basic_execution", 0, exit_code);
	RETURN_TEST("test_basic_execution", 0);
}

int test_missing_executable() {
	ASSERT_THROWS("test_missing_executable", Process("/no/such/stormbyte-executable"), StormByte::System::ExecutableNotFound);
	RETURN_TEST("test_missing_executable", 0);
}

int test_process_to_ostream() {
	Process proc("echo", Args({"Hello, World!"}));
	std::ostringstream oss;
	oss << proc;
	ASSERT_EQUAL("test_process_to_ostream", "Hello, World!\n", oss.str());
	proc.Wait();
	RETURN_TEST("test_process_to_ostream", 0);
}

// -------------------
// Exit
// -------------------
int test_exit_code_false() {
	Process proc("false");
	int exit_code = proc.Wait();
	ASSERT_TRUE("test_exit_code_false", exit_code != 0);
	RETURN_TEST("test_exit_code_false", 0);
}

int test_exit_code_true() {
	Process proc("true");
	int exit_code = proc.Wait();
	ASSERT_EQUAL("test_exit_code_true", 0, exit_code);
	RETURN_TEST("test_exit_code_true", 0);
}

int test_signaled_process() {
	Process proc("sh", Args({"-c", "kill -TERM $$"}));
	ASSERT_EQUAL("test_signaled_process", -1, proc.Wait());
	RETURN_TEST("test_signaled_process", 0);
}

// -------------------
// Move
// -------------------
int test_move_assignment() {
	Process source("echo", Args({"assigned"}));
	Process destination("echo", Args({"discarded"}));
	destination = std::move(source);
	(void)source.Wait();
	std::string output;
	destination >> output;
	ASSERT_EQUAL("test_move_assignment", "assigned\n", output);
	ASSERT_EQUAL("test_move_assignment", 0, destination.Wait());
	RETURN_TEST("test_move_assignment", 0);
}

int test_move_process() {
	Process original("echo", Args({"moved"}));
	Process moved(std::move(original));
	(void)original.Wait();
	std::string output;
	moved >> output;
	ASSERT_EQUAL("test_move_process", "moved\n", output);
	int exit_code = moved.Wait();
	ASSERT_EQUAL("test_move_process", 0, exit_code);
	RETURN_TEST("test_move_process", 0);
}

// -------------------
// Pipeline
// -------------------
int test_pipeline_accumulated_and_future_output() {
	Process producer("cat");
	Process consumer("cat");
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
	Process producer("yes");
	{
		Process consumer("head", Args({"-c", "1"}));
		producer >> consumer;
		std::string output;
		consumer >> output;
		ASSERT_EQUAL("test_pipeline_destination_exits_first", 1u, output.size());
	}

	producer.Wait();
	RETURN_TEST("test_pipeline_destination_exits_first", 0);
}

int test_pipeline_echo_sort_wc() {
	Process proc1("printf", Args({"%s", "orange\nbanana\napple\ncherry\nbanana\napple\n"}));
	Process proc2("sort");
	Process proc3("uniq");
	Process proc4("wc", Args({"-l"}));
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

int test_pipeline_execution() {
	Process proc1("printf", Args({"%s", "Hello\n"}));
	Process proc2("wc", Args({"-c"}));
	proc1 >> proc2;
	std::string output;
	proc2 >> output;
	ASSERT_EQUAL("test_pipeline_execution", "6", Trim(output));
	proc1.Wait();
	proc2.Wait();
	RETURN_TEST("test_pipeline_execution", 0);
}

int test_pipeline_find_sort_wc() {
	Process proc1("printf", Args({"%s", "apple\nbanana\ncherry\napple\nbanana\ncherry\n"}));
	Process proc2("grep", Args({"apple"}));
	Process proc3("sort");
	Process proc4("wc", Args({"-l"}));
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

int test_pipeline_reconnect() {
	Process producer("cat");
	Process first_consumer("cat");
	Process second_consumer("cat");
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
	Process producer("sleep", Args({"1"}));
	Process first_consumer("cat");
	Process second_consumer("cat");
	producer >> first_consumer;
	producer >> second_consumer;
	ASSERT_EQUAL("test_pipeline_reconnect_long_lived", 0, producer.Wait());
	ASSERT_EQUAL("test_pipeline_reconnect_long_lived", 0, first_consumer.Wait());
	ASSERT_EQUAL("test_pipeline_reconnect_long_lived", 0, second_consumer.Wait());
	RETURN_TEST("test_pipeline_reconnect_long_lived", 0);
}

int test_pipeline_sort() {
	Process proc1("printf", Args({"%s", "banana\napple\ncherry\n"}));
	Process proc2("sort");
	proc1 >> proc2;
	std::string output;
	proc2 >> output;
	ASSERT_EQUAL("test_pipeline_sort", "apple\nbanana\ncherry\n", output);
	proc1.Wait();
	proc2.Wait();
	RETURN_TEST("test_pipeline_sort", 0);
}

int test_tr_pipeline() {
	Process proc1("printf", Args({"%s", "abc"}));
	Process proc2("tr", Args({"a-z", "A-Z"}));
	proc1 >> proc2;
	std::string output;
	proc2 >> output;
	ASSERT_EQUAL("test_tr_pipeline", "ABC", output);
	proc1.Wait();
	proc2.Wait();
	RETURN_TEST("test_tr_pipeline", 0);
}

// -------------------
// Stderr
// -------------------
int test_stderr_capture() {
	Process proc("sh", Args({"-c", "printf '%s' 'err-msg' 1>&2"}));
	std::string err;
	proc.Stderr(err);
	ASSERT_EQUAL("test_stderr_capture", "err-msg", err);
	int exit_code = proc.Wait();
	ASSERT_EQUAL("test_stderr_capture", 0, exit_code);
	RETURN_TEST("test_stderr_capture", 0);
}

// -------------------
// Stdin
// -------------------
int test_stdin_roundtrip() {
	Process proc("cat");
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

int test_write_after_consumer_exit() {
	Process proc("true");
	ASSERT_EQUAL("test_write_after_consumer_exit", 0, proc.Wait());
	ASSERT_THROWS("test_write_after_consumer_exit", proc << std::string_view(std::string(4096, 'x')), StormByte::System::ProcessCreationError);
	RETURN_TEST("test_write_after_consumer_exit", 0);
}

// -------------------
// Variable
// -------------------
int test_variable_expansion() {
	ASSERT_EQUAL("test_variable_expansion", "foo~bar", std::string(StormByte::System::Variable::Expand("foo~bar")));
	const char* home = std::getenv("HOME");
	if (home != nullptr && *home != '\0') {
		ASSERT_EQUAL("test_variable_expansion", std::string(home), std::string(StormByte::System::Variable::Expand("~")));
		ASSERT_EQUAL("test_variable_expansion", std::string(home) + "/a", std::string(StormByte::System::Variable::Expand("~/a")));
	}

	RETURN_TEST("test_variable_expansion", 0);
}

// -------------------
// Wait
// -------------------
volatile sig_atomic_t wait_interrupt_signal = 0;

void wait_interrupt_handler(int) {
	wait_interrupt_signal = 1;
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
		Process proc("echo", Args({"descriptor-safe"}));
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

int test_wait_interrupted_by_signal() {
	struct sigaction action{};
	action.sa_handler = wait_interrupt_handler;
	sigemptyset(&action.sa_mask);
	struct sigaction previous{};
	sigaction(SIGUSR1, &action, &previous);
	const pthread_t main_thread = pthread_self();
	Process proc("sleep", Args({"1"}));
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

int test_wait_timeout() {
	Process proc("sleep", Args({"1"}));
	const int timeout_result = proc.Wait(std::chrono::milliseconds(10));
	ASSERT_EQUAL("test_wait_timeout", -1, timeout_result);
	const int exit_code = proc.Wait();
	ASSERT_EQUAL("test_wait_timeout", 0, exit_code);
	RETURN_TEST("test_wait_timeout", 0);
}

int test_wait_with_undrained_pipeline() {
	Process producer("dd", Args({"if=/dev/zero", "bs=1048576", "count=16"}));
	Process consumer("cat");
	producer >> consumer;
	(void)producer.Wait();
	(void)consumer.Wait();
	RETURN_TEST("test_wait_with_undrained_pipeline", 0);
}

#elifdef WINDOWS
// -------------------
// Basic
// -------------------
int test_basic_execution_windows() {
	Process proc("cmd.exe", Args({"/d", "/c", "echo Hello, World!"}));
	std::string output;
	proc >> output;
	ASSERT_EQUAL("test_basic_execution_windows", "Hello, World!", Trim(output));
	DWORD exit_code = proc.Wait();
	ASSERT_EQUAL("test_basic_execution_windows", 0u, exit_code);
	RETURN_TEST("test_basic_execution_windows", 0);
}

int test_dir_lists_something() {
	Process proc("cmd.exe", Args({"/d", "/c", "dir /b"}));
	std::string output;
	proc >> output;
	ASSERT_FALSE("test_dir_lists_something", Trim(output).empty());
	DWORD exit_code = proc.Wait();
	ASSERT_EQUAL("test_dir_lists_something", 0u, exit_code);
	RETURN_TEST("test_dir_lists_something", 0);
}

int test_windows_argument_with_quotes() {
	Process proc("cmd.exe", Args({"/d", "/c", "echo hello \"world\""}));
	std::string output;
	proc >> output;
	ASSERT_EQUAL("test_windows_argument_with_quotes", "hello \"world\"", Trim(output));
	ASSERT_EQUAL("test_windows_argument_with_quotes", 0u, proc.Wait());
	RETURN_TEST("test_windows_argument_with_quotes", 0);
}

int test_windows_argument_with_space() {
	Process proc("cmd.exe", Args({"/d", "/c", "echo hello world"}));
	std::string output;
	proc >> output;
	ASSERT_EQUAL("test_windows_argument_with_space", "hello world", Trim(output));
	ASSERT_EQUAL("test_windows_argument_with_space", 0u, proc.Wait());
	RETURN_TEST("test_windows_argument_with_space", 0);
}

// -------------------
// Exit
// -------------------
int test_exit_code_windows() {
	Process proc("cmd.exe", Args({"/d", "/c", "exit /b 7"}));
	DWORD exit_code = proc.Wait();
	ASSERT_EQUAL("test_exit_code_windows", 7u, exit_code);
	RETURN_TEST("test_exit_code_windows", 0);
}

// -------------------
// Move
// -------------------
int test_move_process_windows() {
	Process original("cmd.exe", Args({"/d", "/c", "echo moved"}));
	Process moved(std::move(original));
	(void)original.Wait();
	std::string output;
	moved >> output;
	ASSERT_EQUAL("test_move_process_windows", "moved", Trim(output));
	DWORD exit_code = moved.Wait();
	ASSERT_EQUAL("test_move_process_windows", 0u, exit_code);
	RETURN_TEST("test_move_process_windows", 0);
}

// -------------------
// Stdin
// -------------------
int test_stdin_roundtrip_windows() {
	Process proc("sort.exe");
	proc << "b\r\n";
	proc << "a\r\n";
	proc << StormByte::System::EoF;
	std::string output;
	proc >> output;
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

// -------------------
// Variable
// -------------------
int test_windows_long_environment_expansion() {
	const std::string value = "0123456789";
	_putenv_s("STORMBYTE_LONG_ENV", value.c_str());
	std::wstring input;
	for (size_t i = 0; i < 4000; ++i)
		input += L"%STORMBYTE_LONG_ENV%";
	const String expanded = StormByte::System::Variable::Expand(std::wstring_view(input));
	_putenv_s("STORMBYTE_LONG_ENV", "");
	ASSERT_EQUAL("test_windows_long_environment_expansion", 40000u, expanded.size());
	RETURN_TEST("test_windows_long_environment_expansion", 0);
}

#endif

int main() {
	int result = 0;
#ifdef UNIX
	// -------------------
	// Basic
	// -------------------
	result += test_basic_execution();
	result += test_missing_executable();
	result += test_process_to_ostream();

	// -------------------
	// Exit
	// -------------------
	result += test_exit_code_false();
	result += test_exit_code_true();
	result += test_signaled_process();

	// -------------------
	// Move
	// -------------------
	result += test_move_assignment();
	result += test_move_process();

	// -------------------
	// Pipeline
	// -------------------
	result += test_pipeline_accumulated_and_future_output();
	result += test_pipeline_destination_exits_first();
	result += test_pipeline_echo_sort_wc();
	result += test_pipeline_execution();
	result += test_pipeline_find_sort_wc();
	result += test_pipeline_reconnect();
	result += test_pipeline_reconnect_long_lived();
	result += test_pipeline_sort();
	result += test_tr_pipeline();

	// -------------------
	// Stderr
	// -------------------
	result += test_stderr_capture();

	// -------------------
	// Stdin
	// -------------------
	result += test_stdin_roundtrip();
	result += test_write_after_consumer_exit();

	// -------------------
	// Variable
	// -------------------
	result += test_variable_expansion();

	// -------------------
	// Wait
	// -------------------
	result += test_standard_descriptor_reuse();
	result += test_wait_interrupted_by_signal();
	result += test_wait_timeout();
	result += test_wait_with_undrained_pipeline();
#elifdef WINDOWS
	// -------------------
	// Basic
	// -------------------
	result += test_basic_execution_windows();
	result += test_dir_lists_something();
	result += test_windows_argument_with_quotes();
	result += test_windows_argument_with_space();

	// -------------------
	// Exit
	// -------------------
	result += test_exit_code_windows();

	// -------------------
	// Move
	// -------------------
	result += test_move_process_windows();

	// -------------------
	// Stdin
	// -------------------
	result += test_stdin_roundtrip_windows();

	// -------------------
	// Variable
	// -------------------
	result += test_windows_long_environment_expansion();
#endif
	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
