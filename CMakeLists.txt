cmake_minimum_required(VERSION 3.7)
project(OSFileSys)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c Block.h Block.c Config.h FileSystem.c FileSystem.h)
add_executable(OSFileSys ${SOURCE_FILES})