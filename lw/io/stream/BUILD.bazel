load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
  name = "buffer",
  visibility = ["//visibility:public"],
  hdrs = ["buffer.h"],
  srcs = ["buffer.cpp"]
)

cc_test(
  name = "buffer_test",
  srcs = ["buffer_test.cpp"],
  deps = [
    ":buffer",
    "@gtest//:gtest_main",
  ]
)
