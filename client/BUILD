package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cloudrun-fs-lib",
    srcs = glob(["**/*.cc"]),
    hdrs = glob(["**/*.h"]),
    deps = [
        "//common:cloudrun-fs-common",
    ],
)

cc_binary(
    name = "cloudrun-fs",
    linkopts = [
        "-lfuse",
    ],
    deps = [
        ":cloudrun-fs-lib",
    ],
)

load("@bazel_tools//tools/build_defs/pkg:pkg.bzl", "pkg_tar", "pkg_deb")

pkg_tar(
    name = "cloudrun-fs-pkg",
    srcs = [":cloudrun-fs"],
    mode = "0755",
    package_dir = "/usr/bin",
    strip_prefix = "/client",
)

pkg_deb(
    name = "cloudrun-fs-deb",
    architecture = "amd64",
    data = ":cloudrun-fs-pkg",
    depends = [
        "fuse",
        "libc6 (>= 2.17)",
        "libfuse2 (>= 2.6)",
    ],
    description = "Read-only caching file system based on FUSE for cloudrun project.",
    maintainer = "The Cloudrun authors <dev@cloudrun.io>",
    package = "cloudrun-fs",
    version = "0.1",
)
