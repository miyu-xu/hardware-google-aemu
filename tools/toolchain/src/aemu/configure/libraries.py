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
import shutil
from pathlib import Path
from typing import Dict, List, Tuple, Any

from aemu.toolchains.package_config_pc import PackageConfigPc


# Base class for shared properties
class Lib:
    """A base class for representing a library dependency."""

    def __init__(self, builder: Any, target: str, version: str, shim: Dict[str, str]) -> None:
        """Initializes a Lib object.

        Args:
            builder: The build system to use for building the library.
            target: The build target for the library.
            version: The version of the library.
            shim: A dictionary of shims to apply.
        """
        self.builder = builder
        self.version = version
        self.shim = shim
        self.target = target

    def get_library_config(
        self, builder: Any, bazel_target: str, shim: Dict[str, str]
    ) -> Tuple[List[Path], Path]:
        """Get the public includes and archive path exported by the given Bazel target.

        Args:
            builder: The builder object.
            bazel_target: The Bazel target to query.
            shim: Additional shims we wish to apply.

        Returns:
            A tuple containing a list of include paths and the archive path.
        """
        if "archive" in shim:
            archive = Path(shim["archive"])
        else:
            if "archive_target" in shim:
                archive = builder.get_archive(shim["archive_target"])
            else:
                archive = builder.get_archive(bazel_target)

        if "includes" in shim:
            # Apply shims, vs. what bazel reports.
            libdir = str(archive.parent)
            includes = list(
                set([Path(x.replace("${libdir}", libdir)) for x in shim["includes"]])
            )
        else:
            if "include_target" in shim:
                includes = builder.get_includes(shim["include_target"])
            else:
                includes = builder.get_includes(bazel_target)

        if "include_suffix" in shim:
            includes = [i / Path(shim["include_suffix"]) for i in includes]

        return includes, archive

    def generate_pkg_config(self, dest: Path, pkg_config_dir: Path) -> None:
        """Generate a pkgconfig .pc file for the given Bazel target.

        This method registers the library with a provided version and applies
        specified shims if needed. This includes building the target, retrieving
        the archive, obtaining library includes, and creating the pkg-config
        discovery file.

        Args:
            dest: The destination directory for the release.
            pkg_config_dir: The directory to write the .pc file to.
        """
        # Build the specified Bazel target.
        builder = self.builder

        # Retrieve the information associated with the target.
        includes, archive = self.get_library_config(builder, self.target, self.shim)

        # Name is @module//:<name>, i.e. the thing after :
        pkglib_name = self.target[self.target.rfind(":") + 1 :]

        cfg = PackageConfigPc(
            name=pkglib_name,
            version=self.version,
            release_dir=dest / "release",
            archive=archive,
            includes=includes,
            shim=self.shim,
            target=self.target,
        )

        cfg.write(pkg_config_dir)
        if not cfg.is_static():
            cfg.binplace(dest)


class BazelLib(Lib):
    """A library dependency that is built with Bazel."""

    def __init__(self, builder: Any, target: str, version: str, shim: Dict[str, str]) -> None:
        """Initializes a BazelLib object.

        Args:
            builder: The Bazel build system to use.
            target: The Bazel target for the library.
            version: The version of the library.
            shim: A dictionary of shims to apply.
        """
        super().__init__(builder, target, version, shim)


class CMakeLib(Lib):
    """A library dependency that is built with CMake."""

    def __init__(self, builder: Any, target: str, version: str, shim: Dict[str, str]) -> None:
        """Initializes a CMakeLib object.

        Args:
            builder: The CMake build system to use.
            target: The CMake target for the library.
            version: The version of the library.
            shim: A dictionary of shims to apply.
        """
        super().__init__(builder, target, version, shim)

    def generate_pkg_config(self, dest: Path, pkg_config_dir: Path) -> None:
        """Generate a pkgconfig .pc file for the given CMake target.

        Args:
            dest: The destination directory for the release.
            pkg_config_dir: The directory to write the .pc file to.
        """
        builder = self.builder
        output = builder.build_target(self.target)
        pkglib_name = self.target[self.target.rfind(":") + 1 :]

        cfg = PackageConfigPc(
            name=pkglib_name,
            version=self.version,
            release_dir=dest / "release",
            archive=output
            / self.shim.get("archive", "unknown"),  # For now we will use shims.
            includes=None,
            shim=self.shim,
            target=self.target,
        )

        cfg.write(pkg_config_dir)
        if not cfg.is_static():
            cfg.binplace(dest)

        pc_file = Path(output) / self.shim.get("pc", "")
        if pc_file.is_file() and pc_file.exists():
            shutil.copyfile(pc_file, pkg_config_dir / pc_file.name)
