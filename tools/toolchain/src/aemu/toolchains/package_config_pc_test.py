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
import unittest
import os
import shutil
from pathlib import Path
from tempfile import TemporaryDirectory
from aemu.toolchains.package_config_pc import PackageConfigPc


class PackageConfigPcTest(unittest.TestCase):

    def test_persist_archive(self):
        with TemporaryDirectory() as tmp_dir:
            tmp_path = Path(tmp_dir)
            workspace = tmp_path / "workspace"
            workspace.mkdir()

            # Create a dummy archive outside the workspace (to ensure it gets persisted)
            outside = tmp_path / "outside"
            outside.mkdir()
            archive = outside / "libfoo.a"
            archive.write_text("dummy archive content")

            packages_dir = tmp_path / "packages"

            cfg = PackageConfigPc(
                name="foo",
                version="1.0",
                release_dir=tmp_path / "release",
                archives=[archive],
                includes=None,
                shim={},
                target="//foo:foo",
            )

            cfg.persist(packages_dir, workspace)

            # Check if archive was persisted
            persisted_archive = packages_dir / "foo" / "lib" / "libfoo.a"
            self.assertTrue(persisted_archive.exists())
            self.assertEqual(persisted_archive.read_text(), "dummy archive content")

            # Check if state was updated
            self.assertEqual(cfg.archives[0], persisted_archive)
            self.assertEqual(cfg.libdir, (packages_dir / "foo" / "lib").as_posix())
            self.assertIn("-lfoo", cfg.libs)
            self.assertIn("-L${libdir}", cfg.libs)

    def test_persist_includes(self):
        with TemporaryDirectory() as tmp_dir:
            tmp_path = Path(tmp_dir)
            workspace = tmp_path / "workspace"
            workspace.mkdir()

            # Create dummy includes outside the workspace
            outside = tmp_path / "outside"
            outside.mkdir()
            archive = outside / "libfoo.a"
            archive.write_text("dummy")
            inc1 = outside / "include1"
            inc1.mkdir()
            (inc1 / "foo.h").write_text("foo.h content")

            inc2 = outside / "include2"
            inc2.mkdir()
            (inc2 / "bar.h").write_text("bar.h content")

            packages_dir = tmp_path / "packages"

            cfg = PackageConfigPc(
                name="foo",
                version="1.0",
                release_dir=tmp_path / "release",
                archives=[archive],
                includes={inc1, inc2},
                shim={},
                target="//foo:foo",
            )

            cfg.persist(packages_dir, workspace)

            # Check if includes were persisted
            persisted_inc_root = packages_dir / "foo" / "include"
            self.assertTrue(
                (persisted_inc_root / "0" / "foo.h").exists()
                or (persisted_inc_root / "1" / "foo.h").exists()
            )
            self.assertTrue(
                (persisted_inc_root / "0" / "bar.h").exists()
                or (persisted_inc_root / "1" / "bar.h").exists()
            )

            # Check if cflags was updated
            self.assertIn("-I" + persisted_inc_root.as_posix(), cfg.cflags)

    def test_persist_inside_workspace_uses_hardlink(self):
        with TemporaryDirectory() as tmp_dir:
            tmp_path = Path(tmp_dir)
            workspace = tmp_path / "workspace"
            workspace.mkdir()

            # Create dummy archive inside the workspace
            archive = workspace / "libfoo.a"
            archive.write_text("dummy archive content")

            packages_dir = tmp_path / "packages"

            cfg = PackageConfigPc(
                name="foo",
                version="1.0",
                release_dir=tmp_path / "release",
                archives=[archive],
                includes=None,
                shim={},
                target="//foo:foo",
            )

            cfg.persist(packages_dir, workspace)

            # Check that archive WAS persisted (as a hardlink)
            persisted_archive = packages_dir / "foo" / "lib" / "libfoo.a"
            self.assertTrue(persisted_archive.exists())
            self.assertEqual(persisted_archive.read_text(), "dummy archive content")
            self.assertFalse(persisted_archive.is_symlink())
            self.assertEqual(cfg.archives[0], persisted_archive)


if __name__ == "__main__":
    unittest.main()
