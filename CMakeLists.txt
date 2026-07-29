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

# ---- hazm_cpp sources (compiled directly) ----
set(HAZM_SOURCES
    hazm_cpp/src/utils.cpp
    hazm_cpp/src/stemmer.cpp
    hazm_cpp/src/conjugation.cpp
    hazm_cpp/src/word_tokenizer.cpp
    hazm_cpp/src/lemmatizer.cpp
    hazm_cpp/src/stopwords.cpp
)

# ---- ezafe_detector sources ----
set(EZAFE_SOURCES
    persian-ezafe-albert-cpp/src/ezafe_detector.cpp
    persian-ezafe-albert-cpp/third_party/sentencepiece_minimal/sentencepiece_minimal.cpp
)

# ---- Homograph + Persian post-process sources ----
set(PERSIAN_POSTPROCESS_SOURCES
    homograph/homograph_dict.cpp
    homograph/homograph_disambiguator.cpp
    persian_phoneme/persian_postprocess.cpp
)

# List all source files
add_executable(${PROJECT_NAME} 
    NormalizeCSVMain.cpp 
    normalize.cpp
    replacements/replacements.cpp
    replacements/replacementHelpers.cpp
    language_detector/language_detector.cpp
    espeak_phonemize/phonemize.cpp
    espeak_phonemize/phoneme_ids.cpp
    shakkelha/shakkelha.cpp
    nawar-halabi/ar_phonemizer.cpp
    vits2-tokenizer/vits2-tokenizer.cpp
    ${HAZM_SOURCES}
    ${EZAFE_SOURCES}
    ${PERSIAN_POSTPROCESS_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    language_detector
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/homograph
    ${CMAKE_CURRENT_SOURCE_DIR}/persian_phoneme
    hazm_cpp/include
    persian-ezafe-albert-cpp/src
    persian-ezafe-albert-cpp/third_party/sentencepiece_minimal
)

# Define data directory for hazm_cpp (point to assets in build dir)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    HAZM_DATA_DIR="./assets/"
)

# ONNX
target_link_libraries(${PROJECT_NAME} PRIVATE onnxruntime::onnxruntime)

# ICU
target_link_libraries(${PROJECT_NAME} PRIVATE ICU::uc ICU::i18n ICU::io)

# Espeak
target_include_directories(${PROJECT_NAME} PRIVATE ${ESPEAK_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${ESPEAK_LIBRARIES})

# Copy all asset files (models, data) to build dir
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/)
message(" Copied assets/ to " ${CMAKE_CURRENT_BINARY_DIR}/assets/)