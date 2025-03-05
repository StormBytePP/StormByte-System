cmake_minimum_required(VERSION 3.12)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project("StormByte C++ Library: System module"
	VERSION 1.0.0
	DESCRIPTION "StormByte-System is a StormByte library module for encapsulating different process handling over different operating system's complexities"
	HOMEPAGE_URL "https://dev.stormbyte.org/StormByte-System"
)

enable_testing()
add_subdirectory(thirdparty)
add_subdirectory(doc)
add_subdirectory(lib)
add_subdirectory(test)
