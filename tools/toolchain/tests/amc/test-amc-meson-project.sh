#!/bin/bash

set -e # Exit on error

BUILD_CONFIG_FILE=""
MESON_PROJECT_DIR=""

while [[ "$#" -gt 0 ]]; do
    case "$1" in
        --build-config)
            if [ -z "$2" ]; then
                echo "Error: --build-config requires an argument."
                exit 1
            fi
            BUILD_CONFIG_FILE="$2"
            shift 2
            ;;
        *)
            if [ -z "$MESON_PROJECT_DIR" ]; then
                MESON_PROJECT_DIR="$1"
            else
                echo "Error: Unknown option or too many arguments: $1"
                echo "Usage: $0 --build-config <config-file> <meson_project_directory>"
                exit 1
            fi
            shift
            ;;
    esac
done

if [ -z "$BUILD_CONFIG_FILE" ] || [ -z "$MESON_PROJECT_DIR" ]; then
  echo "Usage: $0 --build-config <config-file> <meson_project_directory>"
  exit 1
fi

if [ ! -f "$BUILD_CONFIG_FILE" ]; then
  echo "Error: Build config file '$BUILD_CONFIG_FILE' not found."
  exit 1
fi
BUILD_CONFIG_FILE="$(readlink -f "$BUILD_CONFIG_FILE")"

if [ ! -d "$MESON_PROJECT_DIR" ]; then
  echo "Error: Directory '$MESON_PROJECT_DIR' not found."
  exit 1
fi

# Set AOSP_ROOT by navigating up 7 directories from the script's location
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
AOSP_ROOT="$(readlink -f "$SCRIPT_DIR/../../../../../../..")"

# Set output directory relative to the script directory
OUTPUT_DIR="$SCRIPT_DIR/out-amc"

if [ ! -d "$AOSP_ROOT" ]; then
  echo "Error: AOSP_ROOT directory not found at $AOSP_ROOT"
  exit 1
fi

# Absolute path to amc.py
AMC_PY_PATH="$AOSP_ROOT/hardware/google/aemu/tools/toolchain/src/amc.py"

if [ ! -f "$AMC_PY_PATH" ]; then
    echo "Error: amc.py not found at $AMC_PY_PATH"
    exit 1
fi


# Step 1: Generate the AMC Toolchain and Setup Meson
echo "--- Cleaning up output directory ---"
if [ -d "$OUTPUT_DIR" ]; then
    echo "Removing existing output directory: $OUTPUT_DIR"
    rm -rf "$OUTPUT_DIR"
fi

echo "--- Running amc.py setup ---"
cd "$MESON_PROJECT_DIR"
python3 "$AMC_PY_PATH" -v setup --aosp "$AOSP_ROOT" --config "$BUILD_CONFIG_FILE" "$OUTPUT_DIR"

echo "--- Verifying amc.py setup output ---"
if [ ! -d "$OUTPUT_DIR/build" ]; then
  echo "Error: '$OUTPUT_DIR/build' directory not found after setup."
  exit 1
fi

if [ ! -d "$OUTPUT_DIR/toolchain" ]; then
  echo "Error: '$OUTPUT_DIR/toolchain' directory not found after setup."
  exit 1
fi
echo "--- Verification successful ---"

# Step 2: Compile the Project with Ninja
echo "--- Compiling with ninja ---"
cd "$OUTPUT_DIR/toolchain"
./ninja -C ../build

echo "--- Build complete ---"
echo "Output directory: $OUTPUT_DIR"
