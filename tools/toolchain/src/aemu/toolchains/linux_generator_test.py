# -*- coding: utf-8 -*-
# Copyright 2025 - The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import unittest
import tempfile
import shutil
from pathlib import Path
from unittest.mock import MagicMock, patch
from aemu.toolchains.linux_generator import LinuxToLinuxGenerator


class TestLinuxToLinuxGenerator(unittest.TestCase):
    def setUp(self):
        self.test_dir = tempfile.mkdtemp()
        self.aosp = Path("/aosp")
        self.dest = Path(self.test_dir) / "dest"
        self.prefix = "test-"
        self.versions = {"clang": "clang-test", "rust": "1.78.0"}

        # Mock Bazel
        self.mock_bazel = MagicMock()
        self.mock_bazel.info = {
            "output_base": "/ob",
            "output_path": "/op",
            "workspace": "/ws",
            "bazel-bin": "/bb",
        }

        with patch("aemu.toolchains.toolchain_generator.CommandLineReconstructor"):
            with patch(
                "aemu.toolchains.toolchain_generator.check_output",
                return_value="clang version 18.0.0",
            ):
                with patch(
                    "aemu.toolchains.toolchain_generator.ToolchainGenerator.cc_version",
                    return_value="18",
                ):
                    self.gen = LinuxToLinuxGenerator(
                        self.aosp, self.dest, self.prefix, self.versions
                    )
                    self.gen.bazel = self.mock_bazel
                    self.gen.cflags = "-some-cflags"
                    self.gen.with_compat = False
                    self.gen.ccache = None

    def tearDown(self):
        shutil.rmtree(self.test_dir)

    @patch("aemu.toolchains.linux_generator.LinuxToLinuxGenerator.initialize")
    def test_tool_methods(self, mock_init):
        clang_bin = (
            self.aosp
            / "prebuilts"
            / "clang"
            / "host"
            / "linux-x86"
            / "clang-test"
            / "bin"
        )

        self.assertEqual(self.gen.nm()[0], f'"{clang_bin / "llvm-nm"}"')
        self.assertEqual(self.gen.ar()[0], f'"{clang_bin / "llvm-ar"}"')

        cc_cmd, _ = self.gen.cc()
        self.assertIn(str(clang_bin / "clang"), cc_cmd)
        self.assertIn("-some-cflags", cc_cmd)

    @patch("aemu.toolchains.linux_generator.LinuxToLinuxGenerator.gen_script")
    @patch(
        "aemu.toolchains.linux_generator.LinuxToLinuxGenerator.write_toolchain_config"
    )
    @patch("aemu.toolchains.linux_generator.LinuxToLinuxGenerator.link_dirs")
    @patch(
        "aemu.toolchains.linux_generator.LinuxToLinuxGenerator.generate_pkg_config_files"
    )
    @patch("aemu.toolchains.linux_generator.LinuxToLinuxGenerator.initialize")
    def test_gen_toolchain(
        self, mock_init, mock_gen_pc, mock_link, mock_write, mock_gen_script
    ):
        self.mock_bazel.build_target.return_value = []
        self.gen.gen_toolchain([], {})

        calls = [c[0][0] for c in mock_gen_script.call_args_list]
        self.assertIn("cc", calls)
        self.assertIn("c++", calls)
        self.assertIn("objc", calls)
        self.assertIn("ld.lld", calls)
        self.assertIn("tidy", calls)


if __name__ == "__main__":
    unittest.main()
