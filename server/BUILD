package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cloudrun-fs-server-lib",
    srcs = glob(["**/*.cc"]),
    hdrs = glob(["**/*.h"]),
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

load("@bazel_tools//tools/build_defs/pkg:pkg.bzl", "pkg_tar", "pkg_deb")

pkg_tar(
    name = "cloudrun-fs-server-pkg",
    strip_prefix = "/server",
    package_dir = "/usr/bin",
    srcs = [":cloudrun-fs-server"],
    mode = "0755",
)

pkg_deb(
    name = "cloudrun-fs-server-deb",
    architecture = "amd64",
    data = ":cloudrun-fs-server-pkg",
    depends = [
        "libc6 (>= 2.17)",
    ],
    package = "cloudrun-fs-server",
    maintainer = "The Cloudrun authors <dev@cloudrun.io>",
    version = "0.1",
    description = "Server for cloudrun-fs."
)
