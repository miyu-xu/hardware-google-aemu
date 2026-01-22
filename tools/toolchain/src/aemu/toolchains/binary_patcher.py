# -*- coding: utf-8 -*-
# Copyright 2023 - The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the',  help='License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an',  help='AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import logging
import re
from pathlib import Path

from aemu.process.runner import check_output, run


class BinaryPatcher:
    """Responsible for patching binaries."""

    @staticmethod
    def patch_dylib(dylib: Path) -> None:
        """Patches a dylib to work around b/331243894.

        Args:
            dylib: The path to the dylib.
        """
        # Workaround for b/331243894
        rpath_regex = re.compile(r"^.*@rpath\/([^\s]+)")
        result = check_output(["otool", "-L", str(dylib)])
        rpathline = result.splitlines()[1]
        match = rpath_regex.match(rpathline)
        if match:
            bazel_name = dylib.parent / match.group(1)
            if not bazel_name.exists():
                bazel_name.symlink_to(dylib)
            logging.info("Patching up bazel @path %s -> %s", bazel_name, dylib)
        else:
            logging.info("Not patching %s", dylib)

    @staticmethod
    def patch_solib(solib: Path) -> None:
        """Patches a shared library to set up the SONAME.

        Args:
            solib: The path to the shared library.
        """
        # Run the objdump command to extract SONAME
        objdump_output = run(["objdump", "-p", str(solib)])

        # Find the line containing SONAME
        soname_line = next((line for line in objdump_output if "SONAME" in line), None)

        if soname_line:
            # Extract the SONAME file that the library is using and setup the symlink
            soname_string = soname_line.split()[1]
            if soname_string != solib.name:
                soname = solib.parent / soname_string
                target_relative = solib.relative_to(soname.parent)
                if soname.exists():
                    soname.unlink()
                soname.symlink_to(target_relative)
