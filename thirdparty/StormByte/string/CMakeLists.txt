set(STORMBYTE_STRING_OPTSTR "ALIAS=StormByte::String")
if(MSVC)
	string(PREPEND STORMBYTE_STRING_OPTSTR "TOOLCHAIN=clang-cl;")
endif()

if(BUILD_SHARED_LIBS)
	set(STORMBYTE_STRING_MODE shared)
else()
	set(STORMBYTE_STRING_MODE static)
endif()

buildmaster_component(
	StormByte-String
	"StormByte Suite - String"
	"${CMAKE_CURRENT_LIST_DIR}/src"
	"WITH_STORMBYTE=BUNDLED;ENABLE_TEST=OFF"
	${STORMBYTE_STRING_MODE}
	"StormByte-String"
	"${STORMBYTE_STRING_OPTSTR}"
)
