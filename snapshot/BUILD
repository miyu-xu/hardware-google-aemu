cc_library(
    name = "headers",
    hdrs = glob(["include/snapshot/*.h"]),
)

cc_library(
    name = "snapshot",
    srcs = [
        "TextureLoader.cpp",
        "TextureSaver.cpp",
    ],
    hdrs = glob(["include/snapshot/*.h"]),
    includes = ["include"],
    deps = [
        "//hardware/google/aemu/base:headers",
        "//hardware/google/aemu/host-common:headers",
    ],
)
