cmake_minimum_required(VERSION 3.10)
project(CSVProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# List all source files
add_executable(CSVProcessor 
    main.cpp 
    replacement.cpp
    language_detector/language_detector.cpp
)

include_directories(language_detector)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
