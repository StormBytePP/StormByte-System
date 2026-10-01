set(STORMBYTE_BASE_OPTSTR "ALIAS=StormByte::Base")
if(MSVC)
	string(PREPEND STORMBYTE_BASE_OPTSTR "TOOLCHAIN=clang-cl;")
endif()

if(BUILD_SHARED_LIBS)
	set(STORMBYTE_BASE_MODE shared)
else()
	set(STORMBYTE_BASE_MODE static)
endif()

buildmaster_component(
	StormByte-Base
	"StormByte Suite - Base"
	"${CMAKE_CURRENT_LIST_DIR}/src"
	"ENABLE_TEST=OFF"
	${STORMBYTE_BASE_MODE}
	"StormByte"
	"${STORMBYTE_BASE_OPTSTR}"
)
