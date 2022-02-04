workspace(name = "liblw")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Load the rules_foreign_cc tooling.
git_repository(
  name = "rules_foreign_cc",
  branch = "main",
  remote = "https://github.com/bazelbuild/rules_foreign_cc.git",
)
load(
  "@rules_foreign_cc//foreign_cc:repositories.bzl",
  "rules_foreign_cc_dependencies"
)
rules_foreign_cc_dependencies()

git_repository(
  name = "gtest",
  commit = "1de637fbdd4ab0051229707f855eee76f5a3d5da",
  shallow_since = "1614008411 -0500",
  remote = "https://github.com/google/googletest.git",
)

http_archive(
  name = "openssl",
  strip_prefix = "openssl-openssl-3.0.1",
  sha256 = "53d8121af1c33c62a05a5370e9ba40fcc237717b79a7d99009b0c00c79bd7d78",
  build_file_content =
    'filegroup(name = "all", srcs = glob(["**"]), ' +
    'visibility = ["//visibility:public"])',
  url = "https://github.com/openssl/openssl/archive/openssl-3.0.1.zip",
)

git_repository(
  name = "com_google_protobuf",
  tag = "v3.19.4",
  remote = "https://github.com/protocolbuffers/protobuf.git",
)
git_repository(
  name = "com_github_grpc_grpc",
  tag = "v1.43.2",
  remote = "https://github.com/grpc/grpc.git",
)
load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")
protobuf_deps()
load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
grpc_deps()
load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")
grpc_extra_deps()
