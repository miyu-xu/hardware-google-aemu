"""This query helper prints out the following items for a target, to be consumed
by a cmake rule:

* The output archive file of the target.
* The include paths.
* The public defines.
"""

def format(target):
    """Returns info needed by cmake in a json string.

    Args:
        target: the target object passed from cquery.

    Returns:
        A json string that looks like:
        {
            "archive": "path/to/archive",
            "includes": "path1;path2;...",
            "defines": "key1;key2=val;...",
        }
        paths are relative to either output base directory or workspace root.
    """

    # Since Bazel 9 we have to refer to the provider like this.
    # It seems fragile. If it breaks often we could try looping the keys and
    # checking "CcInfo" in str(key).
    cc_info = providers(target).get("@@rules_cc+//cc/private:cc_info.bzl%CcInfo")

    combined_includes = []
    defines = []
    if hasattr(cc_info, "compilation_context"):
        compilation_context = cc_info.compilation_context
        quote_includes = compilation_context.quote_includes.to_list()
        system_includes = compilation_context.system_includes.to_list()
        external_includes = compilation_context.external_includes.to_list()

        # includes seems to be always empty, but we don't care and treat it the
        # same as others.
        includes = compilation_context.includes.to_list()
        combined_includes = _uniq([
            _normalize_execroot_path(i)
            for i in quote_includes + system_includes + external_includes + includes
        ])

        defines = compilation_context.defines.to_list()

    # Extract transitive dependencies and their archives.
    dependencies = []
    if hasattr(cc_info, "linking_context"):
        linker_inputs = cc_info.linking_context.linker_inputs.to_list()
        for linker_input in linker_inputs:
            for lib in linker_input.libraries:
                lib_path = ""
                if lib.static_library:
                    lib_path = lib.static_library.path
                elif lib.pic_static_library:
                    lib_path = lib.pic_static_library.path

                if lib_path:
                    dependencies.append(str(linker_input.owner) + "|" + _normalize_execroot_path(lib_path))

    json_struct = {
        "includes": ";".join(combined_includes),
        "defines": ";".join(defines),
        "dependencies": ";".join(_uniq(dependencies)),
    }

    # Add archive information if present.
    output_group_info = providers(target).get("OutputGroupInfo")
    if hasattr(output_group_info, "archive"):
        archive = output_group_info.archive.to_list()[0].path
        json_struct["archive"] = _normalize_execroot_path(archive)
    elif hasattr(output_group_info, "interface_library"):
        archive = output_group_info.interface_library.to_list()[0].path
        json_struct["archive"] = _normalize_execroot_path(archive)

    return json.encode(json_struct)

def _uniq(hashables):
    uniq = dict([(o, None) for o in hashables])
    return uniq.keys()

def _normalize_execroot_path(path):
    if path.startswith("external/"):
        # Note we assume you are *NOT* using a sibling repository
        return "${output_base}/" + path
    if path.startswith("../"):
        # For paths to third_party repositories, use "<output-base>/external" instead.
        return "${output_base}/external/" + path.removeprefix("../")
    if path.startswith("bazel-out"):
        # Use bazel info bazel_out to see what you should replace this with.
        return "${bazel_out}/" + path.removeprefix("bazel-out")

    # Otherwise it's a workspace path
    return "${workspace}/" + path
