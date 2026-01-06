set(STORMBYTE_BASE_SRCDIR "${CMAKE_CURRENT_LIST_DIR}/src")
ensure_build_dir(STORMBYTE_BASE_BUILDDIR)
set(STORMBYTE_BASE_COMPONENT "StormByte Suite - Base")
set(STORMBYTE_BASE_OPTIONS "")
create_cmake_component(
	STORMBYTE_BASE_LIBRARY_CREATE_FILE
	"StormByte"
	"${STORMBYTE_BASE_COMPONENT}"
	"${STORMBYTE_BASE_SRCDIR}"
	"${STORMBYTE_BASE_BUILDDIR}"
	"${STORMBYTE_BASE_OPTIONS}"
	"shared"
	"StormByte"
	1
)
include("${STORMBYTE_BASE_LIBRARY_CREATE_FILE}")
add_dependencies(StormByte-System StormByte_install)

target_link_libraries(StormByte-System PUBLIC StormByte)