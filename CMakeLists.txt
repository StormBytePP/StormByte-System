cmake_minimum_required(VERSION 3.12)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project("StormByteSystem"
	VERSION 1.0.0
	DESCRIPTION "StormByte Config is a StormByte library for parsing configuration files"
	HOMEPAGE_URL "https://github.com/StormBytePP/StormByteSystem"
)

add_subdirectory(thirdparty)
add_subdirectory(doc)
add_subdirectory(lib)
add_subdirectory(test)
