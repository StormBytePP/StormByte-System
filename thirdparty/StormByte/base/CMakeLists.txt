if(MSVC)
	set(STORMBYTE_BASE_OPTSTR "TOOLCHAIN=clang-cl;ALIAS=StormByte::StormByte")
else()
	set(STORMBYTE_BASE_OPTSTR "ALIAS=StormByte::StormByte")
endif()

buildmaster_component(
	StormByte
	"StormByte Suite - Base"
	"${CMAKE_CURRENT_LIST_DIR}/src"
	""
	shared
	"StormByte"
	"${STORMBYTE_BASE_OPTSTR}"
)
