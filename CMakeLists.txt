cmake_minimum_required(VERSION 3.18)
project(EAPO_Cpp LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

# ——————————————————————————————————————————————
# LibTorch (PyTorch C++ API)
# ——————————————————————————————————————————————
if(NOT DEFINED Torch_DIR)
  message(FATAL_ERROR
    "You must pass -DTorch_DIR to the directory containing TorchConfig.cmake\n"
    "e.g. -DTorch_DIR=/opt/libtorch/share/cmake/Torch"
  )
endif()
list(APPEND CMAKE_PREFIX_PATH "${Torch_DIR}")
find_package(Torch REQUIRED CONFIG)

# ——————————————————————————————————————————————
# JSON (nlohmann) and Boost
# ——————————————————————————————————————————————
find_package(nlohmann_json REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)

# ——————————————————————————————————————————————
# Optional: NVML (energy sampling)
# ——————————————————————————————————————————————
option(USE_NVML "Enable NVML energy sampling" ON)
if(USE_NVML)
  find_path(NVML_INCLUDE_DIR nvml.h
    HINTS ${CUDA_TOOLKIT_ROOT_DIR}/include /usr/include
  )
  find_library(NVML_LIBRARY nvidia-ml nvml
    HINTS ${CUDA_TOOLKIT_ROOT_DIR}/lib64 /usr/lib/x86_64-linux-gnu
  )
  if(NVML_INCLUDE_DIR AND NVML_LIBRARY)
    message(STATUS "NVML support: ON (found ${NVML_LIBRARY})")
    add_compile_definitions(USE_NVML)
  else()
    message(WARNING "NVML not found; disabling NVML support")
    set(USE_NVML OFF)
  endif()
endif()

# ——————————————————————————————————————————————
# Optional: SentencePiece tokenizer
# ——————————————————————————————————————————————
option(USE_SENTENCEPIECE "Enable SentencePiece tokenizer" ON)
if(USE_SENTENCEPIECE)
  find_path(SP_INCLUDE_DIR sentencepiece_processor.h
    HINTS /usr/local/include /usr/include
  )
  find_library(SP_LIBRARY sentencepiece
    HINTS /usr/local/lib /usr/lib/x86_64-linux-gnu
  )
  if(SP_INCLUDE_DIR AND SP_LIBRARY)
    message(STATUS "SentencePiece support: ON (found ${SP_LIBRARY})")
    add_compile_definitions(USE_SENTENCEPIECE)
  else()
    message(WARNING "SentencePiece not found; disabling support")
    set(USE_SENTENCEPIECE OFF)
  endif()
endif()

# ——————————————————————————————————————————————
# Optional: HuggingFace Tokenizers
# ——————————————————————————————————————————————
option(USE_TOKENIZERS "Enable HuggingFace Tokenizers support" OFF)
if(USE_TOKENIZERS)
  find_package(tokenizers CONFIG QUIET)
  if(tokenizers_FOUND)
    message(STATUS "Tokenizers support: ON")
    add_compile_definitions(USE_TOKENIZERS)
  else()
    message(WARNING "Tokenizers not found; disabling support")
    set(USE_TOKENIZERS OFF)
  endif()
endif()

# ——————————————————————————————————————————————
# Gather all .cpp under src/
# ——————————————————————————————————————————————
file(GLOB_RECURSE ALL_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# ——————————————————————————————————————————————
# Exclude the two main* files from module list
# ——————————————————————————————————————————————
list(REMOVE_ITEM ALL_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/search_and_summary.cpp"
)

# ——————————————————————————————————————————————
# eapo_cpp: uses main.cpp + modules
# ——————————————————————————————————————————————
add_executable(eapo_cpp
  src/main.cpp
  ${ALL_SRCS}
)

target_link_libraries(eapo_cpp PRIVATE
  ${TORCH_LIBRARIES}
  nlohmann_json::nlohmann_json
  Boost::program_options
  $<$<BOOL:${USE_NVML}>:${NVML_LIBRARY}>
  $<$<BOOL:${USE_SENTENCEPIECE}>:${SP_LIBRARY}>
  $<$<BOOL:${USE_TOKENIZERS}>:tokenizers::tokenizers>
)

# RPATH for libtorch
set_target_properties(eapo_cpp PROPERTIES
  BUILD_WITH_INSTALL_RPATH TRUE
  INSTALL_RPATH "$ORIGIN/../libtorch/lib"
)

# ——————————————————————————————————————————————
# eapo_search: uses search_and_summary.cpp + modules
# ——————————————————————————————————————————————
add_executable(eapo_search
  src/search_and_summary.cpp
  ${ALL_SRCS}
)

target_link_libraries(eapo_search PRIVATE
  ${TORCH_LIBRARIES}
  nlohmann_json::nlohmann_json
  Boost::program_options
  $<$<BOOL:${USE_NVML}>:${NVML_LIBRARY}>
  $<$<BOOL:${USE_SENTENCEPIECE}>:${SP_LIBRARY}>
  $<$<BOOL:${USE_TOKENIZERS}>:tokenizers::tokenizers>
)

# ——————————————————————————————————————————————
# Summary of build
# ——————————————————————————————————————————————
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "LibTorch at: ${TORCH_INSTALL_PREFIX}")
message(STATUS "Torch version: ${Torch_VERSION}")
