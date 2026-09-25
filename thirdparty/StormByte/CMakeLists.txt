if(WIN32)
	set(WITH_STORMBYTE "BUNDLED" CACHE STRING "Use StormByte components (SYSTEM/BUNDLED)" FORCE)
else()
	set(WITH_STORMBYTE "BUNDLED" CACHE STRING "Use StormByte components (SYSTEM/BUNDLED)")
endif()

if(WITH_STORMBYTE STREQUAL "SYSTEM")
	find_package(StormByte REQUIRED String)
else()
	add_subdirectory(string)
endif()

buildmaster_link(StormByte-System StormByte-String)
