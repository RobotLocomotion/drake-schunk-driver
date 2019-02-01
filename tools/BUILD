# -*- python -*-
# This file contains rules for Bazel; see drake/doc/bazel.rst.

package(default_visibility = ["//visibility:public"])

py_library(
    name = "lcm_adapter",
    srcs = ["lcm_adapter.py"],
    deps = ["@lcm//:lcm-python"],
    )

py_binary(
    name = "lcm_logger",
    srcs = ["lcm_logger.py"],
    deps = [
        ":lcm_adapter",
        "@drake//lcmtypes:lcmtypes_drake_py",
    ],
)

py_binary(
    name = "lcm_joystick_control",
    srcs = ["lcm_joystick_control.py"],
    deps = [
        ":lcm_adapter",
        "@drake//lcmtypes:lcmtypes_drake_py",
    ],
)
