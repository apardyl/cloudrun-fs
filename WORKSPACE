workspace(name = "io_cloudrun_fs")

git_repository(
  name = "org_pubref_rules_protobuf",
  remote = "https://github.com/pubref/rules_protobuf",
  tag = "v0.8.2",
)

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cpp_proto_repositories")
cpp_proto_repositories()

new_local_repository(
  name = "system_libs",
  path = "/usr/lib/",
  build_file_content = """
cc_library(
    name = "fuse",
    srcs = ["libfuse.so"],
    visibility = ["//visibility:public"],
)
""",
)
