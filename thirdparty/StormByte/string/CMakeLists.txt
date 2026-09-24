set(STORMBYTE_STRING_OPTSTR "ALIAS=StormByte::String")
if(MSVC)
	string(PREPEND STORMBYTE_STRING_OPTSTR "TOOLCHAIN=clang-cl;")
endif()

buildmaster_component(
	StormByte-String
	"StormByte Suite - String"
	"${CMAKE_CURRENT_LIST_DIR}/src"
	"WITH_STORMBYTE=BUNDLED"
	shared
	"StormByte-String"
	"${STORMBYTE_STRING_OPTSTR}"
)
