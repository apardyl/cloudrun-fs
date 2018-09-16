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

load("@bazel_tools//tools/build_defs/pkg:pkg.bzl", "pkg_tar", "pkg_deb")

pkg_tar(
    name = "cloudrun-fs-pkg",
    strip_prefix = "/client",
    package_dir = "/usr/bin",
    srcs = [":cloudrun-fs"],
    mode = "0755",
)

pkg_deb(
    name = "cloudrun-fs-deb",
    architecture = "amd64",
    data = ":cloudrun-fs-pkg",
    depends = [
        "fuse",
        "libc6 (>= 2.17)",
        "libfuse2 (>= 2.9)",
    ],
    package = "cloudrun-fs",
    maintainer = "The Cloudrun authors <dev@cloudrun.io>",
    version = "0.1",
    description = "Read-only caching file system based on FUSE for cloudrun project."
)
