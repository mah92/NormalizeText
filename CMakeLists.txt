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
    replacements/replacements.cpp
    replacements/replacementHelpers.cpp
    language_detector/language_detector.cpp
    espeak_phonemize/phonemize.cpp
    espeak_phonemize/phoneme_ids.cpp
    nawar-halabi/ar_phonemizer.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE language_detector)

# ONNX
target_link_libraries(${PROJECT_NAME} PRIVATE onnxruntime::onnxruntime)

# ICU
target_link_libraries(${PROJECT_NAME} PRIVATE ICU::uc ICU::i18n ICU::io)

# Espeak
target_include_directories(${PROJECT_NAME} PRIVATE ${ESPEAK_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${ESPEAK_LIBRARIES})

# Find all .onnx files recursively from the source directory
# Copy them to the build directory
file(GLOB_RECURSE ONNX_FILES "${CMAKE_CURRENT_SOURCE_DIR}/shakkelha/*.onnx")
file(COPY ${ONNX_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
message(" Copied: " ${ONNX_FILES} " To " ${CMAKE_CURRENT_BINARY_DIR})
