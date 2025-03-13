#include <StormByte/system/process.hxx>
#include <StormByte/test_handlers.h>

#ifdef LINUX
int test_basic_execution() {
    // Test a simple command that prints "Hello, World!".
    std::vector<std::string> args = { "Hello, World!" };
    StormByte::System::Process proc("/bin/echo", args);
    
    std::string output;
    proc >> output;

    ASSERT_EQUAL("test_basic_execution", "Hello, World!\n", output);

    int exit_code = proc.Wait();
    ASSERT_EQUAL("test_basic_execution", 0, exit_code);

    return 0;
}

int test_pipeline_execution() {
    // Test a pipeline between two commands.
    std::vector<std::string> args1 = { "Hello" };
    std::vector<std::string> args2 = { "-c" };

    StormByte::System::Process proc1("/bin/echo", args1);
    StormByte::System::Process proc2("/usr/bin/wc", args2);

    proc1 >> proc2;

    std::string output;
    proc2 >> output;

    ASSERT_EQUAL("test_pipeline_execution", "6\n", output);  // "Hello\n" has 6 characters including the newline.

    proc1.Wait();
    proc2.Wait();

    return 0;
}

int test_pipeline_sort() {
    // Test a pipeline that sorts input.
    std::vector<std::string> args1 = { "-e", "banana\napple\ncherry" };

    StormByte::System::Process proc1("/bin/echo", args1);
    StormByte::System::Process proc2("/usr/bin/sort");

    proc1 >> proc2;

    std::string output;
    proc2 >> output;

    ASSERT_EQUAL("test_pipeline_sort", "apple\nbanana\ncherry\n", output);  // Sorted output.

    proc1.Wait();
    proc2.Wait();

    return 0;
}

int test_pipeline_find_sort_wc() {
    // Test a pipeline that finds a pattern, sorts the results, and counts lines.
    std::vector<std::string> args1 = { "-e", "apple\nbanana\ncherry\napple\nbanana\ncherry" };
    std::vector<std::string> args2 = { "apple" };
    std::vector<std::string> args4 = { "-l" };

    StormByte::System::Process proc1("/bin/echo", args1);
    StormByte::System::Process proc2("/bin/grep", args2);
    StormByte::System::Process proc3("/usr/bin/sort");
    StormByte::System::Process proc4("/usr/bin/wc", args4);

    proc1 >> proc2 >> proc3 >> proc4;

    std::string output;
    proc4 >> output;

    ASSERT_EQUAL("test_pipeline_find_sort_wc", "2\n", output);  // There are 2 lines containing "apple".

    proc1.Wait();
    proc2.Wait();
    proc3.Wait();
    proc4.Wait();

    return 0;
}

int test_pipeline_echo_sort_wc() {
    // Test a pipeline that echoes, sorts, and counts lines.
    std::vector<std::string> args1 = { "-e", "orange\nbanana\napple\ncherry\nbanana\napple" };
    std::vector<std::string> args4 = { "-l" };

    StormByte::System::Process proc1("/bin/echo", args1);
    StormByte::System::Process proc2("/usr/bin/sort");
    StormByte::System::Process proc3("/usr/bin/uniq");
    StormByte::System::Process proc4("/usr/bin/wc", args4);

    proc1 >> proc2 >> proc3 >> proc4;

    std::string output;
    proc4 >> output;

    ASSERT_EQUAL("test_pipeline_echo_sort_wc", "4\n", output);  // There are 4 unique sorted lines.

    proc1.Wait();
    proc2.Wait();
    proc3.Wait();
    proc4.Wait();

    return 0;
}

int process_to_ostream() {
	// Test that a process can be sent to an ostream.
	std::vector<std::string> args = { "Hello, World!" };
	StormByte::System::Process proc("/bin/echo", args);

	std::ostringstream oss;
	oss << proc;

	std::string output = oss.str();
	ASSERT_EQUAL("process_to_ostream", "Hello, World!\n", output);

	return 0;
}

#else
int test_basic_execution_windows() {
    // Test a simple command that prints "Hello, World!".
    std::vector<std::string> args = { "Hello, World!" };
    StormByte::System::Process proc(L"cmd.exe /c echo", args);

    std::string output;
    proc >> output;

    ASSERT_EQUAL("test_basic_execution_windows", "Hello, World! \r\n", output);

    DWORD exit_code = proc.Wait();
    ASSERT_EQUAL("test_basic_execution_windows", 0, exit_code);

    return 0;
}

int test_complex_command_windows() {
    // Test a more complex command with multiple arguments.
    std::vector<std::string> args = { "/c", "dir", "/b", "/a-d" };
    StormByte::System::Process proc(L"cmd.exe", args);

    std::string output;
    proc >> output;

    ASSERT_FALSE("test_complex_command_windows", output.empty());  // Check that output is not empty.

    DWORD exit_code = proc.Wait();
    ASSERT_EQUAL("test_complex_command_windows", 0, exit_code);

    return 0;
}
#endif

int main() {
    int result = 0;
	#ifdef LINUX
		result += test_basic_execution();
		result += test_pipeline_execution();
		result += test_pipeline_sort();
		result += test_pipeline_find_sort_wc();
		result += test_pipeline_echo_sort_wc();
		result += process_to_ostream();
	#else
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
