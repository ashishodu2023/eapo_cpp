cmake_minimum_required(VERSION 3.18)
project(EAPO_Cpp LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Prefer config-mode packages over module-mode
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

# —————————————————————————————
# LibTorch (PyTorch C++ API)
# —————————————————————————————
if(NOT DEFINED Torch_DIR)
  message(FATAL_ERROR
    "You must pass -DTorch_DIR to the directory containing\n"
    "  TorchConfig.cmake\n"
    "e.g. -DTorch_DIR=/opt/libtorch/share/cmake/Torch"
  )
endif()
list(APPEND CMAKE_PREFIX_PATH "${Torch_DIR}")
find_package(Torch REQUIRED CONFIG)

# —————————————————————————————
# JSON (nlohmann) and Boost
# —————————————————————————————
find_package(nlohmann_json REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)

# —————————————————————————————
# Optional: NVML (energy sampling)
# —————————————————————————————
option(USE_NVML "Enable NVML energy sampling" ON)
if(USE_NVML)
  find_path(NVML_INCLUDE_DIR
    NAMES nvml.h
    HINTS ${CUDA_TOOLKIT_ROOT_DIR}/include /usr/include
  )
  find_library(NVML_LIBRARY
    NAMES nvidia-ml nvml
    HINTS ${CUDA_TOOLKIT_ROOT_DIR}/lib64 /usr/lib/x86_64-linux-gnu
  )
  if(NVML_INCLUDE_DIR AND NVML_LIBRARY)
    message(STATUS "NVML support: ON  (found ${NVML_LIBRARY})")
    add_definitions(-DUSE_NVML)
  else()
    message(WARNING "NVML not found; disabling NVML support")
    set(USE_NVML OFF)
  endif()
endif()

# —————————————————————————————
# Optional: SentencePiece tokenizer
# —————————————————————————————
option(USE_SENTENCEPIECE "Enable SentencePiece tokenizer" ON)
if(USE_SENTENCEPIECE)
  find_path(SP_INCLUDE_DIR
    NAMES sentencepiece_processor.h
    HINTS /usr/local/include /usr/include
  )
  find_library(SP_LIBRARY
    NAMES sentencepiece
    HINTS /usr/local/lib /usr/lib/x86_64-linux-gnu
  )
  if(SP_INCLUDE_DIR AND SP_LIBRARY)
    message(STATUS "SentencePiece support: ON  (found ${SP_LIBRARY})")
    add_definitions(-DUSE_SENTENCEPIECE)
  else()
    message(WARNING "SentencePiece not found; disabling SPM support")
    set(USE_SENTENCEPIECE OFF)
  endif()
endif()

# —————————————————————————————
# Optional: HuggingFace Tokenizers
# —————————————————————————————
option(USE_TOKENIZERS "Enable HuggingFace Tokenizers support" OFF)
if(USE_TOKENIZERS)
  if(DEFINED tokenizers_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${tokenizers_DIR}")
  endif()
  find_package(tokenizers CONFIG QUIET)
  if(tokenizers_FOUND)
    message(STATUS "Tokenizers support: ON (found tokenizers)")
    add_definitions(-DUSE_TOKENIZERS)
  else()
    message(WARNING "tokenizers not found; disabling tokenizers support")
    set(USE_TOKENIZERS OFF)
  endif()
endif()

# —————————————————————————————
# Gather source files (auto-reconfigure on new .cpp)
# —————————————————————————————
file(GLOB CONFIGURE_DEPENDS SRC_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# —————————————————————————————
# Build executable
# —————————————————————————————
file(GLOB SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(eapo_cpp ${SRC_FILES})

# —————————————————————————————
# Link libraries
# —————————————————————————————
target_link_libraries(eapo_cpp
  PRIVATE
    ${TORCH_LIBRARIES}
    nlohmann_json::nlohmann_json
    Boost::program_options
)

if(USE_NVML)
  target_include_directories(eapo_cpp PRIVATE ${NVML_INCLUDE_DIR})
  target_link_libraries(eapo_cpp PRIVATE ${NVML_LIBRARY})
endif()

if(USE_SENTENCEPIECE)
  target_include_directories(eapo_cpp PRIVATE ${SP_INCLUDE_DIR})
  target_link_libraries(eapo_cpp PRIVATE ${SP_LIBRARY})
endif()

if(USE_TOKENIZERS)
  target_link_libraries(eapo_cpp PRIVATE tokenizers::tokenizers)
endif()

# —————————————————————————————
# Runtime RPATH for LibTorch
# —————————————————————————————
set_target_properties(eapo_cpp PROPERTIES
  BUILD_WITH_INSTALL_RPATH TRUE
  INSTALL_RPATH "$ORIGIN/../libtorch/lib"
)

# —————————————————————————————
# Configuration summary
# —————————————————————————————
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "LibTorch found at: ${TORCH_INSTALL_PREFIX}")
message(STATUS "Torch version: ${Torch_VERSION}")
