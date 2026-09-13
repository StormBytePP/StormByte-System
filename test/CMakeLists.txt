option(ENABLE_TEST "Enable Unit Tests" OFF)
if(ENABLE_TEST)
	enable_testing()
	
	add_executable(ProcessTests process_test.cxx)
	target_link_libraries(ProcessTests StormByte::System)
	add_test(NAME ProcessTests COMMAND ProcessTests)
	set_tests_properties(ProcessTests PROPERTIES TIMEOUT 15)
endif()