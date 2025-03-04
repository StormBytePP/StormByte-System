
option(ENABLE_DOC "Enable documentation generation with Doxygen" OFF)
if(ENABLE_DOC)
	find_package(Doxygen REQUIRED)
	set(DOXYGEN_PROJECT_NAME "StormByte C++ Library")
	set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION}")
	set(DOXYGEN_PROJECT_BRIEF "${PROJECT_DESCRIPTION}")
	set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/docs")
	set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md")
	set(DOXYGEN_EXCLUDE_PATTERNS
		"*/CMakeLists.txt"
		"*/.git/*"
		"*/doc/*"
		"*/thirdparty/*"
		"*/build/*"
		"*/*.cxx"
	)
	set(DOXYGEN_GENERATE_HTML YES)
	set(DOXYGEN_HTML_OUTPUT ".")
	set(DOXYGEN_GENERATE_MAN NO)
	set(DOXYGEN_GENERATE_LATEX NO)

	doxygen_add_docs(
		StormByteSystemDoc
		"${CMAKE_SOURCE_DIR}"
		COMMENT "Generate HTML pages"
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	)
endif()