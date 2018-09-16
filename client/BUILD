package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cloudrun-fs-lib",
    srcs = [
        "cachedfilestore.cc",
        "cachefs.cc",
        "fs_fuse.cc",
        "hashfs.cc",
        "hashstore.cc",
        "main.cc",
        "remotefsconnection.cc",
    ],
    hdrs = [
        "cachedfilestore.h",
        "cachefs.h",
        "config.h",
        "fs_fuse.h",
        "hashfs.h",
        "hashstore.h",
        "remotefsconnection.h",
    ],
    deps = [
        "//common:cloudrun-fs-common",
    ]
)

cc_binary(
    name = "cloudrun-fs",
    deps = [
        ":cloudrun-fs-lib",
        "@system_libs//:fuse",
    ],
)
