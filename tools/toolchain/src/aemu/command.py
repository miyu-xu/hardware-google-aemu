# Copyright 2025 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""A module for reconstructing the command used to invoke a script."""
import os
import sys
import shlex
from typing import List


class CommandLineReconstructor:
    """A class for reconstructing the command used to invoke a script."""

    def get_command_parts(self) -> List[str]:
        """Reconstructs the command used to invoke the script as a list of parts.

        The --update flag is filtered out to prevent infinite loops.

        Returns:
            A list of strings representing the command and its arguments.
        """
        args = [arg for arg in sys.argv[1:] if arg != "--update"]
        if "BAZEL_SELF_LABEL" in os.environ:
            command = ["bazel", "run", os.environ["BAZEL_SELF_LABEL"]]
            if args:
                command += ["--"] + args
            return command

        # Reconstruct python command
        # sys.argv is ['script.py', 'arg1', ...]
        # We want ['/path/to/python', 'script.py', 'arg1', ...]
        filtered_argv = [sys.argv[0]] + args
        return [sys.executable] + filtered_argv

    def get_command_string(self) -> str:
        """Reconstructs the command used to invoke the script as a properly quoted string."""
        return " ".join(shlex.quote(p) for p in self.get_command_parts())

    def get_cwd(self) -> str:
        """Returns the current working directory.

        If running under bazel, this will be the workspace root.

        Returns:
            The path to the current working directory.
        """
        # BUILD_WORKSPACE_DIRECTORY is set by Bazel to the root of the workspace.
        return os.environ.get("BUILD_WORKSPACE_DIRECTORY", os.getcwd())
