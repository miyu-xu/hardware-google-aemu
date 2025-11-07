# Android Meson Configurator (amc.py) Tests

## Overview

This directory contains tests for `amc.py`, a tool used to convert Meson-based projects into Bazel-based projects within the Android Open Source Project (AOSP) environment. The primary goal of these tests is to verify the successful conversion and build process.

## Testing Workflow

Follow these steps to test the Meson to Bazel conversion and build process.

### Prerequisites

Ensure the `$AOSP_ROOT` environment variable is set to the root of your AOSP checkout.

### Steps

1.  **Generate the AMC Toolchain and Setup Meson**

    From the directory containing the `meson.build` file, run `amc.py` to generate the necessary toolchain and set up the Meson build directory (`out-amc`).

    ```bash
    python3 $AOSP_ROOT/hardware/google/aemu/tools/toolchain/src/amc.py -v setup --aosp $AOSP_ROOT --config build-config.jsonc out-amc
    ```

2.  **Compile the Project with Ninja**

    After the setup is complete, navigate to the generated toolchain directory and compile the project using Ninja.

    ```bash
    cd out-amc/toolchain && ./ninja -C ../build
    ```

This workflow allows you to validate that the Meson project can be correctly configured and built using the `amc.py`-generated toolchain and build files.