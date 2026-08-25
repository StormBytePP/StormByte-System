include(GNUInstallDirs)

file(GLOB_RECURSE STORMBYTE_SYSTEM_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cxx")

add_library(StormByte-System SHARED ${STORMBYTE_SYSTEM_SOURCES})
add_library(StormByte::System ALIAS StormByte-System)

set_target_properties(StormByte-System PROPERTIES
	LINKER_LANGUAGE CXX
	SOVERSION ${CMAKE_PROJECT_VERSION}
	VERSION ${CMAKE_PROJECT_VERSION}
	POSITION_INDEPENDENT_CODE ON
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

if(MSVC)
	target_compile_options(StormByte-System PRIVATE
		/EHsc
		$<$<CONFIG:Release>:/O2>
		$<$<CONFIG:Debug>:/Od>
	)
	# No /LTCG /GL on link — breaks lld-link; use INTERPROCEDURAL_OPTIMIZATION instead
else()
	target_compile_options(StormByte-System PRIVATE
		-fvisibility=hidden
		$<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>
		$<$<CONFIG:Debug>:-pipe -g -ggdb -Wall -Wextra -Wnon-virtual-dtor -pedantic -pedantic-errors -O0>
	)
endif()

target_include_directories(StormByte-System
	SYSTEM BEFORE PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/public"
		"${CMAKE_CURRENT_LIST_DIR}/private"
)
