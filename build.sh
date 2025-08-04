#!/usr/bin/env bash

# Configuration (override via env vars if needed):
# LIBTORCH_DIR: path to your LibTorch install (default: ~/libs/libtorch)
# BUILD_DIR: build directory (default: build)
# USE_NVML: ON/OFF (default: ON)
# USE_SENTENCEPIECE: ON/OFF (default: ON)
# USE_TOKENIZERS: ON/OFF (default: OFF)

LIBTORCH_DIR="${LIBTORCH_DIR:-$HOME/libs/libtorch}"
BUILD_DIR="${BUILD_DIR:-build}"
USE_NVML="${USE_NVML:-ON}"
USE_SENTENCEPIECE="${USE_SENTENCEPIECE:-ON}"
USE_TOKENIZERS="${USE_TOKENIZERS:-OFF}"

echo "=== EAPO_Cpp Build Script ==="
echo "LibTorch:       $LIBTORCH_DIR"
echo "Build dir:      $BUILD_DIR"
echo "Use NVML:       $USE_NVML"
echo "Use SentencePiece: $USE_SENTENCEPIECE"
echo "Use Tokenizers:   $USE_TOKENIZERS"

BUILD_DIR=./build

# Prepare build directory
echo "\n-- Configuring and building EAPO_Cpp..."
set -e

mkdir -p build
cd build
cmake .. \
  -DTorch_DIR="$LIBTORCH_DIR/share/cmake/Torch" \
  -DUSE_NVML=$USE_NVML \
  -DUSE_SENTENCEPIECE=$USE_SENTENCEPIECE \
  -DUSE_TOKENIZERS=$USE_TOKENIZERS \
  -DCMAKE_BUILD_TYPE=Release

echo "\n=== Build complete! ==="
echo "Executable: $BUILD_DIR/eapo_cpp"
