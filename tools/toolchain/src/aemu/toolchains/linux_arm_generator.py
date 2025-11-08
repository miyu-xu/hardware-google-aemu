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

from aemu.toolchains.toolchain_generator import ToolchainGenerator
from pathlib import Path
import shutil
from typing import Tuple, Dict


class LinuxToLinuxAarch64Generator(ToolchainGenerator):
    """A cross compilation toolchain configurator using clang

    It will try to use the hardcoded version if available, otherwise
    it will fallback to the default one.
    """

    GCC_VER = 10
    GCC_PREFIX = "aarch64-linux-gnu-"

    def __init__(
        self, aosp: Path, dest: Path, prefix: str, versions: Dict[str, str] = None
    ) -> None:
        """Initializes a LinuxToLinuxAarch64Generator object.

        Args:
            aosp: The path to the AOSP source tree.
            dest: The destination directory for the toolchain.
            prefix: The prefix for the toolchain binaries.
        """
        super().__init__(aosp, dest, prefix, versions)
        self.target_arch = "aarch64"

    def _fetch_toolchain(self) -> Path:
        """Fetches the toolchain and sysroot.

        Returns:
            The path to the sysroot.
        """
        # Make sure we have the sysroot.
        sysroot = Path(self.dest) / "sysroot"
        if not sysroot.exists():
            # This will pull down the sysroot if not available.
            self.bazel.build_target("@aemu//tools/toolchain:hello")
            sysroot.mkdir(exist_ok=True, parents=True)
            src = Path(self.bazel.info["output_base"], "external", "arm-sysroot+")
            shutil.copytree(src, sysroot, dirs_exist_ok=True)

        return sysroot

    def cc(self) -> Tuple[str, str]:
        """Generates the script for the C compiler."""
        cache = f"{self.ccache}" if self.ccache else ""
        toolchain = self._fetch_toolchain()
        sysroot = toolchain / "aarch64-none-linux-gnu" / "libc"

        script = (
            f"{cache} {self.clang()}/bin/clang "
            f"--target=aarch64-none-linux-gnu --sysroot={sysroot} "
            f"--gcc-toolchain={toolchain} -fuse-ld=lld"
        )
        return script, ""

    def cxx(self) -> Tuple[str, str]:
        """Generates the script for the C++ compiler."""
        return self.cc()

    def rustc(self) -> Tuple[str, str]:
        """Gets the path to the rustc compiler."""
        return "echo <not yet implemented>", ""

    def cargo(self) -> Tuple[str, str]:
        """Gets the path to the cargo command."""
        return "echo <not yet implemented>", ""

    def strip(self) -> Tuple[str, str]:
        """Generates the script for the strip command."""
        objcopy = self.clang() / "bin" / "llvm-objcopy"
        script = "mkdir -p build/debug_info\n"
        script += "target=$(basename $1)\n"
        script += f'{objcopy} --only-keep-debug  $1 "build/debug_info/$target.debug" \n'
        script += f"{objcopy} --strip-unneeded  $1\n"
        script += f'{objcopy} --add-gnu-debuglink="build/debug_info/$target.debug" $1\n'
        script += "# EXPLICITLY DISABLED ARBITRARY ARGUMENTS: "
        return script, ""
