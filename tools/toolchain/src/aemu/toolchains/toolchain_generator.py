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
import json
import logging
import platform
import re
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Tuple, List, Dict, Any, Callable

from aemu.process.bazel import Bazel


class ToolchainGenerator:
    """A class for generating toolchain wrappers."""

    PKGCFG_DIR = "pc-config"

    def __init__(self, aosp: Path, dest: Path, prefix: str) -> None:
        """Initializes a ToolchainGenerator object.

        Args:
            aosp: The path to the AOSP source tree.
            dest: The destination directory for the toolchain.
            prefix: The prefix for the toolchain binaries.
        """
        self.aosp = aosp.absolute()
        # self.bazel = Bazel(self.aosp, dest)
        # This needs to be set externally now.
        self.bazel: Bazel = None

        toolchain_json = (
            self.aosp / "build" / "bazel" / "toolchains" / "tool_versions.json"
        )
        with open(
            toolchain_json,
            encoding="utf-8",
        ) as f:
            self.versions = json.load(f)
        self.dest: Path = dest
        self.prefix = prefix
        self.toolchain_map: Dict[str, str] = {}
        self.dest.mkdir(exist_ok=True, parents=True)
        self.ccache = shutil.which("sccache") or shutil.which("ccache")
        self.py_exe = Path(sys.executable).absolute()

        # Create pkgconfig directory.
        self.pkgconfig_directory = self.dest / ToolchainGenerator.PKGCFG_DIR
        self.pkgconfig_directory.mkdir(parents=True, exist_ok=True)

    def version(self) -> str:
        """Returns the clang version."""
        return self.versions.get("clang", "clang-stable")

    def rust_version(self) -> str:
        """Returns the rust version."""
        return self.versions.get("rust", "1.78.0")

    def cc_version(self) -> str:
        """Returns the C compiler version."""
        logging.info("Using clang: %s", self.clang())
        result = subprocess.check_output(
            [str(self.clang() / "bin" / "clang"), "-v"],
            encoding="utf-8",
            stderr=subprocess.STDOUT,
        )
        version = result.splitlines()[0]
        match = re.match(r".*clang version (\d+).(\d+).(\d).*", version)
        if match:
            return match.group(1)

        raise ValueError(f"Could not find version string in {version}")

    def clang(self) -> Path:
        """Returns the path to the clang directory."""
        return (
            self.aosp
            / "prebuilts"
            / "clang"
            / "host"
            / f"{self.host()}-x86"
            / self.version()
        )

    def cmake(self) -> Tuple[str, str]:
        """Returns the cmake command and extra arguments."""
        cmake = (
            self.aosp / "prebuilts" / "cmake" / f"{self.host()}-x86" / "bin" / "cmake"
        )
        return (
            f"{cmake} ",
            "",
        )

    def host(self) -> str:
        """Returns the host operating system."""
        return platform.system().lower()

    def nm(self) -> Tuple[Path, str]:
        """Returns the nm command and extra arguments."""
        return self.clang() / "bin" / "llvm-nm", ""

    def ar(self) -> Tuple[Path, str]:
        """Returns the ar command and extra arguments."""
        return self.clang() / "bin" / "llvm-ar", ""

    def objdump(self) -> Tuple[Path, str]:
        """Returns the objdump command and extra arguments."""
        return self.clang() / "bin" / "llvm-objdump", ""

    def strings(self) -> Tuple[Path, str]:
        """Returns the strings command and extra arguments."""
        return self.clang() / "bin" / "llvm-strings", ""

    def ranlib(self) -> Tuple[Path, str]:
        """Returns the ranlib command and extra arguments."""
        return self.clang() / "bin" / "llvm-ranlib", ""

    def cxx(self) -> Tuple[Path, str]:
        """Returns the C++ compiler command and extra arguments."""
        return self.clang() / "bin" / "clang++", ""

    def cc(self) -> Tuple[Path, str]:
        """Returns the C compiler command and extra arguments."""
        return self.clang() / "bin" / "clang", ""

    def lld(self) -> Tuple[Path, str]:
        """Returns the lld command and extra arguments."""
        return self.clang() / "bin" / "lld", ""

    def ninja(self) -> Tuple[Path, str]:
        """Returns the ninja command and extra arguments."""
        return (
            self.aosp
            / "prebuilts"
            / "build-tools"
            / f"{self.host()}-x86"
            / "bin"
            / "ninja",
            "",
        )

    def rust_flags(self) -> List[str]:
        """Returns the rust flags."""
        return []

    def rustc(self) -> Tuple[Path, str]:
        """Returns the rustc command and extra arguments."""
        return (
            self.aosp
            / "prebuilts"
            / "rust"
            / f"{self.host()}-x86"
            / self.rust_version()
            / "bin"
            / "rustc",
            "",
        )

    def cargo(self) -> Tuple[str, str]:
        """Returns the cargo command and extra arguments."""
        rustc_bin = (
            self.aosp
            / "prebuilts"
            / "rust"
            / f"{self.host()}-x86"
            / self.rust_version()
            / "bin"
            / "rustc"
        )
        cargo_bin = (
            self.aosp
            / "prebuilts"
            / "rust"
            / f"{self.host()}-x86"
            / self.rust_version()
            / "bin"
            / "cargo"
        )

        script = f"CARGO_BUILD_RUSTC={rustc_bin}\n" f"{cargo_bin}"
        return script, ""

    def meson(self) -> Tuple[str, str]:
        """Returns the meson command and extra arguments."""
        meson_py = self.aosp / "third_party" / "meson" / "meson.py"
        exe = f"{self.py_exe} {meson_py} "
        return exe, ""

    def strip(self) -> Tuple[str, str]:
        """Returns the strip command and extra arguments."""
        return "", ""

    def pkg_config(self) -> Tuple[str, str]:
        """Returns the pkg-config command and extra arguments."""
        # Build pkg-config from source for the host.
        self.bazel.build_target("@pkg-config", for_host=True)
        return (
            f'PKG_CONFIG_PATH={self.pkgconfig_directory}  PKG_CONFIG_LIBDIR="" '
            f"{self.bazel.info['bazel-bin']}/external/pkg-config+/pkg-config",
            "",
        )

    def gen_script(self, name: str, exe: Path, cmd_generator_fn: Callable[[], Tuple[Any, str]]) -> None:
        """Generates a script.

        Args:
            name: The name of the script.
            exe: The path to the script.
            cmd_generator_fn: A function that returns the command and extra arguments.
        """
        current_file = Path(__file__).resolve()
        self.toolchain_map[name] = exe.absolute().as_posix()

        logging.info("Generating %s", exe)

        rem = "#"
        run, extra = cmd_generator_fn()
        params = '"$@"'

        script = f"""#!/bin/sh
{rem} Auto-generated by {current_file}, DO NOT EDIT!!
{run} {params} {extra}
"""
        with open(exe, "w", encoding="utf-8") as f:
            f.write(script)

        exe.chmod(0o755)

    def _get_toolchain_config(self) -> str:
        """Returns the toolchain configuration."""
        result = f"# Auto generated by Android Meson Generator - do not modify\n"
        result += """[properties]
[built-in options]
c_args = []
cpp_args = []
objc_args = []
c_link_args = []
cpp_link_args = []
[binaries]
"""
        for name, dest in self.toolchain_map.items():
            result += f"{name} = '{dest}'\n"

        cc = self.toolchain_map["cc"]
        result += f"c = '{cc}'\n"
        return result

    def write_toolchain_config(self) -> None:
        """Writes a toolchain configuration that can be consumed by meson."""
        with open(self.dest / "aosp-cl.ini", "w", encoding="utf-8") as f:
            f.write(self._get_toolchain_config())

    def link_dirs(self) -> None:
        """Setup links to libc++.so etc.."""
        clang_lib = self.clang() / "lib"
        if not clang_lib.exists():
            logging.warning("The clang lib directory: %s, does not exist.", clang_lib)
            return

        (self.dest / "lib").symlink_to(clang_lib)

    def gen_toolchain(self, packages: List[Any] = [], binaries: Dict[str, str] = {}) -> None:
        """Generates the toolchain.

        Args:
            packages: A list of packages to generate pkg-config files for.
            binaries: A dictionary of binaries to generate wrappers for.
        """
        cmds = {
            "nm": self.nm,
            "ar": self.ar,
            "c++": self.cxx,
            "cc": self.cc,
            "clang": self.cc,
            "clang++": self.cxx,
            "g++": self.cxx,
            "gcc": self.cc,
            "lld": self.lld,
            "ld": self.lld,
            "objdump": self.objdump,
            "strings": self.strings,
            "ranlib": self.ranlib,
            "meson": self.meson,
            "ninja": self.ninja,
            "pkg-config": self.pkg_config,
            "strip": self.strip,
            "cmake": self.cmake,
            "cargo": self.cargo,
            "rustc": self.rustc,
        }
        for cmd, fn in cmds.items():
            self.gen_script(cmd, self.dest / f"{self.prefix}{cmd}", fn)

        for name, target in binaries.items():
            self._generate_bazel_binary_wrapper(name, target)

        self.generate_pkg_config_files(packages)
        self.link_dirs()
        self.write_toolchain_config()

    def _generate_bazel_binary_wrapper(self, name: str, target: str) -> None:
        """Generates a wrapper for a Bazel binary.

        Args:
            name: The name of the binary.
            target: The Bazel target.
        """
        exe = self.dest / f"{self.prefix}{name}"
        self.toolchain_map[name] = exe.absolute().as_posix()
        bazel_exe = self.bazel.build_exe(target)
        script = f'''#!/bin/sh
# Auto-generated by ToolchainGenerator, DO NOT EDIT!!
# Bazel: {target}
{bazel_exe} "$@"
'''
        with open(exe, "w", encoding="utf-8") as f:
            f.write(script)
        exe.chmod(0o755)

    def generate_pkg_config_files(self, packages: List[Any]) -> None:
        """Generates pkg-config files for all defined dependencies.

        Args:
            packages: A list of packages to generate pkg-config files for.
        """
        for package in packages:
            package.generate_pkg_config(
                self.dest,
                self.pkgconfig_directory,
            )

