load("@rules_cc//cc:defs.bzl", "cc_binary")

COPTS = [
    "-pedantic",
    "-pedantic-errors",
    "-std=c++20",
    "-Wall",
    "-Wextra",
    "-Wshadow",
    "-fstrict-aliasing",
]

cc_binary(
    name = "explore",
    srcs = ["explore.cc"],
    copts = COPTS,
    deps = [
        ":bataille",
    ],
)

cc_library(
    name = "bataille",
    srcs = ["bataille.cc"],
    hdrs = ["bataille.h"],
    copts = COPTS,
    deps = [
    ],
)

cc_test(
    name = "bataille_test",
    srcs = ["bataille_test.cc"],
    copts = COPTS,
    deps = [
        ":bataille",
        "@google_benchmark//:benchmark",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "benchmark",
    srcs = ["benchmark.cc"],
    copts = COPTS,
    deps = [
        ":bataille",
        "@google_benchmark//:benchmark",
        "@google_benchmark//:benchmark_main",
    ],
)
