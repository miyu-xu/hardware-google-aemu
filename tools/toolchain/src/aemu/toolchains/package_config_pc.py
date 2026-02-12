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
import os
import re
import shutil
from pathlib import Path
from typing import Dict, Set, Optional, List

from aemu.process.runner import check_output, run
from aemu.toolchains.binary_patcher import BinaryPatcher
from aemu.util import safe_link, safe_link_tree


class PackageConfigPc:
    """
    PackageConfigPc - A class for generating pkg-config (.pc) files.

    This class facilitates the creation of pkg-config files, which are used to provide
    configuration information to build systems and compile/link applications.

    The following things can be shimmed:

    'Requires'    : Set of dependencies you want to specify
    'link_flags'  : Extra link_flags that should be used
    'link_name'   : Create symlink to the existing archive
    'name'        : Shim the name, vs. using the derived one.
    """

    def __init__(
        self,
        name: str,
        version: str,
        release_dir: Path,
        archives: List[Path],
        includes: Optional[Set[Path]],
        shim: Dict[str, str],
        target: str,
    ) -> None:
        """Initializes a PackageConfigPc object.

        Args:
            name: The name of the package.
            version: The version of the package.
            release_dir: The release directory.
            archives: A list of paths to archive files.
            includes: A set of include paths.
            shim: A dictionary of shims to apply.
            target: The bazel target.
        """
        self.name = shim.get("name", name)
        self.release_dir = release_dir.as_posix()
        self.version = version
        self.requires = shim.get("Requires", "")
        self.link_flags = shim.get("link_flags", "")
        self.extra_vars = shim.get("extra_vars", {})
        # Deduplicate archives by name, preferring .a
        deduped = {}
        for archive in archives:
            if not archive or not archive.name:
                continue
            lib_base = archive.with_suffix("").name
            if lib_base.startswith("lib"):
                lib_base = lib_base[3:]

            if lib_base not in deduped or archive.suffix == ".a":
                deduped[lib_base] = archive

        self.archives = list(deduped.values())
        self.target = target

        # Use the first archive's parent as the primary libdir for now
        self.libdir = self.archives[0].parent.as_posix() if self.archives else ""
        if not archive.name:
            if "Libs" in shim:
                self.libs = shim.get("Libs")
                self.lib = self.name
        else:
            libs_list = []
            libdirs = set()
            for archive in self.archives:
                lib_name = archive.with_suffix("").name
                if lib_name.startswith("lib"):
                    lib_name = lib_name[3:]

                libdirs.add(archive.parent.as_posix())
                libs_list.append(f"-l{lib_name}")

            l_flags = " ".join([f"-L{d}" for d in sorted(list(libdirs))])
            if self.libdir:
                l_flags = l_flags.replace(self.libdir, "${libdir}")
            self.libs = f"{l_flags} {' '.join(libs_list)} {self.link_flags}".strip()

        self.shim = shim

        self.all_includes = includes if includes else set()
        if includes is None:
            self.include_dir = ""
            self.cflags = ""
        else:
            self.include_dir = next(iter(includes)).as_posix()
            self.cflags = (
                " ".join([f"-I{path.as_posix()}" for path in includes if path.exists()])
                + f" {shim.get('cflags', '')}"
            )
            if self.include_dir:
                self.cflags = self.cflags.replace(self.include_dir, "${includedir}")

    def is_static(self) -> bool:
        """Returns True if the library is static, False otherwise."""
        return self.archives[0].suffix == ".a" if self.archives else True

    @property
    def extra_args(self) -> str:
        """Returns the extra variables in a format suitable for a .pc file."""
        return "\n".join([f"{k}={v}" for k, v in self.extra_vars.items()])

    def persist(self, packages_dir: Path, workspace: Optional[Path] = None) -> None:
        """
        Persist the dependency artifacts (archives and includes) into the toolchain's
        'packages' directory.

        Strategy:
        - If the artifact is likely ephemeral (in sandbox or bazel cache), we use hardlinks.
        - If the artifact is external/system, we use symlinks.
        """
        pkg_root = packages_dir / self.name
        pkg_root.mkdir(parents=True, exist_ok=True)

        def is_ephemeral(path: Path) -> bool:
            path_str = path.as_posix()
            # If it's in the workspace, it might be cleaned.
            if workspace:
                try:
                    path.relative_to(workspace)
                    return True
                except ValueError:
                    pass
            # If it's in the bazel cache/output-base, it's definitely ephemeral.
            if "/.cache/bazel/" in path_str or "/bazel-out/" in path_str:
                return True
            return False

        new_archives = []
        for archive in self.archives:
            if archive and archive.name and archive.exists():
                new_lib_dir = pkg_root / "lib"
                new_lib_dir.mkdir(parents=True, exist_ok=True)
                new_archive = new_lib_dir / archive.name

                if new_archive.exists() or new_archive.is_symlink():
                    new_archive.unlink()

                if is_ephemeral(archive):
                    logging.info(
                        "Hard-linking ephemeral archive: %s -> %s", archive, new_archive
                    )
                    safe_link(archive, new_archive)
                else:
                    logging.info(
                        "Symlinking external archive: %s -> %s", archive, new_archive
                    )
                    os.symlink(archive.absolute(), new_archive)

                new_archives.append(new_archive)
            else:
                logging.warning(
                    "Archive %s does not exist or has no name, skipping persist.",
                    archive,
                )
                new_archives.append(archive)

        # Deduplicate archives by name, preferring .a
        deduped = {}
        for archive in new_archives:
            if not archive or not archive.name:
                continue
            lib_base = archive.with_suffix("").name
            if lib_base.startswith("lib"):
                lib_base = lib_base[3:]

            if lib_base not in deduped or archive.suffix == ".a":
                deduped[lib_base] = archive

        self.archives = list(deduped.values())
        if self.archives:
            self.libdir = self.archives[0].parent.as_posix()

        # Update self.libs since it might have contained the old paths
        if "Libs" not in self.shim:
            libs_list = []
            libdirs = set()
            for archive in self.archives:
                lib_name = archive.with_suffix("").name
                if lib_name.startswith("lib"):
                    lib_name = lib_name[3:]

                libdirs.add(archive.parent.as_posix())
                libs_list.append(f"-l{lib_name}")

            l_flags = " ".join([f"-L{d}" for d in sorted(list(libdirs))])
            if self.libdir:
                l_flags = l_flags.replace(self.libdir, "${libdir}")
            self.libs = f"{l_flags} {' '.join(libs_list)} {self.link_flags}".strip()

        if self.all_includes:
            new_include_root = pkg_root / "include"
            new_include_root.mkdir(parents=True, exist_ok=True)

            new_include_paths = []
            for idx, include in enumerate(sorted(list(self.all_includes))):
                if not include.exists():
                    logging.debug(
                        "Include path %s does not exist, skipping persist.", include
                    )
                    continue

                target_include_dir = new_include_root / str(idx)
                if target_include_dir.exists() or target_include_dir.is_symlink():
                    if (
                        target_include_dir.is_dir()
                        and not target_include_dir.is_symlink()
                    ):
                        shutil.rmtree(target_include_dir)
                    else:
                        target_include_dir.unlink()

                if is_ephemeral(include):
                    safe_link_tree(include, target_include_dir)
                else:
                    os.symlink(include.absolute(), target_include_dir)

                new_include_paths.append(target_include_dir)

            if new_include_paths:
                self.include_dir = new_include_paths[0].as_posix()
                self.cflags = (
                    " ".join([f"-I{path.as_posix()}" for path in new_include_paths])
                    + f" {self.shim.get('cflags', '')}"
                )
                if self.include_dir:
                    self.cflags = self.cflags.replace(self.include_dir, "${includedir}")

    def _template(self) -> str:
        """Returns the template for the .pc file."""
        amc_bin_target = self.archives[0].absolute().as_posix() if self.archives else ""
        return f"""prefix={self.release_dir}
includedir={self.include_dir}
libdir={self.libdir}
bindir={{prefix}}/bin
{self.extra_args}
amc_bazel_target={self.target}
amc_bin_target={amc_bin_target}

Name: {self.name}
Description: Auto generated by Android Meson Generator
Version: {self.version}

Requires: {self.requires}
Cflags: {self.cflags}
Libs: {self.libs}
"""

    def _shim_link(self, archive: Path, shim: Dict[str, str]) -> None:
        """
        Create a symbolic link to the library if specified in the shim.

        Args:
            archive (Path): The path to the library archive.
            shim (Dict): The shim configuration.

        Returns:
            None
        """
        if "link_name" in shim and archive.name != shim.get("link_name"):
            logging.info("Shimming link to: %s (%s)", archive, archive.exists())
            shim_link = archive.parent / shim.get("link_name")
            if not shim_link.exists():
                archive.link_to(shim_link)

    def binplace(self, dest_dir: Path) -> None:
        """Binplace the shared libraries to the given location."""
        so_ext = [".so", ".dylib", ".dll"]
        for archive in self.archives:
            if not archive or not archive.name:
                continue

            lib_base_name = archive.with_suffix("").name
            if lib_base_name.startswith("lib"):
                lib_name_no_prefix = lib_base_name[3:]
            else:
                lib_name_no_prefix = lib_base_name

            for ext in so_ext:
                for lib in [
                    lib_base_name,
                    lib_name_no_prefix,
                    self.name,
                ]:
                    possible = archive.parent / f"{lib}{ext}"
                    if possible.exists():
                        logging.debug("Binplacing: %s -> %s", possible, dest_dir)
                        destination = dest_dir / f"{lib}{self.shim.get('dll_ext', ext)}"

                        # We must unlink the destination first to avoid PermissionError
                        # if the destination is a read-only file (e.g. a hard link
                        # from the bazel cache).
                        if destination.exists() or destination.is_symlink():
                            destination.unlink()
                        shutil.copy2(possible, destination)
                        if ext == ".dylib":
                            # Patch up bazel @rpath
                            BinaryPatcher.patch_dylib(destination)
                        if ext == ".so":
                            # Patch up links if needed
                            BinaryPatcher.patch_solib(destination)
                        break

    def write(self, dest_dir: Path) -> None:
        """
        Write the generated pkg-config file to the specified destination directory.

        Args:
            dest_dir (Path): The destination directory.

        Returns:
            None
        """
        dest_dir.mkdir(exist_ok=True)
        location = dest_dir / f"{self.name}.pc"

        # Write the pkg-config file.
        logging.info("Generating %s", location)
        logging.debug(">>> %s", self._template().replace("\n", "\n>>>"))
        with open(location, "w", encoding="utf-8") as f:
            f.write(self._template())
