cmake_minimum_required(VERSION 3.16)

project(riz-embedded-platform LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(RIZ_ENABLE_ASAN "Enable AddressSanitizer and LeakSanitizer" OFF)

if(RIZ_ENABLE_ASAN)
    message(STATUS "Enabling AddressSanitizer (ASan) and LeakSanitizer (LSan)")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
endif()

add_subdirectory(src)
add_subdirectory(tests)
