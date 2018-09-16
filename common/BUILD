package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cloudrun-fs-common",
    srcs = glob(["**/*.cc"], exclude = ["proto/**"]),
    hdrs = glob(["**/*.h"], exclude = ["proto/**"]),
    deps = [
        "//common/proto:cloudrun-fs-grpc",
        "//external:libssl",
    ],
)
