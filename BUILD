load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "explore",
    srcs = ["explore.cc"],
    deps = [
        ":bataille",
    ],
)

cc_library(
    name = "bataille",
    srcs = ["bataille.cc"],
    hdrs = ["bataille.h"],
    deps = [
    ],
)

cc_test(
    name = "bataille_test",
    srcs = ["bataille_test.cc"],
    deps = [
        ":bataille",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
