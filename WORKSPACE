workspace(name = "liblw")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Load the rules_foreign_cc tooling.
http_archive(
   name = "rules_foreign_cc",
   strip_prefix = "rules_foreign_cc-master",
   sha256 = "3e6b0691fc57db8217d535393dcc2cf7c1d39fc87e9adb6e7d7bab1483915110",
   url = "https://github.com/bazelbuild/rules_foreign_cc/archive/master.zip",
)
load(
  "@rules_foreign_cc//:workspace_definitions.bzl",
  "rules_foreign_cc_dependencies"
)
rules_foreign_cc_dependencies()

git_repository(
  name = "gtest",
  commit = "703bd9caab50b139428cea1aaff9974ebee5742e",
  shallow_since = "1570114335 -0400",
  remote = "https://github.com/google/googletest.git",
)

http_archive(
  name = "botan",
  strip_prefix = "botan-2.16.0",
  sha256 = "80c2b5c81c21bfeaa1dc74c92bec3bca5b689ec0f681833a9f6b6121e0d84a43",
  build_file_content =
    'filegroup(name = "all", srcs = glob(["**"]), ' +
    'visibility = ["//visibility:public"])',
  url = "https://github.com/randombit/botan/archive/2.16.0.zip",
)
