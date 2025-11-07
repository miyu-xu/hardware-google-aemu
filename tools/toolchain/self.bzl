"""Bazel utilities for obtaining information about the build environment itself.

This module provides helper functions to introspect the Bazel build environment,
such as obtaining the label of the current target.
"""

load("@rules_python//python:defs.bzl", "py_binary")

def py_binary_with_self_label(name, **kwargs):
    """A wrapper around py_binary that injects the target's own label into its environment.

    This allows the binary to identify its own Bazel target label at runtime via the
    `BAZEL_SELF_LABEL` environment variable.


    Args:
      name: The name of the py_binary target.
      **kwargs: Additional arguments passed to py_binary.
    """
    env = kwargs.pop("env", {})
    repo = native.repository_name().rstrip("+")

    current_label = repo + "//" + native.package_name() + ":" + name
    env["BAZEL_SELF_LABEL"] = current_label
    py_binary(
        name = name,
        env = env,
        **kwargs
    )
