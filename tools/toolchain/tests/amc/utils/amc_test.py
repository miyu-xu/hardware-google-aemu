# Copyright 2025 - The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-20.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import argparse
import os
import shutil
import subprocess
import sys
import zipfile

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--amc", required=True)
    parser.add_argument("--build-config", required=True)
    parser.add_argument("--shim", required=True)
    args = parser.parse_args()

    amc_script = os.path.abspath(args.amc)
    config_file = os.path.abspath(args.build_config)
    shim_file = os.path.abspath(args.shim)
    output_dir = "test_output"
    
    os.makedirs(output_dir, exist_ok=True)

    # Change to the workspace root before running amc
    workspace_root = os.path.join(os.environ["TEST_SRCDIR"], os.environ["TEST_WORKSPACE"])
    os.chdir(workspace_root)

    cmd = [
        os.path.relpath(amc_script, start=workspace_root),
        "-v",
        "--bazel_build_options=--sandbox_default_allow_network=true",
        "bazel",
        "--config",
        os.path.relpath(config_file, start=workspace_root),
        "--shim",
        os.path.relpath(shim_file, start=workspace_root),
        output_dir,
    ]

    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running amc.py: {e}")
        sys.exit(1)

    # Verify that the zip file was created
    zip_files = [f for f in os.listdir(output_dir) if f.endswith(".zip")]
    if not zip_files:
        print("No zip file found in output directory.")
        sys.exit(1)

    # Unzip the file and check for BUILD.linux-x86_64
    with zipfile.ZipFile(os.path.join(output_dir, zip_files[0]), "r") as zip_ref:
        zip_ref.extractall(output_dir)

    build_file_name = "BUILD.linux-x86_64"
    build_file_found = False
    for root, _, files in os.walk(output_dir):
        if build_file_name in files:
            build_file_found = True
            break
    
    if not build_file_found:
        print(f"{build_file_name} not found in zip file.")
        sys.exit(1)

    print("Test passed!")
    shutil.rmtree(output_dir)

if __name__ == "__main__":
    main()
