cmake_minimum_required(VERSION 3.20)
project(cstd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(cstd
	crt.cpp
	../common/main.cpp
)

target_include_directories(cstd PRIVATE ../../src)

target_compile_options(cstd PRIVATE
	-fno-exceptions
	-fno-rtti
	-mpopcnt
	-Wall
	-Wextra
	-Wpedantic
)

target_link_options(cstd PRIVATE
	-nodefaultlibs
	-lc
)
