cmake_minimum_required(VERSION 3.14)
project(hazm_cpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library target
add_library(hazm STATIC
    src/utils.cpp
    src/stemmer.cpp
    src/conjugation.cpp
    src/word_tokenizer.cpp
    src/lemmatizer.cpp
    src/stopwords.cpp
)

target_include_directories(hazm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Define data directory path
target_compile_definitions(hazm PRIVATE
    HAZM_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/"
)

# Tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.4
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)

    add_executable(hazm_tests
        tests/test_stemmer.cpp
        tests/test_conjugation.cpp
        tests/test_lemmatizer.cpp
        tests/test_stopwords.cpp
        tests/test_cross_validate.cpp
        tests/test_sample_texts.cpp
    )

    target_link_libraries(hazm_tests PRIVATE hazm Catch2::Catch2WithMain)
    target_include_directories(hazm_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

    include(CTest)
    catch_discover_tests(hazm_tests)
endif()