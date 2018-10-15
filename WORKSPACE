workspace(name = "io_cloudrun_fs")

git_repository(
  name = "org_pubref_rules_protobuf",
  remote = "https://github.com/pubref/rules_protobuf",
  tag = "v0.8.2",
)

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cpp_proto_repositories")
cpp_proto_repositories()

new_http_archive(
  name = "com_github_libfuse",
  url = "https://github.com/libfuse/libfuse/releases/download/fuse-2.9.8/fuse-2.9.8.tar.gz",
  sha256 = "5e84f81d8dd527ea74f39b6bc001c874c02bad6871d7a9b0c14efb57430eafe3",
  build_file = "BUILD.libfuse",
  strip_prefix = "fuse-2.9.8",
)
