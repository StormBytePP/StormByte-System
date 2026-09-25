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

# Temporary: StormByte-String PUBLIC-links Base in-tree, but BuildMaster
# does not forward that INTERFACE across an installed component. Name
# StormByte here until BM transitive deps work for a nested consumer.
buildmaster_link(StormByte-System StormByte StormByte-String)
