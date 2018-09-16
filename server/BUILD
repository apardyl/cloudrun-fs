package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cloudrun-fs-server-lib",
    srcs = [
        "rpc/RemoteFSService.cc",
        "rpc/RemoteFSServer.cc",
        "scanfs/scanfs.cc",
        "main.cc",
    ],
    hdrs = [
        "rpc/RemoteFSServer.h",
        "rpc/RemoteFSService.h",
        "scanfs/scanfs.h",
    ],
    deps = [
        "//common:cloudrun-fs-common",
    ]
)

cc_binary(
    name = "cloudrun-fs-server",
    deps = [
        ":cloudrun-fs-server-lib",
    ],
)
