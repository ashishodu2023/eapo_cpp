#!/usr/bin/env bash

# =============================================================================
# EAPO_Cpp Build & Make Script
# =============================================================================

# Configuration (override via env vars if needed):
#   LIBTORCH_DIR:        path to your LibTorch install (default: ~/libs/libtorch)
#   BUILD_DIR:           build directory (default: build)
#   USE_NVML:            ON/OFF (default: ON)
#   USE_SENTENCEPIECE:   ON/OFF (default: ON)
#   USE_TOKENIZERS:      ON/OFF (default: OFF)
# =============================================================================

LIBTORCH_DIR="${LIBTORCH_DIR:-$HOME/libs/libtorch}"
BUILD_DIR="${BUILD_DIR:-build}"
USE_NVML="${USE_NVML:-ON}"
USE_SENTENCEPIECE="${USE_SENTENCEPIECE:-ON}"
USE_TOKENIZERS="${USE_TOKENIZERS:-OFF}"

echo "=== EAPO_Cpp Build & Make Script ==="
echo "  LibTorch:           $LIBTORCH_DIR"
echo "  Build Directory:    $BUILD_DIR"
echo "  Use NVML:           $USE_NVML"
echo "  Use SentencePiece:  $USE_SENTENCEPIECE"
echo "  Use Tokenizers:     $USE_TOKENIZERS"
echo

set -e

# Create and enter the build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake configuration
echo "-- Configuring with CMake..."
cmake .. \
  -DTorch_DIR="$LIBTORCH_DIR/share/cmake/Torch" \
  -DUSE_NVML=$USE_NVML \
  -DUSE_SENTENCEPIECE=$USE_SENTENCEPIECE \
  -DUSE_TOKENIZERS=$USE_TOKENIZERS \
  -DCMAKE_BUILD_TYPE=Release

# Build all targets
echo
echo "-- Building targets with make --"
make -j"$(nproc)"

echo
echo "=== Build complete! ==="
echo "Executables available in $(pwd):"
echo "  - eapo_cpp"
echo "  - eapo_search"
echo
echo "You can now run:"
echo "  ./eapo_cpp --mode evaluate --config ../path/to/config.json --prompt '{...}'"
echo "  ./eapo_search ../path/to/config.json"

