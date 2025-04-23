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

# List all source files
add_executable(${PROJECT_NAME} 
    main.cpp 
    replacement.cpp
    language_detector/language_detector.cpp
)

include_directories(language_detector)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE
        ICU::uc    # Core Unicode support
        ICU::i18n  # Internationalization
        ICU::io
)