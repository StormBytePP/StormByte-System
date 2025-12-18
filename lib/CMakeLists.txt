include(GNUInstallDirs)

# Sources
file(GLOB_RECURSE STORMBYTE_SYSTEM_SOURCES CONFIGURE_DEPEND "${CMAKE_CURRENT_LIST_DIR}/*.cxx")

# Library
add_library(StormByte-System SHARED ${STORMBYTE_SYSTEM_SOURCES})
add_library(StormByte::System ALIAS StormByte-System)
set_target_properties(StormByte-System PROPERTIES
	LINKER_LANGUAGE CXX
	SOVERSION		${CMAKE_PROJECT_VERSION}
	VERSION 		${CMAKE_PROJECT_VERSION}
)
set_target_properties(StormByte-System PROPERTIES
	LINKER_LANGUAGE CXX
	SOVERSION		${CMAKE_PROJECT_VERSION}
	VERSION 		${CMAKE_PROJECT_VERSION}
)

# Compile options
if(MSVC)
	target_compile_options(StormByte-System PRIVATE /EHsc)
	target_compile_options(StormByte-System PRIVATE $<$<CONFIG:Release>:/O2>)
	target_compile_options(StormByte-System PRIVATE $<$<CONFIG:Debug>:/Od>)
	target_link_options(StormByte-System PRIVATE $<$<CONFIG:Release>:/LTCG /GL>)
else()
	set(CMAKE_CXX_FLAGS_DEBUG "-pipe -g -ggdb -Wall -Wextra -Wnon-virtual-dtor -pedantic -pedantic-errors -O0")
	target_compile_options(StormByte-System PRIVATE -fvisibility=hidden $<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>)
endif()

# Include directories
target_include_directories(StormByte-System
	SYSTEM BEFORE PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public" "${CMAKE_CURRENT_LIST_DIR}/private"
)

