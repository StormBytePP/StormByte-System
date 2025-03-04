#pragma once

#define RETURN_TEST(fn_name, fn_result) if (fn_result != 0) { \
	std::cerr << "Test " << fn_name << " FAILED!" << std::endl; \
} \
return fn_result;

#define CurrentFileDirectory std::filesystem::path(__FILE__).parent_path()

#define ASSERT_EQUAL(fn_name, expected, actual) if ((expected) != (actual)) { \
	std::cerr << fn_name << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": expected \"" << (expected) << "\", got \"" << (actual) << "\"" << std::endl; \
	return 1; \
}

#define ASSERT_FALSE(fn_name, condition) if ((condition)) { \
	std::cerr << fn_name << ": Assertion failed at " << __FILE__ << ":" << __LINE__ << ": condition is true, expected false" << std::endl; \
	return 1; \
}
