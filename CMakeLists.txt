cmake_minimum_required(VERSION 3.10)
project(NormalizeCSV)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

find_package(ICU REQUIRED COMPONENTS uc i18n io)

# Find ONNX Runtime
# set(onnxruntime_ROOT "/path/to/onnxruntime/install") # if not installed in the default path
find_package(onnxruntime REQUIRED)

# Find the espeak-ng library
find_package(PkgConfig REQUIRED)
pkg_check_modules(ESPEAK REQUIRED espeak-ng)

# List all source files
add_executable(${PROJECT_NAME} 
    NormalizeCSVMain.cpp 
    normalize.cpp
    replacementHelpers.cpp
    language_detector/language_detector.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE language_detector)

target_link_libraries(${PROJECT_NAME} PRIVATE onnxruntime::onnxruntime)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ICU::uc    # Core Unicode support
        ICU::i18n  # Internationalization
        ICU::io
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${ESPEAK_INCLUDE_DIRS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${ESPEAK_LIBRARIES})