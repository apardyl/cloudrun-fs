package(default_visibility = ["//common:__pkg__"])

# proto rules
load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")

cc_proto_library(
    name = "cloudrun-fs-proto",
    protos = [
        "filesystem.proto",
        "remotefs.proto",
    ],
    with_grpc = True,
)

# grpc rules
load("@org_pubref_rules_protobuf//cpp:rules.bzl", "GRPC_COMPILE_DEPS")

cc_library(
    name = "cloudrun-fs-grpc",
    deps = [
        ":cloudrun-fs-proto",
    ] + GRPC_COMPILE_DEPS,
)
