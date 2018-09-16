package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cloudrun-fs-common",
    srcs = [
        "config.cc",
        "fsutils.cc",
        "randomutils.cc",
        "sha256.cc",
    ],
    hdrs = [
        "config.h",
        "fsutils.h",
        "randomutils.h",
        "sha256.h",
    ],
    deps = [
        "//common/proto:cloudrun-fs-grpc",
        "//external:libssl",
    ],
)
