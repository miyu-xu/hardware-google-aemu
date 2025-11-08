# Android Meson Configurator (amc.py) Tests

## Overview

This directory contains tests for `amc.py`, a tool used to convert Meson-based projects into Bazel-based projects within the Android Open Source Project (AOSP) environment. The primary goal of these tests is to verify the successful conversion and build process.

## How to Add a New Test

Follow these steps to create a new test for the `amc.py` workflow.

### 1. Create a New Test Directory

Each test case should reside in its own dedicated folder. Ideally, the directory name should correspond to the Meson function or feature being tested (e.g., `custom_target/` for testing Meson's [**custom_target**](https://mesonbuild.com/Reference-manual_functions.html#custom_target) function). This keeps the test project and its configuration isolated. For example, to create a test named `my_new_test`, run:

```bash
mkdir my_new_test
```

### 2. Add a Meson Project and Configuration Files

Inside your new directory (`my_new_test`), create the necessary files for your Meson project. This typically includes:

-   **A `meson.build` file:** This is the core build script for Meson. It defines your project's sources, dependencies, and build targets.
-   **Source files:** (e.g., `main.cpp`) The code for your test application.
-   **A `build-config.jsonc` file:** This is the configuration file for `amc.py`. It tells `amc` which Bazel targets to use for dependencies, what Meson options to set, and which binaries to make available to the Meson build.
-   **A `shim.jsonc` file (Required):** If you plan to use `amc.py` to generate Bazel build files from your Meson project, this file can be used to apply necessary modifications (shims) to the generated files.

### 3. Verify the Meson Build

Before adding the test to the build system, verify that `amc.py` can successfully configure and build your new Meson project using the `test-amc-meson-project.sh` script. This script automates the setup and compilation steps, handling the creation of the toolchain, the Meson setup, and the final Ninja build.

**Usage:**

```bash
./test-amc-meson-project.sh --build-config <path_to_build_config> <meson_project_directory>
```

-   `--build-config`: Specifies the path to the `build-config.jsonc` file for the project.
-   `<meson_project_directory>`: The directory containing the `meson.build` file.

**Example:**

To test your new `my_new_test` project, run the following command from this directory:

```bash
./test-amc-meson-project.sh --build-config my_new_test/build-config.jsonc my_new_test
```

If the script completes successfully, it means your Meson project is correctly configured. The script will create an `out-amc` directory containing the build output and will print the absolute path to this directory upon completion.

### 4. Add the Test to `BUILD.bazel`

Finally, integrate your test into the automated test suite by adding it to the `tests/amc/BUILD.bazel` file. This allows the test to be run as part of the broader project build.

Open `tests/amc/BUILD.bazel` and add a new `amc_test` target. This rule, defined in `utils/amc_test.bzl`, wraps the test execution in a Bazel-compatible way.

For example, to add the `my_new_test` case, you would add the following:

```bzl
load("//hardware/google/aemu/tools/toolchain/tests/amc/utils:amc_test.bzl", "amc_test")

# ... existing tests ...

amc_test(
    name = "my_new_test",
    srcs = [
        "//hardware/google/aemu/tools/toolchain/tests/amc/my_new_test:main.cpp",
        "//hardware/google/aemu/tools/toolchain/tests/amc/my_new_test:meson.build",
    ],
    build_config = "//hardware/google/aemu/tools/toolchain/tests/amc/my_new_test:build-config.jsonc",
    shim = "//hardware/google/aemu/tools/toolchain/tests/amc/my_new_test:shim.jsonc",
)
```

Make sure to list all the necessary source and configuration files in the `srcs`, `build_config`, and `shim` attributes. The test will now run when you build the `//hardware/google/aemu/tools/toolchain/tests/amc:all` target.
